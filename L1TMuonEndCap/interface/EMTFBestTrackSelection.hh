#ifndef L1TMuonEndCap_EMTFBestTrackSelection_hh
#define L1TMuonEndCap_EMTFBestTrackSelection_hh

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFCommon.hh"


class EMTFBestTrackSelection {
public:
  void configure(
      int verbose, int endcap, int sector, int bx,
      int bxWindow,
      int maxRoadsPerZone, int maxTracks, bool useSecondEarliest
  );

  void process(
      const std::deque<EMTFTrackExtraCollection>& extended_best_track_cands,
      EMTFTrackExtraCollection& best_tracks
  ) const;

  void cancel_one_bx(
      const std::deque<EMTFTrackExtraCollection>& extended_best_track_cands,
      EMTFTrackExtraCollection& best_tracks
  ) const;

  void cancel_multi_bx(
      const std::deque<EMTFTrackExtraCollection>& extended_best_track_cands,
      EMTFTrackExtraCollection& best_tracks
  ) const;

private:
  int verbose_, endcap_, sector_, bx_;

  int bxWindow_;
  int maxRoadsPerZone_, maxTracks_;
  bool useSecondEarliest_;
};

#endif
