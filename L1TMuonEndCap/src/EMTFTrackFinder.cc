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
  // Extract trigger primitives
  TriggerPrimitiveCollection muon_primitives;

  EMTFSubsystemCollector collector;
  if (useCSC_)
    collector.extractPrimitives<L1TMuonEndCap::CSCTag>(iEvent, tokenCSC_, muon_primitives);
  if (useRPC_)
    collector.extractPrimitives<L1TMuonEndCap::RPCTag>(iEvent, tokenRPC_, muon_primitives);

  // Check trigger primitives
  if (verbose_ > 2) {
    std::ostringstream o;
    for (const auto& p : muon_primitives) {
      p.print(o);
      std::cout << o.str() << std::endl;
      o.str("");
    }
  }

  // ___________________________________________________________________________
  // Run each sector processor

  EMTFSectorProcessor sector_processor(config_);

  for (int isector = 0; isector < NUM_SECTORS; isector++) {
    sector_processor.reset(isector);
    sector_processor.process(muon_primitives, out_hits, out_tracks);
  }

  return;
}
