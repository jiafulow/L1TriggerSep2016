#ifndef L1TMuonEndCap_TrackAdaptor_hh
#define L1TMuonEndCap_TrackAdaptor_hh

#include "L1Trigger/L1TMuonEndCap/interface/Common.hh"


class TrackAdaptor {
public:
  explicit TrackAdaptor();
  ~TrackAdaptor();

  void convert_hit(const EMTFHit& in_hit, EMTFHit& out_hit) const;

  void convert_track(const EMTFTrack& in_track, EMTFTrack& out_track) const;

  void convert_all(
      const EMTFHitCollection& in_hits, const EMTFTrackCollection& in_tracks,
      EMTFHitCollection& out_hits, EMTFTrackCollection& out_tracks
  ) const;

private:
};

#endif
