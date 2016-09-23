#ifndef L1TMuonEndCap_EMTFMicroGMTConverter_hh
#define L1TMuonEndCap_EMTFMicroGMTConverter_hh

#include "DataFormats/L1TMuon/interface/RegionalMuonCand.h"
#include "DataFormats/L1TMuon/interface/RegionalMuonCandFwd.h"

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFCommon.hh"


class EMTFMicroGMTConverter {
public:
  explicit EMTFMicroGMTConverter();
  ~EMTFMicroGMTConverter();

  void convert(
      const EMTFTrackExtra& in_track,
      l1t::RegionalMuonCand& out_cand
  ) const;

  void convert_many(
      const EMTFTrackExtraCollection& in_tracks,
      l1t::RegionalMuonCandBxCollection& out_cands
  ) const;

private:
};

#endif
