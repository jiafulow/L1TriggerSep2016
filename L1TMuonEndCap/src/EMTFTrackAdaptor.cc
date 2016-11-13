#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFTrackAdaptor.hh"

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFTrackTools.hh"


EMTFTrackAdaptor::EMTFTrackAdaptor() {

}

EMTFTrackAdaptor::~EMTFTrackAdaptor() {

}

void EMTFTrackAdaptor::convert_hit(const EMTFHitExtra& in_hit, EMTFHit& out_hit) const {
  out_hit.set_endcap       ( (in_hit.endcap == 2) ? -1 : 1    );
  out_hit.set_station      ( in_hit.station       );
  out_hit.set_ring         ( in_hit.ring          );
  out_hit.set_sector       ( in_hit.sector        );
  out_hit.set_sector_index ( (in_hit.endcap-1)*6 + (in_hit.sector-1) );
  out_hit.set_subsector    ( in_hit.subsector     );
  out_hit.set_chamber      ( in_hit.chamber       );
  out_hit.set_csc_ID       ( in_hit.csc_ID        );
  out_hit.set_neighbor     ( (in_hit.pc_station == 5) ? 1 : 0 );
  out_hit.set_mpc_link     ( in_hit.mpc_link      );
  out_hit.set_wire         ( in_hit.wire          );
  out_hit.set_strip        ( in_hit.strip         );
  out_hit.set_track_num    ( in_hit.track_num     );
  out_hit.set_quality      ( in_hit.quality       );
  out_hit.set_pattern      ( in_hit.pattern       );
  out_hit.set_bend         ( in_hit.bend          );
  out_hit.set_valid        ( in_hit.valid         );
  out_hit.set_sync_err     ( in_hit.sync_err      );
  out_hit.set_bc0          ( in_hit.bc0           );
  out_hit.set_bx           ( in_hit.bx            );
  out_hit.set_stub_num     ( in_hit.stub_num      );
  out_hit.set_is_CSC_hit   ( (in_hit.subsystem == TriggerPrimitive::kCSC) ? 1 : 0 );
  out_hit.set_is_RPC_hit   ( (in_hit.subsystem == TriggerPrimitive::kRPC) ? 1 : 0 );
}

void EMTFTrackAdaptor::convert_track(const EMTFTrackExtra& in_track, EMTFTrack& out_track) const {
  namespace l1t = L1TMuonEndCap;

  for (const auto& in_hit : in_track.xhits) {
    EMTFHit out_hit;

    convert_hit(in_hit, out_hit);
    out_track.push_Hit(out_hit);

    if ( out_hit.Neighbor() == 0 ) out_track.set_all_neighbor(0);
    if ( out_hit.Neighbor() == 1 ) out_track.set_has_neighbor(1);
    if ( out_hit.Neighbor() == 0 && out_track.Has_neighbor() == -999 ) out_track.set_has_neighbor(0);
    if ( out_hit.Neighbor() == 1 && out_track.All_neighbor() == -999 ) out_track.set_all_neighbor(0);
  }

  //const int endcap = in_track.endcap;
  const int sector = in_track.sector;

  const EMTFPtLUTData& ptlut_data = in_track.ptlut_data;

  int theta_int = in_track.theta_int;
  float theta_angle = l1t::calc_theta_rad_from_int( theta_int );
  float eta = l1t::calc_eta_from_theta_rad( theta_angle );
  eta = (in_track.endcap == 2) ? -eta : eta;

  int phi_loc_int = in_track.phi_int;
  float phi_loc_deg = l1t::calc_phi_loc_deg( phi_loc_int );
  float phi_loc_rad = l1t::calc_phi_loc_rad( phi_loc_int );
  //float phi_loc_deg = l1t::calc_phi_loc_deg_corr( phi_loc_int, endcap );
  //float phi_loc_rad = l1t::calc_phi_loc_rad_corr( phi_loc_int, endcap );
  float phi_glob_deg = l1t::calc_phi_glob_deg( phi_loc_deg, sector );
  float phi_glob_rad = l1t::calc_phi_glob_rad( phi_loc_rad, sector );

  auto get_signed_int = [](int var, int sign) {
    return (sign == 1) ? (var * 1) : (var * -1);
  };

  auto adapt_dphi = [](int x) {
    return (x == 8191) ? -999 : x;
  };

  auto adapt_dtheta = [](int x) {
    return (x == 127) ? -999 : x;
  };

  out_track.set_endcap        ( (in_track.endcap == 2) ? -1 : 1       );
  out_track.set_sector        ( in_track.sector         );
  out_track.set_sector_GMT    ( in_track.sector-1       );
  out_track.set_sector_index  ( (in_track.endcap-1)*6 + (in_track.sector-1) );
  out_track.set_mode          ( in_track.mode           );
  out_track.set_mode_LUT      ( in_track.mode_inv       );
  out_track.set_quality       ( in_track.gmt_quality    );
  out_track.set_bx            ( in_track.bx             );
  out_track.set_pt            ( in_track.pt             );
  out_track.set_pt_GMT        ( in_track.gmt_pt         );
  out_track.set_pt_LUT_addr   ( in_track.ptlut_address  );
  out_track.set_eta           ( eta                     );
  out_track.set_eta_GMT       ( in_track.gmt_eta        );
  out_track.set_eta_LUT       ( in_track.theta_int >> 2 );
  out_track.set_phi_loc_int   ( phi_loc_int             );
  out_track.set_phi_loc_deg   ( phi_loc_deg             );
  out_track.set_phi_loc_rad   ( phi_loc_rad             );
  out_track.set_phi_GMT       ( in_track.gmt_phi        );
  out_track.set_phi_glob_deg  ( phi_glob_deg            );
  out_track.set_phi_glob_rad  ( phi_glob_rad            );
  out_track.set_charge        ( (in_track.gmt_charge == 1) ? -1 : 1   );
  out_track.set_charge_GMT    ( in_track.gmt_charge     );
  out_track.set_charge_valid  ( in_track.gmt_charge_valid );
  out_track.set_dPhi_12       ( adapt_dphi(get_signed_int(ptlut_data.delta_ph[0], ptlut_data.sign_ph[0]) ) );
  out_track.set_dPhi_13       ( adapt_dphi(get_signed_int(ptlut_data.delta_ph[1], ptlut_data.sign_ph[1]) ) );
  out_track.set_dPhi_14       ( adapt_dphi(get_signed_int(ptlut_data.delta_ph[2], ptlut_data.sign_ph[2]) ) );
  out_track.set_dPhi_23       ( adapt_dphi(get_signed_int(ptlut_data.delta_ph[3], ptlut_data.sign_ph[3]) ) );
  out_track.set_dPhi_24       ( adapt_dphi(get_signed_int(ptlut_data.delta_ph[4], ptlut_data.sign_ph[4]) ) );
  out_track.set_dPhi_34       ( adapt_dphi(get_signed_int(ptlut_data.delta_ph[5], ptlut_data.sign_ph[5]) ) );
  out_track.set_dTheta_12     ( adapt_dtheta(get_signed_int(ptlut_data.delta_th[0], 1-ptlut_data.sign_th[0]) ) );
  out_track.set_dTheta_13     ( adapt_dtheta(get_signed_int(ptlut_data.delta_th[1], 1-ptlut_data.sign_th[1]) ) );
  out_track.set_dTheta_14     ( adapt_dtheta(get_signed_int(ptlut_data.delta_th[2], 1-ptlut_data.sign_th[2]) ) );
  out_track.set_dTheta_23     ( adapt_dtheta(get_signed_int(ptlut_data.delta_th[3], 1-ptlut_data.sign_th[3]) ) );
  out_track.set_dTheta_24     ( adapt_dtheta(get_signed_int(ptlut_data.delta_th[4], 1-ptlut_data.sign_th[4]) ) );
  out_track.set_dTheta_34     ( adapt_dtheta(get_signed_int(ptlut_data.delta_th[5], 1-ptlut_data.sign_th[5]) ) );
  out_track.set_clct_1        ( ptlut_data.cpattern[0]  );
  out_track.set_clct_2        ( ptlut_data.cpattern[1]  );
  out_track.set_clct_3        ( ptlut_data.cpattern[2]  );
  out_track.set_clct_4        ( ptlut_data.cpattern[3]  );
  out_track.set_fr_1          ( ptlut_data.fr[0]        );
  out_track.set_fr_2          ( ptlut_data.fr[1]        );
  out_track.set_fr_3          ( ptlut_data.fr[2]        );
  out_track.set_fr_4          ( ptlut_data.fr[3]        );
  out_track.set_track_num     ( in_track.track_num      );
  //out_track.set_has_neighbor  ( 0                       );
  //out_track.set_all_neighbor  ( 0                       );
}

void EMTFTrackAdaptor::convert_all(
    const EMTFHitExtraCollection& in_hits, const EMTFTrackExtraCollection& in_tracks,
    EMTFHitCollection& out_hits, EMTFTrackCollection& out_tracks
) const {
  out_hits.clear();
  out_tracks.clear();

  for (const auto& in_hit : in_hits) {
    EMTFHit out_hit;

    convert_hit(in_hit, out_hit);
    out_hits.push_back(out_hit);
  }

  for (const auto& in_track : in_tracks) {
    EMTFTrack out_track;

    convert_track(in_track, out_track);
    out_tracks.push_back(out_track);
  }
}
