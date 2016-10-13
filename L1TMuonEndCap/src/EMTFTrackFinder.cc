#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFTrackFinder.hh"

#include <iostream>
#include <sstream>

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFSubsystemCollector.hh"


EMTFTrackFinder::EMTFTrackFinder(const edm::ParameterSet& iConfig, edm::ConsumesCollector&& iConsumes) :
    sector_processor_lut_(),
    pt_assign_engine_(),
    sector_processors_(),
    config_(iConfig),
    tokenCSC_(iConsumes.consumes<CSCCorrelatedLCTDigiCollection>(iConfig.getParameter<edm::InputTag>("CSCInput"))),
    tokenRPC_(iConsumes.consumes<RPCDigiCollection>(iConfig.getParameter<edm::InputTag>("RPCInput"))),
    verbose_(iConfig.getUntrackedParameter<int>("verbosity"))
{
  useCSC_      = iConfig.getParameter<bool>("CSCEnable");
  useRPC_      = iConfig.getParameter<bool>("RPCEnable");

  minBX_       = iConfig.getParameter<int>("MinBX");
  maxBX_       = iConfig.getParameter<int>("MaxBX");
  bxWindow_    = iConfig.getParameter<int>("BXWindow");
  bxShiftCSC_  = iConfig.getParameter<int>("CSCInputBXShift");

  version_     = iConfig.getParameter<int>("Version");
  ptlut_ver_   = iConfig.getParameter<int>("PtLUTVersion");

  const edm::ParameterSet spPCParams16 = config_.getParameter<edm::ParameterSet>("spPCParams16");
  auto phThLUT            = spPCParams16.getParameter<std::string>("PhThLUT");
  auto includeNeighbor    = spPCParams16.getParameter<bool>("IncludeNeighbor");
  auto duplicateTheta     = spPCParams16.getParameter<bool>("DuplicateTheta");
  auto fixZonePhi         = spPCParams16.getParameter<bool>("FixZonePhi");

  const edm::ParameterSet spPRParams16 = config_.getParameter<edm::ParameterSet>("spPRParams16");
  auto zoneBoundaries1    = spPRParams16.getParameter<std::vector<int> >("ZoneBoundaries1");
  auto zoneBoundaries2    = spPRParams16.getParameter<std::vector<int> >("ZoneBoundaries2");
  auto zoneOverlap        = spPRParams16.getParameter<int>("ZoneOverlap");
  auto pattDefinitions    = spPRParams16.getParameter<std::vector<std::string> >("PatternDefinitions");
  auto symPattDefinitions = spPRParams16.getParameter<std::vector<std::string> >("SymPatternDefinitions");
  auto maxRoadsPerZone    = spPRParams16.getParameter<int>("MaxRoadsPerZone");
  auto thetaWindow        = spPRParams16.getParameter<int>("ThetaWindow");
  auto maxTracks          = spPRParams16.getParameter<int>("MaxTracks");
  auto useSecondEarliest  = spPRParams16.getParameter<bool>("UseSecondEarliest");
  auto useSymPatterns     = spPRParams16.getParameter<bool>("UseSymmetricalPatterns");

  const edm::ParameterSet spPAParams16 = config_.getParameter<edm::ParameterSet>("spPAParams16");
  auto bdtXMLDir          = spPAParams16.getParameter<std::string>("BDTXMLDir");
  auto readPtLUTFile      = spPAParams16.getParameter<bool>("ReadPtLUTFile");
  auto fixMode15HighPt    = spPAParams16.getParameter<bool>("FixMode15HighPt");
  auto fix9bDPhi          = spPAParams16.getParameter<bool>("Fix9bDPhi");


  try {
    // Configure sector processor LUT
    sector_processor_lut_.read(phThLUT);

    // Configure pT assignment engine
    pt_assign_engine_.read(bdtXMLDir);

    // Configure sector processors
    for (int endcap = MIN_ENDCAP; endcap <= MAX_ENDCAP; ++endcap) {
      for (int sector = MIN_TRIGSECTOR; sector <= MAX_TRIGSECTOR; ++sector) {
        const int es = (endcap-1) * 6 + (sector-1);

        sector_processors_.at(es).configure(
            &sector_processor_lut_,
            &pt_assign_engine_,
            verbose_, endcap, sector,
            minBX_, maxBX_, bxWindow_, bxShiftCSC_,
            includeNeighbor, duplicateTheta, fixZonePhi,
            zoneBoundaries1, zoneBoundaries2, zoneOverlap,
            pattDefinitions, symPattDefinitions,
            maxRoadsPerZone, thetaWindow, maxTracks,
            useSecondEarliest, useSymPatterns,
            readPtLUTFile, fixMode15HighPt, fix9bDPhi
        );
      }
    }
    assert(sector_processors_.size() == NUM_SECTORS);

  } catch (...) {
    throw;
  }
}

EMTFTrackFinder::~EMTFTrackFinder() {

}

void EMTFTrackFinder::process(
    const edm::Event& iEvent, const edm::EventSetup& iSetup,
    EMTFHitExtraCollection& out_hits,
    EMTFTrackExtraCollection& out_tracks
) const {

  out_hits.clear();
  out_tracks.clear();

  // ___________________________________________________________________________
  // Extract all trigger primitives
  TriggerPrimitiveCollection muon_primitives;

  EMTFSubsystemCollector collector;
  if (useCSC_)
    collector.extractPrimitives(CSCTag(), iEvent, tokenCSC_, muon_primitives);
  if (useRPC_)
    collector.extractPrimitives(RPCTag(), iEvent, tokenRPC_, muon_primitives);

  // Check trigger primitives
  if (verbose_ > 2) {  // debug
    std::cout << "Num of TriggerPrimitive: " << muon_primitives.size() << std::endl;
    for (const auto& p : muon_primitives) {
      p.print(std::cout);
    }
  }

  // ___________________________________________________________________________
  // Run each sector processor

  for (int endcap = MIN_ENDCAP; endcap <= MAX_ENDCAP; ++endcap) {
    for (int sector = MIN_TRIGSECTOR; sector <= MAX_TRIGSECTOR; ++sector) {
      const int es = (endcap-1) * 6 + (sector-1);

      sector_processors_.at(es).process(
          iEvent.id().event(),
          muon_primitives,
          out_hits,
          out_tracks
      );
    }
  }

  if (verbose_ > 0) {  // debug
    std::cout << "Num of EMTFHitExtra: " << out_hits.size() << std::endl;
    std::cout << "bx e s ss st vf ql cp wg id bd hs" << std::endl;
    for (const auto& h : out_hits) {
      int bx      = h.bx + 3;
      int sector  = h.pc_sector;
      int station = (h.pc_station == 0 && h.subsector == 1) ? 1 : h.pc_station;
      int chamber = h.pc_chamber + 1;
      int strip   = (h.station == 1 && h.ring == 4) ? h.strip + 128 : h.strip;  // ME1/1a
      std::cout << bx << " " << h.endcap << " " << sector << " " << h.subsector << " " << station << " " << h.valid << " " << h.quality << " " << h.pattern << " " << h.wire << " " << chamber << " " << h.bend << " " << strip << std::endl;
    }

    std::cout << "Converted hits: " << std::endl;
    for (const auto& h : out_hits) {
      std::cout << h.pc_station << " " << h.pc_chamber << " " << h.phi_fp << " " << h.theta_fp << " " << (1ul<<h.ph_hit) << " " << h.phzvl << std::endl;
    }

    std::cout << "Num of EMTFTrackExtra: " << out_tracks.size() << std::endl;
    std::cout << "bx e s a mo et ph cr q pt" << std::endl;
    for (const auto& t : out_tracks) {
      std::cout << t.bx << " " << t.endcap << " " << t.sector << " " << t.ptlut_address << " " << t.mode << " " << t.gmt_eta << " " << t.gmt_phi << " " << t.gmt_charge << " " << t.gmt_quality << " " << t.pt << std::endl;
    }
  }

  return;
}
