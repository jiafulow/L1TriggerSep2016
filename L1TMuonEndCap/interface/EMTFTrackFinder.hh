#ifndef L1TMuonEndCap_EMTFTrackFinder_hh
#define L1TMuonEndCap_EMTFTrackFinder_hh

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

//#include "DataFormats/CSCDigi/interface/CSCCorrelatedLCTDigiCollection.h"
//#include "DataFormats/RPCDigi/interface/RPCDigiCollection.h"
#include "DataFormats/L1TMuon/interface/EMTFHitExtra.h"
#include "DataFormats/L1TMuon/interface/EMTFTrackExtra.h"
#include "DataFormats/L1TMuon/interface/RegionalMuonCand.h"
#include "DataFormats/L1TMuon/interface/RegionalMuonCandFwd.h"


class EMTFTrackFinder {
public:
  EMTFTrackFinder(const edm::ParameterSet& iConfig, edm::ConsumesCollector&& iConsumes);
  ~EMTFTrackFinder();

  void process(
      const edm::Event& iEvent, const edm::EventSetup& iSetup,
      l1t::EMTFHitExtraCollection& out_hits,
      l1t::EMTFTrackExtraCollection& out_tracks,
      l1t::RegionalMuonCandBxCollection& out_cands
  );

private:
  edm::EDGetToken tokenCSC_;
  edm::EDGetToken tokenRPC_;
  int verbose_;

};

#endif
