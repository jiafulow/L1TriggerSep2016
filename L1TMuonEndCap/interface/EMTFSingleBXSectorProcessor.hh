#ifndef L1TMuonEndCap_EMTFSingleBXSectorProcessor_hh
#define L1TMuonEndCap_EMTFSingleBXSectorProcessor_hh

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFCommon.hh"


class EMTFSingleBXSectorProcessor {
public:
  EMTFSingleBXSectorProcessor(const edm::ParameterSet& iConfig);
  ~EMTFSingleBXSectorProcessor();

  void reset(int sector, int bx);

  void process(
      const TriggerPrimitiveCollection& muon_primitives,
      EMTFHitExtraCollection& out_hits,
      EMTFTrackExtraCollection& out_tracks
  );

  int sector() const { return sector_; }

  int bx() const { return bx_; }

private:
  const edm::ParameterSet& config_;
  int verbose_;

  int sector_;
  int bx_;
};

#endif
