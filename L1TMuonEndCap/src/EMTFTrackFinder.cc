#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFTrackFinder.hh"

#include <iostream>
#include <sstream>

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFSubsystemCollector.hh"


EMTFTrackFinder::EMTFTrackFinder(const edm::ParameterSet& iConfig, edm::ConsumesCollector&& iConsumes) :
    sector_processor_(new EMTFSectorProcessor()),
    sector_processor_lut_(new EMTFSectorProcessorLUT()),
    config_(iConfig),
    tokenCSC_(iConsumes.consumes<CSCCorrelatedLCTDigiCollection>(iConfig.getParameter<edm::InputTag>("CSCInput"))),
    tokenRPC_(iConsumes.consumes<RPCDigiCollection>(iConfig.getParameter<edm::InputTag>("RPCInput"))),
    verbose_(iConfig.getUntrackedParameter<int>("verbosity"))
{
  useCSC_      = iConfig.getParameter<bool>("CSCEnable");
  useRPC_      = iConfig.getParameter<bool>("RPCEnable");

  ph_th_lut_   = iConfig.getParameter<std::string>("PhThLUT");

  const edm::ParameterSet spPRParams16 = config_.getParameter<edm::ParameterSet>("spPRParams16");
  minBX_    = spPRParams16.getParameter<int>("MinBX");
  maxBX_    = spPRParams16.getParameter<int>("MaxBX");
  bxWindow_ = spPRParams16.getParameter<int>("BXWindow");

  const edm::ParameterSet spPCParams16 = config_.getParameter<edm::ParameterSet>("spPCParams16");
  includeNeighbor_ = spPCParams16.getParameter<bool>("IncludeNeighbor");
  duplicateWires_  = spPCParams16.getParameter<bool>("DuplicateWires");
}

EMTFTrackFinder::~EMTFTrackFinder() {

}

void EMTFTrackFinder::process(
    const edm::Event& iEvent, const edm::EventSetup& iSetup,
    EMTFHitExtraCollection& out_hits,
    EMTFTrackExtraCollection& out_tracks
) {

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

  sector_processor_lut_->read(ph_th_lut_);

  for (int iendcap = MIN_ENDCAP; iendcap <= MAX_ENDCAP; iendcap++) {
    for (int isector = MIN_TRIGSECTOR; isector <= MAX_TRIGSECTOR; isector++) {
      sector_processor_->configure(
          sector_processor_lut_.get(),
          iendcap, isector,
          minBX_, maxBX_, bxWindow_,
          includeNeighbor_, duplicateWires_
      );

      sector_processor_->process(muon_primitives, out_hits, out_tracks);
    }
  }

  if (verbose_ > 1) {
    std::cout << "Num of EMTFHitExtra: " << out_hits.size() << std::endl;
    for (const auto& h : out_hits) {
      int bx      = h.bx + 3;
      int station = (h.pc_station == 0 && h.subsector == 1) ? 1 : h.pc_station;
      int csc_ID  = h.pc_chamber + 1;
      std::cout << bx << " " << h.endcap << " " << h.sector << " " << h.subsector << " " << station << " " << h.valid << " " << h.quality << " " << h.pattern << " " << h.wire << " " << csc_ID << " " << h.bend << " " << h.strip << std::endl;
    }

    std::cout << "Primitive conversion: " << std::endl;
    for (const auto& h : out_hits) {
      std::cout << h.pc_station << " " << h.pc_chamber << " " << h.phi_fp << " " << h.theta_fp << " " << (1ul<<h.ph_hit) << " " << h.phzvl << std::endl;
    }
  }


  //assert(muon_primitives.size() == out_hits.size());

  return;
}
