#ifndef L1TMuonEndCap_EMTFPrimitiveMatching_hh
#define L1TMuonEndCap_EMTFPrimitiveMatching_hh

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFCommon.hh"


class EMTFPrimitiveMatching {
public:
  typedef EMTFHitExtraCollection::const_iterator hit_ptr_t;
  typedef std::pair<int, hit_ptr_t> hit_sort_pair_t;  // key=ph_diff, value=hit

  void configure(
      int verbose, int endcap, int sector, int bx,
      bool fixZonePhi
  );

  void process(
      const std::deque<EMTFHitExtraCollection>& extended_conv_hits,
      const zone_array<EMTFRoadExtraCollection>& zone_roads,
      zone_array<EMTFTrackExtraCollection>& zone_tracks
  ) const;

  void process_single_zone_station(
      int station,
      const EMTFRoadExtraCollection& roads,
      const EMTFHitExtraCollection& conv_hits,
      std::vector<hit_sort_pair_t>& phi_differences
  ) const;

  void sort_ph_diff(
      std::vector<hit_sort_pair_t>& phi_differences
  ) const;

  void insert_hits(
      hit_ptr_t conv_hit_ptr, const EMTFHitExtraCollection& conv_hits,
      EMTFTrackExtra& track
  ) const;

  void insert_hit(
      hit_ptr_t conv_hit_ptr,
      EMTFTrackExtra& track
  ) const;

private:
  int verbose_, endcap_, sector_, bx_;

  bool fixZonePhi_;
};

#endif
