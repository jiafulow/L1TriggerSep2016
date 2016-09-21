#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFTrackFinder.hh"

#include <iostream>
#include <sstream>

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFSubsystemCollector.hh"


EMTFTrackFinder::EMTFTrackFinder(const edm::ParameterSet& iConfig, edm::ConsumesCollector&& iConsumes) :
    sector_processor_lut_(),
    sector_processors_(),
    config_(iConfig),
    tokenCSC_(iConsumes.consumes<CSCCorrelatedLCTDigiCollection>(iConfig.getParameter<edm::InputTag>("CSCInput"))),
    tokenRPC_(iConsumes.consumes<RPCDigiCollection>(iConfig.getParameter<edm::InputTag>("RPCInput"))),
    verbose_(iConfig.getUntrackedParameter<int>("verbosity"))
{
  useCSC_      = iConfig.getParameter<bool>("CSCEnable");
  useRPC_      = iConfig.getParameter<bool>("RPCEnable");

  ph_th_lut_   = iConfig.getParameter<std::string>("PhThLUT");

  const edm::ParameterSet spPCParams16 = config_.getParameter<edm::ParameterSet>("spPCParams16");
  auto includeNeighbor  = spPCParams16.getParameter<bool>("IncludeNeighbor");
  auto duplicateWires   = spPCParams16.getParameter<bool>("DuplicateWires");

  const edm::ParameterSet spPRParams16 = config_.getParameter<edm::ParameterSet>("spPRParams16");
  auto minBX            = spPRParams16.getParameter<int>("MinBX");
  auto maxBX            = spPRParams16.getParameter<int>("MaxBX");
  auto bxWindow         = spPRParams16.getParameter<int>("BXWindow");
  auto zoneBoundaries1  = spPRParams16.getParameter<std::vector<int> >("ZoneBoundaries1");
  auto zoneBoundaries2  = spPRParams16.getParameter<std::vector<int> >("ZoneBoundaries2");
  auto zoneOverlap      = spPRParams16.getParameter<int>("ZoneOverlap");
  auto pattDefinitions  = spPRParams16.getParameter<std::vector<std::string> >("PatternDefinitions");
  auto maxRoadsPerZone  = spPRParams16.getParameter<int>("MaxRoadsPerZone");
  auto thetaWindow      = spPRParams16.getParameter<int>("ThetaWindow");
  auto maxTracks        = spPRParams16.getParameter<int>("MaxTracks");

  try {
    // Configure sector processor LUT
    sector_processor_lut_.read(ph_th_lut_);

    // Configure sector processors
    for (int endcap = MIN_ENDCAP; endcap <= MAX_ENDCAP; ++endcap) {
      for (int sector = MIN_TRIGSECTOR; sector <= MAX_TRIGSECTOR; ++sector) {
        sector_processors_.push_back(EMTFSectorProcessor());

        sector_processors_.back().configure(
            &sector_processor_lut_,
            endcap, sector,
            includeNeighbor, duplicateWires,
            minBX, maxBX, bxWindow,
            zoneBoundaries1, zoneBoundaries2, zoneOverlap,
            pattDefinitions,
            maxRoadsPerZone, thetaWindow, maxTracks
        );
      }
    }
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
  if (verbose_ > 2) {
    std::cout << "Num of TriggerPrimitive: " << muon_primitives.size() << std::endl;
    std::ostringstream o;
    for (const auto& p : muon_primitives) {
      p.print(o);
      std::cout << o.str() << std::endl;
      o.str("");
    }
  }

  // ___________________________________________________________________________
  // Run each sector processor

  for (int endcap = MIN_ENDCAP; endcap <= MAX_ENDCAP; ++endcap) {
    for (int sector = MIN_TRIGSECTOR; sector <= MAX_TRIGSECTOR; ++sector) {
      int es = (endcap-1) * 6 + (sector-1);

      sector_processors_.at(es).process(
          iEvent.id().event(), muon_primitives,
          out_hits, out_tracks
      );
    }
  }

  if (verbose_ > 1) {
    std::cout << "Num of EMTFHitExtra: " << out_hits.size() << std::endl;
    std::cout << "bx e s ss st vf ql cp wg id bd hs" << std::endl;
    for (const auto& h : out_hits) {
      int bx      = h.bx + 3;
      int station = (h.pc_station == 0 && h.subsector == 1) ? 1 : h.pc_station;
      int chamber = h.pc_chamber + 1;
      int strip   = (h.station == 1 && h.ring == 4) ? h.strip + 128 : h.strip;  // ME1/1a
      std::cout << bx << " " << h.endcap << " " << h.sector << " " << h.subsector << " " << station << " " << h.valid << " " << h.quality << " " << h.pattern << " " << h.wire << " " << chamber << " " << h.bend << " " << strip << std::endl;
    }

    std::cout << "Primitives: " << std::endl;
    for (const auto& h : out_hits) {
      std::cout << h.pc_station << " " << h.pc_chamber << " " << h.phi_fp << " " << h.theta_fp << " " << (1ul<<h.ph_hit) << " " << h.phzvl << std::endl;
    }

    std::cout << "Tracks: " << std::endl;
    for (const auto& t : out_tracks) {
      std::cout << t.winner << " " << t.rank << " " << t.xroad.zone << " " << t.xroad.winner<< std::endl;
    }
  }


  return;
}
