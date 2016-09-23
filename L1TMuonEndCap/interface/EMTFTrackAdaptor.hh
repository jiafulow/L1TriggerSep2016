#ifndef L1TMuonEndCap_EMTFTrackAdaptor_hh
#define L1TMuonEndCap_EMTFTrackAdaptor_hh

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFCommon.hh"


class EMTFTrackAdaptor {
public:
  explicit EMTFTrackAdaptor();
  ~EMTFTrackAdaptor();

  void convert_hit(const EMTFHitExtra& in_hit, EMTFHit& out_hit) const;

  void convert_track(const EMTFTrackExtra& in_track, EMTFTrack& out_track) const;

  void convert_all(
      const EMTFHitExtraCollection& in_hits, const EMTFTrackExtraCollection& in_tracks,
      EMTFHitCollection& out_hits, EMTFTrackCollection& out_tracks
  ) const;

private:
};

#endif
