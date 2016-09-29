#ifndef L1TMuonEndCap_EMTFAngleCalculation_hh
#define L1TMuonEndCap_EMTFAngleCalculation_hh

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFCommon.hh"


class EMTFAngleCalculation {
public:
  void configure(
      int verbose, int endcap, int sector, int bx,
      int thetaWindow
  );

  void process(
      std::vector<EMTFTrackExtraCollection>& zone_tracks
  ) const;

  void calculate_angles(EMTFTrackExtra& track) const;

  void erase_tracks(EMTFTrackExtraCollection& tracks) const;

  int get_bt_chamber(const EMTFHitExtra& conv_hit) const;

private:
  int verbose_, endcap_, sector_, bx_;

  int thetaWindow_;
};

#endif
