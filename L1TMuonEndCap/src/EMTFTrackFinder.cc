#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFTrackFinder.hh"

#include <iostream>
#include <sstream>

#include "FWCore/Utilities/interface/Exception.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFSubsystemCollector.hh"

#define NUM_SECTORS 12


EMTFTrackFinder::EMTFTrackFinder(const edm::ParameterSet& iConfig, edm::ConsumesCollector&& iConsumes) :
    tokenCSC_(iConsumes.consumes<CSCCorrelatedLCTDigiCollection>(iConfig.getParameter<edm::InputTag>("CSCInput"))),
    tokenRPC_(iConsumes.consumes<RPCDigiCollection>(iConfig.getParameter<edm::InputTag>("RPCInput"))),
    verbose_(iConfig.getUntrackedParameter<int>("verbosity"))
{

}

EMTFTrackFinder::~EMTFTrackFinder() {

}

void EMTFTrackFinder::process(
    const edm::Event& iEvent, const edm::EventSetup& iSetup,
    l1t::EMTFHitExtraCollection& out_hits,
    l1t::EMTFTrackExtraCollection& out_tracks,
    l1t::RegionalMuonCandBxCollection& out_cands
) {

  // ___________________________________________________________________________
  // Extract trigger primitives
  L1TMuon::TriggerPrimitiveCollection triggerPrimitives;

  EMTFSubsystemCollector collector;
  collector.extractPrimitives<L1TMuonEndCap::CSCTag>(iEvent, tokenCSC_, triggerPrimitives);
  collector.extractPrimitives<L1TMuonEndCap::RPCTag>(iEvent, tokenRPC_, triggerPrimitives);

  // Check trigger primitives
  if (verbose_ > 2) {
    std::ostringstream o;
    for (const auto& tp : triggerPrimitives) {
      tp.print(o);
      std::cout << o.str() << std::endl;
      o.str("");
    }
  }

  // ___________________________________________________________________________
  // Run each sector processor
  for (int isector=0; isector<NUM_SECTORS; isector++) {

  }

  return;
}
