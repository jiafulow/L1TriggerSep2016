#ifndef L1TMuonEndCap_EMTFTrackFinder_hh
#define L1TMuonEndCap_EMTFTrackFinder_hh

#include <memory>
#include <string>
#include <vector>

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFSectorProcessor.hh"
#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFSectorProcessorLUT.hh"


class EMTFTrackFinder {
public:
  explicit EMTFTrackFinder(const edm::ParameterSet& iConfig, edm::ConsumesCollector&& iConsumes);
  ~EMTFTrackFinder();

  void process(
      const edm::Event& iEvent, const edm::EventSetup& iSetup,
      EMTFHitExtraCollection& out_hits,
      EMTFTrackExtraCollection& out_tracks
  );

private:
  std::unique_ptr<EMTFSectorProcessor> sector_processor_;
  std::unique_ptr<EMTFSectorProcessorLUT> sector_processor_lut_;

  const edm::ParameterSet config_;
  const edm::EDGetToken tokenCSC_;
  const edm::EDGetToken tokenRPC_;
  int verbose_;

  bool useCSC_, useRPC_;

  std::string ph_th_lut_;

  int minBX_, maxBX_, bxWindow_;
  std::vector<int> zoneBoundaries1_, zoneBoundaries2_;
  int zoneOverlap_;
  std::vector<std::string> pattDefinitions_;
  int maxRoadsPerZone_;

  bool includeNeighbor_, duplicateWires_;

};

#endif
