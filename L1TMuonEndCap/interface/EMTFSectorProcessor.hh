#ifndef L1TMuonEndCap_EMTFSectorProcessor_hh
#define L1TMuonEndCap_EMTFSectorProcessor_hh

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFCommon.hh"


class EMTFSectorProcessor {
public:

  void configure(
      int endcap, int sector,
      int minBX, int maxBX, int bxWindow,
      bool includeNeighbor, bool duplicateWires
  );

  void process(
      const TriggerPrimitiveCollection& muon_primitives,
      EMTFHitExtraCollection& out_hits,
      EMTFTrackExtraCollection& out_tracks
  );

  void process_single_bx(
      int bx,
      const TriggerPrimitiveCollection& muon_primitives,
      EMTFHitExtraCollection& out_hits,
      EMTFTrackExtraCollection& out_tracks
  );

  int sector() const { return sector_; }

private:
  int endcap_, sector_;

  int minBX_, maxBX_, bxWindow_;

  bool includeNeighbor_, duplicateWires_;
};

#endif
