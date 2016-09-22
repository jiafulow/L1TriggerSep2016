#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPtAssignment.hh"

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPtAssignmentEngine.hh"


void EMTFPtAssignment::configure(
    const EMTFPtAssignmentEngine* pt_assign_engine,
    int endcap, int sector, int bx
) {
  //pt_assign_engine_ = pt_assign_engine;
  pt_assign_engine_ = const_cast<EMTFPtAssignmentEngine*>(pt_assign_engine);

  endcap_ = endcap;
  sector_ = sector;
  bx_     = bx;
}

void EMTFPtAssignment::assign(EMTFTrackExtraCollection& best_tracks) {
  using address_t = EMTFPtAssignmentEngine::address_t;

  const int ntracks = best_tracks.size();

  for (int i = 0; i < ntracks; ++i) {
    EMTFTrackExtra& track = best_tracks.at(i);  // pass by reference

    address_t address = pt_assign_engine_->calculate_address(track);
    float     xmlpt   = pt_assign_engine_->calculate_pt(address);

    // gmt_phi
    // gmt_eta
    // gmt_quality
    // gmt_charge

    // _________________________________________________________________________
    // Output
    track.ptlut_address = address;
    track.pt_xml        = xmlpt;
    track.pt            = xmlpt*1.4;
  }

}
