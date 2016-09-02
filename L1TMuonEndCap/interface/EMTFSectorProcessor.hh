#ifndef L1TMuonEndCap_EMTFSectorProcessor_hh
#define L1TMuonEndCap_EMTFSectorProcessor_hh

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFSingleBXSectorProcessor.hh"


class EMTFSectorProcessor {
public:
  EMTFSectorProcessor(const edm::ParameterSet& iConfig);
  ~EMTFSectorProcessor();

  void reset(int sector);

  void process(
      const L1TMuon::TriggerPrimitiveCollection& muon_primitives,
      l1t::EMTFHitExtraCollection& out_hits,
      l1t::EMTFTrackExtraCollection& out_tracks
  );

  int sector() const { return sector_; }

private:
  const edm::ParameterSet& config_;
  int verbose_;

  int sector_;
};

#endif
