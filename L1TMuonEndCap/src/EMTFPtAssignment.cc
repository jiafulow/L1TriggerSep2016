#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPtAssignment.hh"

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPtAssignmentEngine.hh"
#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPtAssignmentHelper.hh"


void EMTFPtAssignment::configure(
    const EMTFPtAssignmentEngine* pt_assign_engine,
    int verbose, int endcap, int sector, int bx,
    bool readPtLUTFile, bool fixMode15HighPt, bool fix9bDPhi
) {
  //pt_assign_engine_ = pt_assign_engine;
  pt_assign_engine_ = const_cast<EMTFPtAssignmentEngine*>(pt_assign_engine);

  verbose_ = verbose;
  endcap_  = endcap;
  sector_  = sector;
  bx_      = bx;

  pt_assign_engine_->configure(
      verbose_,
      readPtLUTFile, fixMode15HighPt, fix9bDPhi
  );
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
    float     pt      = (xmlpt < 0.) ? 1. : xmlpt;  // Matt used fabs(-1) when mode is invalid
    pt *= 1.4;  // multiply by 1.4 to keep efficiency above 90% when the L1 trigger pT cut is applied

    // compressed pt = pt*2 (scale) + 1 (pt = 0 is empty candidate)
    int gmt_pt = (pt * 2) + 1;
    gmt_pt = (gmt_pt > 511) ? 511 : gmt_pt;

    pt = (gmt_pt <= 0) ?  0 : (gmt_pt-1) * 0.5;

    int gmt_phi = getGMTPhi(track.phi_int);

    int gmt_eta = getGMTEta(track.theta_int, endcap_);
    bool use_ones_complem_gmt_eta = true;
    if (use_ones_complem_gmt_eta) {
      gmt_eta = (gmt_eta < 0) ? ~(-gmt_eta) : gmt_eta;
    }

    int gmt_quality = getGMTQuality(track.mode, track.theta_int);

    std::vector<uint16_t> delta_ph(&(track.ptlut_data.delta_ph[0]), &(track.ptlut_data.delta_ph[0]) + NUM_STATION_PAIRS);
    std::vector<uint16_t> sign_ph(&(track.ptlut_data.sign_ph[0]), &(track.ptlut_data.sign_ph[0]) + NUM_STATION_PAIRS);
    std::pair<int,int> gmt_charge = getGMTCharge(track.mode, delta_ph, sign_ph);

    // _________________________________________________________________________
    // Output
    track.ptlut_address    = address;
    track.pt_xml           = xmlpt;
    track.pt               = pt;

    track.gmt_pt           = gmt_pt;
    track.gmt_phi          = gmt_phi;
    track.gmt_eta          = gmt_eta;
    track.gmt_quality      = gmt_quality;
    track.gmt_charge       = gmt_charge.first;
    track.gmt_charge_valid = gmt_charge.second;
  }

  if (verbose_ > 0) {  // debug
    for (const auto& track: best_tracks) {
      std::cout << "track: " << track.winner << " pt address: " << track.ptlut_address << " GMT pt: " << track.gmt_pt
          << " pt: " << track.pt << " mode: " << track.mode
          << " GMT charge: " << track.gmt_charge << " quality: " << track.gmt_quality
          << " eta: " << track.gmt_eta << " phi: " << track.gmt_phi
          << std::endl;
    }
  }

}
