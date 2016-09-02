#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFTrackFinder.hh"

#include <iostream>
#include <sstream>

#include "FWCore/Utilities/interface/Exception.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFSubsystemCollector.hh"
#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFSectorProcessor.hh"

#define NUM_SECTORS 12


EMTFTrackFinder::EMTFTrackFinder(const edm::ParameterSet& iConfig, edm::ConsumesCollector&& iConsumes) :
    config_(iConfig),
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

  out_hits.clear();
  out_tracks.clear();
  out_cands.clear();

  // ___________________________________________________________________________
  // Extract trigger primitives
  L1TMuon::TriggerPrimitiveCollection muon_primitives;

  EMTFSubsystemCollector collector;
  collector.extractPrimitives<L1TMuonEndCap::CSCTag>(iEvent, tokenCSC_, muon_primitives);
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

  // ___________________________________________________________________________
  // Put into uGMT format

  //FIXME: implement this

  return;
}
