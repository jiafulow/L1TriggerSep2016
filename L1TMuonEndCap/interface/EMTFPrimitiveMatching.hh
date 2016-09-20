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

  void match_single_zone_station(
    int station,
    const EMTFRoadExtraCollection& roads,
    const EMTFHitExtraCollection& conv_hits,
    std::vector<std::pair<int, int> >& phi_differences
  ) const;

  void sort_ph_diff(std::vector<std::pair<int, int> >& phi_differences) const;

  void insert_hits(
      int station, int ichit, int ph_diff, const EMTFHitExtraCollection& conv_hits,
      EMTFTrackExtra& track
  ) const;

  void insert_hit(
      int station, int ichit, int ph_diff, const EMTFHitExtraCollection& conv_hits,
      EMTFTrackExtra& track
  ) const;

  unsigned int get_fs_zone_code(const EMTFHitExtra& conv_hit) const;

  unsigned int get_fs_segment(const EMTFHitExtra& conv_hit) const;

private:
  int endcap_, sector_, bx_;
};

#endif
