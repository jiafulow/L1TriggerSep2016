#ifndef L1TMuonEndCap_EMTFPrimitiveMatching_hh
#define L1TMuonEndCap_EMTFPrimitiveMatching_hh

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFCommon.hh"


class EMTFPrimitiveMatching {
public:
  void configure(
    int endcap, int sector, int bx
  );

  void match(
    const std::deque<EMTFHitExtraCollection>& extended_conv_hits,
    const std::vector<EMTFRoadExtraCollection>& zone_roads,
    std::vector<EMTFTrackExtraCollection>& zone_tracks
  ) const;


  unsigned int get_zone_code_pm(const EMTFHitExtra& conv_hit) const;

  void sort_ph_diff(std::vector<std::pair<int, int> >& phi_differences) const;

  EMTFTrackExtra make_track(
      const EMTFHitExtraCollection& conv_hits,
      std::pair<int, int> best_ph_diff
  ) const;

private:
  int endcap_, sector_, bx_;
};

#endif
