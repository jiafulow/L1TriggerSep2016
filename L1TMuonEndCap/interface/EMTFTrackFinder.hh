#ifndef L1TMuonEndCap_EMTFTrackFinder_hh
#define L1TMuonEndCap_EMTFTrackFinder_hh

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFCommon.hh"


class EMTFTrackFinder {
public:
  EMTFTrackFinder(const edm::ParameterSet& iConfig, edm::ConsumesCollector&& iConsumes);
  ~EMTFTrackFinder();

  void process(
      const edm::Event& iEvent, const edm::EventSetup& iSetup,
      EMTFHitExtraCollection& out_hits,
      EMTFTrackExtraCollection& out_tracks
  );

private:
  const edm::ParameterSet& config_;
  edm::EDGetToken tokenCSC_;
  edm::EDGetToken tokenRPC_;
  int verbose_;

};

#endif
