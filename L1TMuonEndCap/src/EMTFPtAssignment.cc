#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPtAssignment.hh"

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPtAssignmentEngine.hh"


void EMTFPtAssignment::configure(
    const EMTFPtAssignmentEngine* pt_assign_engine,
    int verbose, int endcap, int sector, int bx,
    bool readPtLUTFile, bool fixMode15HighPt,
    bool bug9BitDPhi, bool bugMode7CLCT, bool bugNegPt
) {
  assert(pt_assign_engine != nullptr);

  //pt_assign_engine_ = pt_assign_engine;
  pt_assign_engine_ = const_cast<EMTFPtAssignmentEngine*>(pt_assign_engine);

  verbose_ = verbose;
  endcap_  = endcap;
  sector_  = sector;
  bx_      = bx;

  pt_assign_engine_->configure(
      verbose_,
      readPtLUTFile, fixMode15HighPt,
      bug9BitDPhi, bugMode7CLCT, bugNegPt
  );
}

void EMTFPtAssignment::process(
    EMTFTrackExtraCollection& best_tracks
) {
  using address_t = EMTFPtAssignmentEngine::address_t;

  EMTFTrackExtraCollection::iterator best_tracks_it  = best_tracks.begin();
  EMTFTrackExtraCollection::iterator best_tracks_end = best_tracks.end();

  for (; best_tracks_it != best_tracks_end; ++best_tracks_it) {
    EMTFTrackExtra& track = *best_tracks_it;  // pass by reference

    address_t address = pt_assign_engine_->calculate_address(track);
    float     xmlpt   = pt_assign_engine_->calculate_pt(address);
    float     pt      = (xmlpt < 0.) ? 1. : xmlpt;  // Matt used fabs(-1) when mode is invalid
    pt *= 1.4;  // multiply by 1.4 to keep efficiency above 90% when the L1 trigger pT cut is applied

    int gmt_pt = aux().getGMTPt(pt);            // Encode integer pT in GMT format
    pt = (gmt_pt <= 0) ?  0 : (gmt_pt-1) * 0.5; // Decode integer pT (result is in 0.5 GeV step)

    int gmt_phi = aux().getGMTPhi(track.phi_int);

    int gmt_eta = aux().getGMTEta(track.theta_int, endcap_);  // Convert to integer eta using FW LUT

    // Explanation from Alex:
    // When using two's complement, you get two eta bins with zero coordinate.
    // This peculiarity is created because positive and negative endcaps are
    // processed by separate processors, so each of them gets its own zero bin.
    // With simple inversion, the eta scale becomes uniform, one bin for one
    // eta value.
    bool use_ones_complem_gmt_eta = true;
    if (use_ones_complem_gmt_eta) {
      gmt_eta = (gmt_eta < 0) ? ~(-gmt_eta) : gmt_eta;
    }

    int gmt_quality = aux().getGMTQuality(track.mode, track.theta_int);

    std::vector<int> phidiffs;
    for (int i = 0; i < NUM_STATION_PAIRS; ++i) {
      int phidiff = (track.ptlut_data.sign_ph[i] == 1) ? track.ptlut_data.delta_ph[i] : -track.ptlut_data.delta_ph[i];
      phidiffs.push_back(phidiff);
    }
    std::pair<int,int> gmt_charge = aux().getGMTCharge(track.mode, phidiffs);

    // _________________________________________________________________________
    // Output
    track.ptlut_address    = address;
    track.xmlpt            = xmlpt;
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

const EMTFPtAssignmentEngineAux& EMTFPtAssignment::aux() const {
  return pt_assign_engine_->aux();
}
