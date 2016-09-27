#ifndef L1TMuonEndCap_EMTFTrackFinder_hh
#define L1TMuonEndCap_EMTFTrackFinder_hh

#include <memory>
#include <string>
#include <vector>
#include <array>

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFSectorProcessorLUT.hh"
#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPtAssignmentEngine.hh"
#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFSectorProcessor.hh"


class EMTFTrackFinder {
public:
  explicit EMTFTrackFinder(const edm::ParameterSet& iConfig, edm::ConsumesCollector&& iConsumes);
  ~EMTFTrackFinder();

  void process(
      const edm::Event& iEvent, const edm::EventSetup& iSetup,
      EMTFHitExtraCollection& out_hits,
      EMTFTrackExtraCollection& out_tracks
  ) const;

private:
  EMTFSectorProcessorLUT sector_processor_lut_;

  EMTFPtAssignmentEngine pt_assignment_engine_;

  std::array<EMTFSectorProcessor, 12> sector_processors_;

  const edm::ParameterSet config_;

  const edm::EDGetToken tokenCSC_, tokenRPC_;

  int verbose_;

  bool useCSC_, useRPC_;

  int version_, ptlut_ver_;

  std::string ph_th_lut_;
};

#endif
