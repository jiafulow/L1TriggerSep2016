#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPtAssignment.hh"

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPtAssignmentEngine.hh"
#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPtAssignmentHelper.hh"


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
    float     xmlpt   = pt_assign_engine_->calculate_pt(address, track);


    // convert phi into gmt scale according to DN15-017
    // full scale is -16 to 100, or 116 values, covers range -10 to 62.5 deg
    // my internal ph scale is 0..5000, covers from -22 to 63.333 deg
    // converted to GMT scale it is from -35 to 95
    // bt_phi * 107.01/4096, equivalent to bt_phi * 6849/0x40000
    int gmt_phi_mult = track.phi_int * 6849;
    int gmt_phi = (gmt_phi_mult>>18); // divide by 0x40000
    gmt_phi -= 35; // offset of -22 deg

    int gmt_eta = getGMTEta(track.theta_int, endcap_);

    int gmt_quality = getGMTQuality(track.mode, track.theta_int);

    const EMTFPtLUTData& ptlut_data = track.ptlut_data;
    int gmt_charge = getGMTCharge(ptlut_data.ph[0], ptlut_data.ph[1], ptlut_data.ph[2], ptlut_data.ph[3], track.mode);

    // _________________________________________________________________________
    // Output
    track.ptlut_address = address;
    track.pt_xml        = xmlpt;
    track.pt            = xmlpt*1.4;

    track.gmt_phi       = gmt_phi;
    track.gmt_eta       = gmt_eta;
    track.gmt_quality   = gmt_quality;
    track.gmt_charge    = gmt_charge;
  }

  if (true) {  // debug
    for (const auto& track: best_tracks) {
      std::cout << "track: " << track.winner << " pt address: " << track.ptlut_address << " pt: " << track.pt << " mode: " << track.mode << " GMT charge: " << track.gmt_charge << " quality: " << track.gmt_quality << " eta: " << track.gmt_eta << " phi: " << track.gmt_phi << std::endl;
    }
  }

}
