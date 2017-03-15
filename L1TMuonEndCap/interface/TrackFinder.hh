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
      // Input
      const edm::Event& iEvent, const edm::EventSetup& iSetup,
      // Output
      EMTFHitExtraCollection& out_hits,
      EMTFTrackExtraCollection& out_tracks
  ) const;

private:
  // 'mutable' because GeometryTranslator has to 'update' inside the const function
  mutable GeometryTranslator geometry_translator_;

  EMTFSectorProcessorLUT sector_processor_lut_;

  EMTFPtAssignmentEngine pt_assign_engine_;

  std::array<EMTFSectorProcessor, NUM_SECTORS> sector_processors_;

  const edm::ParameterSet config_;

  const edm::EDGetToken tokenCSC_, tokenRPC_;

  int verbose_;

  bool useCSC_, useRPC_;
};

#endif
