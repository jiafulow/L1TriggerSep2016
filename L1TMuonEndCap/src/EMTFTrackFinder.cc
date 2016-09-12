#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFTrackFinder.hh"

#include <iostream>
#include <sstream>

#include "FWCore/Utilities/interface/Exception.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFSubsystemCollector.hh"
#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFSectorProcessor.hh"


EMTFTrackFinder::EMTFTrackFinder(const edm::ParameterSet& iConfig, edm::ConsumesCollector&& iConsumes) :
    config_(iConfig),
    tokenCSC_(iConsumes.consumes<CSCCorrelatedLCTDigiCollection>(iConfig.getParameter<edm::InputTag>("CSCInput"))),
    tokenRPC_(iConsumes.consumes<RPCDigiCollection>(iConfig.getParameter<edm::InputTag>("RPCInput"))),
    verbose_(iConfig.getUntrackedParameter<int>("verbosity"))
{
  useCSC_      = iConfig.getParameter<bool>("CSCEnable");
  useRPC_      = iConfig.getParameter<bool>("RPCEnable");

  const edm::ParameterSet spPRParams16 = config_.getParameter<edm::ParameterSet>("spPRParams16");
  minBX_    = spPRParams16.getParameter<int>("MinBX");
  maxBX_    = spPRParams16.getParameter<int>("MaxBX");
  bxWindow_ = spPRParams16.getParameter<int>("BXWindow");

  const edm::ParameterSet spPCParams16 = config_.getParameter<edm::ParameterSet>("spPCParams16");
  includeNeighbor_ = spPCParams16.getParameter<bool>("IncludeNeighbor");
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

  EMTFSectorProcessor sector_processor;

  for (int iendcap = MIN_ENDCAP; iendcap <= MAX_ENDCAP; iendcap++) {
    for (int isector = MIN_TRIGSECTOR; isector <= MAX_TRIGSECTOR; isector++) {
      sector_processor.configure(
          iendcap, isector,
          minBX_, maxBX_, bxWindow_,
          includeNeighbor_
      );

      sector_processor.process(muon_primitives, out_hits, out_tracks);
    }
  }

  if (verbose_ > 1) {
    std::cout << "Num of EMTFHitExtra: " << out_hits.size() << std::endl;
    for (const auto& h : out_hits) {
      std::cout << h.getData().bx+3 << " " << h.getData().endcap << " " << h.getData().sector << " " << h.getData().subsector << " " << h.getData().station << " " << h.getData().valid << " " << h.getData().quality << " " << h.getData().pattern << " " << h.getData().wire << " " << h.getData().csc_ID << " " << h.getData().bend << " " << h.getData().strip << " neigh? " << h.getData().neighbor << std::endl;
    }
  }


  //assert(muon_primitives.size() == out_hits.size());

  return;
}
