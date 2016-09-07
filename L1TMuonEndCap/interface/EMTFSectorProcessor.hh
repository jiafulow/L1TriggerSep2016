#ifndef L1TMuonEndCap_EMTFSectorProcessor_hh
#define L1TMuonEndCap_EMTFSectorProcessor_hh

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFCommon.hh"


class EMTFSectorProcessor {
public:
  EMTFSectorProcessor(const edm::ParameterSet& iConfig);
  ~EMTFSectorProcessor();

  void reset(int sector);

  void process(
      const TriggerPrimitiveCollection& muon_primitives,
      EMTFHitExtraCollection& out_hits,
      EMTFTrackExtraCollection& out_tracks
  );

  int sector() const { return sector_; }

private:
  const edm::ParameterSet config_;
  int verbose_;

  int minBX_, maxBX_, bxWindow_;

  int sector_;
};

#endif
