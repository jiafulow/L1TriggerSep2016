#ifndef L1TMuonEndCap_EMTFAngleCalculation_hh
#define L1TMuonEndCap_EMTFAngleCalculation_hh

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFCommon.hh"


class EMTFAngleCalculation {
public:
  void configure(
      int verbose, int endcap, int sector, int bx,
      int bxWindow,
      int thetaWindow
  );

  void process(
      zone_array<EMTFTrackExtraCollection>& zone_tracks
  ) const;

  void calculate_angles(EMTFTrackExtra& track) const;

  void calculate_bx(EMTFTrackExtra& track) const;

  void erase_tracks(EMTFTrackExtraCollection& tracks) const;

private:
  int verbose_, endcap_, sector_, bx_;

  int bxWindow_;
  int thetaWindow_;
};

#endif
