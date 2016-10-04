#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPtAssignment.hh"

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPtAssignmentEngine.hh"
#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPtAssignmentHelper.hh"


void EMTFPtAssignment::configure(
    const EMTFPtAssignmentEngine* pt_assign_engine,
    int verbose, int endcap, int sector, int bx
) {
  //pt_assign_engine_ = pt_assign_engine;
  pt_assign_engine_ = const_cast<EMTFPtAssignmentEngine*>(pt_assign_engine);

  verbose_ = verbose;
  endcap_  = endcap;
  sector_  = sector;
  bx_      = bx;
}

void EMTFPtAssignment::process(
    EMTFTrackExtraCollection& best_tracks
) {
  using address_t = EMTFPtAssignmentEngine::address_t;

  const int ntracks = best_tracks.size();

  for (int i = 0; i < ntracks; ++i) {
    EMTFTrackExtra& track = best_tracks.at(i);  // pass by reference

    address_t address = pt_assign_engine_->calculate_address(track);
    float     xmlpt   = pt_assign_engine_->calculate_pt(address);
    float     pt      = xmlpt * 1.4; // Should do proper rounding (to 0.5) here. - AWB 03.10.16


    // compressed pt = pt*2 (scale) + 1 (pt = 0 is empty candidate)
    int gmt_pt = pt;
    gmt_pt = (gmt_pt*2) + 1;
    if (gmt_pt > 511)
      gmt_pt = 511;

    // convert phi into gmt scale according to DN15-017
    // full scale is -16 to 100, or 116 values, covers range -10 to 62.5 deg
    // my internal ph scale is 0..5000, covers from -22 to 63.333 deg
    // converted to GMT scale it is from -35 to 95
    // bt_phi * 107.01/4096, equivalent to bt_phi * 6849/0x40000
    int gmt_phi_mult = track.phi_int * 6849;
    int gmt_phi = (gmt_phi_mult>>18); // divide by 0x40000
    gmt_phi -= 35; // offset of -22 deg

    int gmt_eta = getGMTEta(track.theta_int, endcap_); // From EMTFPtAssignmentHelper.hh. Does this bit-wise match the FW? - AWB 03.10.16
    bool use_ones_complem_gmt_eta = true;
    if (use_ones_complem_gmt_eta) { // Is this the proper thing to do? - AWB 04.10.16
      gmt_eta = (gmt_eta < 0) ? ~(-gmt_eta) : gmt_eta;
    }

    int gmt_quality = getGMTQuality(track.mode, track.theta_int);

    std::vector<uint16_t> delta_ph(&(track.ptlut_data.delta_ph[0]), &(track.ptlut_data.delta_ph[0]) + 6);
    std::vector<uint16_t> sign_ph(&(track.ptlut_data.sign_ph[0]), &(track.ptlut_data.sign_ph[0]) + 6);
    std::pair<int,int> gmt_charge = getGMTCharge(track.mode, delta_ph, sign_ph);

    // _________________________________________________________________________
    // Output
    track.ptlut_address = address;
    track.pt_xml        = xmlpt;
    track.pt            = pt;

    track.gmt_pt           = gmt_pt;
    track.gmt_phi          = gmt_phi;
    track.gmt_eta          = gmt_eta;
    track.gmt_quality      = gmt_quality;
    track.gmt_charge       = gmt_charge.first;
    track.gmt_charge_valid = gmt_charge.second;
  }

  if (verbose_ > 0) {  // debug
    for (const auto& track: best_tracks) {
      std::cout << "track: " << track.winner << " pt address: " << track.ptlut_address << " pt: " << track.pt << " mode: " << track.mode << " GMT charge: " << track.gmt_charge << " quality: " << track.gmt_quality << " eta: " << track.gmt_eta << " phi: " << track.gmt_phi << std::endl;
    }
  }

}
