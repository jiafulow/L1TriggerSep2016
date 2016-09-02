#ifndef L1TMuonEndCap_EMTFSingleBXSectorProcessor_hh
#define L1TMuonEndCap_EMTFSingleBXSectorProcessor_hh

#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "L1TriggerSep2016/L1TMuonEndCap/interface/MuonTriggerPrimitive.h"
#include "L1TriggerSep2016/L1TMuonEndCap/interface/MuonTriggerPrimitiveFwd.h"
#include "DataFormats/L1TMuon/interface/EMTFHitExtra.h"
#include "DataFormats/L1TMuon/interface/EMTFTrackExtra.h"


class EMTFSingleBXSectorProcessor {
public:
  EMTFSingleBXSectorProcessor(const edm::ParameterSet& iConfig);
  ~EMTFSingleBXSectorProcessor();

  void reset(int sector, int bx);

  void process(
      const L1TMuon::TriggerPrimitiveCollection& muon_primitives,
      l1t::EMTFHitExtraCollection& out_hits,
      l1t::EMTFTrackExtraCollection& out_tracks
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
