#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPrimitiveMatching.hh"

#include "helper.h"  // to_hex, to_binary

namespace {
  static const int bw_fph = 13;  // bit width of ph, full precision
  static const int bpow = 7;     // (1 << bpow) is count of input ranks
  static const int bw_th = 7;    // bit width of th
  static const int invalid_ph_diff = 31;
}


void EMTFPrimitiveMatching::configure(
    int endcap, int sector, int bx,
    int thetaWindow
) {
  endcap_ = endcap;
  sector_ = sector;
  bx_     = bx;

  thetaWindow_     = thetaWindow;
}

void EMTFPrimitiveMatching::match(
    const std::deque<EMTFHitExtraCollection>& extended_conv_hits,
    const std::vector<EMTFRoadExtraCollection>& zone_roads,
    std::vector<EMTFTrackExtraCollection>& zone_tracks
) const {

  // Organize converted hits by (zone, station)
  std::array<EMTFHitExtraCollection, NUM_ZONES*NUM_STATIONS> zs_conv_hits;

  bool use_fs_zone_code = true;  // use zone code as in firmware find_segment module

  // Loop over converted hits and fill the array
  std::deque<EMTFHitExtraCollection>::const_iterator ext_conv_hits_it  = extended_conv_hits.begin();
  std::deque<EMTFHitExtraCollection>::const_iterator ext_conv_hits_end = extended_conv_hits.end();

  for (; ext_conv_hits_it != ext_conv_hits_end; ++ext_conv_hits_it) {
    EMTFHitExtraCollection::const_iterator conv_hits_it  = ext_conv_hits_it->begin();
    EMTFHitExtraCollection::const_iterator conv_hits_end = ext_conv_hits_it->end();

    for (; conv_hits_it != conv_hits_end; ++conv_hits_it) {
      const EMTFHitExtra& conv_hit = *conv_hits_it;

      // A hit can go into multiple zones
      int istation = conv_hit.station-1;

      int zone_code = conv_hit.zone_code;  // decide based on original zone code
      if (use_fs_zone_code)
        zone_code = get_fs_zone_code(conv_hit);  // decide based on new zone code

      for (int izone = 0; izone < NUM_ZONES; ++izone) {
        if (zone_code & (1<<izone)) {
          const int zs = (izone*4) + istation;
          zs_conv_hits.at(zs).push_back(conv_hit);
        }
      }

    }  // end loop over conv_hits
  }  // end loop over extended_conv_hits

  if (true) {  // debug
    for (int izone = 0; izone < NUM_ZONES; ++izone) {
      for (int istation = 0; istation < NUM_STATIONS; ++istation) {
        const int zs = (izone*4) + istation;
        for (const auto& conv_hit : zs_conv_hits.at(zs)) {
          std::cout << "z: " << izone << " st: " << istation+1 << " cscid: " << conv_hit.csc_ID << " ph_seg: " << conv_hit.phi_fp << " ph_seg_red: " << (conv_hit.phi_fp>>((bw_fph-bpow-1)))<< std::endl;
        }
      }
    }
  }


  // Keep the best phi difference for every road by (zone, station)
  std::array<std::vector<std::pair<int, int> >, NUM_ZONES*NUM_STATIONS> zs_phi_differences;

  // Get the best-matching hits by comparing phi difference between
  // pattern and segment
  for (int izone = 0; izone < NUM_ZONES; ++izone) {
    for (int istation = 0; istation < NUM_STATIONS; ++istation) {
      const int zs = (izone*4) + istation;

      match_single_zone_station(
          istation + 1,
          zone_roads.at(izone),
          zs_conv_hits.at(zs),
          zs_phi_differences.at(zs)
      );

      assert(zone_roads.at(izone).size() == zs_phi_differences.at(zs).size());
    }  // end loop over stations
  }  // end loop over zones

  if (true) {  // debug
    for (int izone = 0; izone < NUM_ZONES; ++izone) {
      for (int istation = 0; istation < NUM_STATIONS; ++istation) {
        const int zs = (izone*4) + istation;
        int i = 0;
        for (const auto& ph_diff_pair : zs_phi_differences.at(zs)) {
          std::cout << "z: " << izone << " r: " << zone_roads.at(izone).at(i).winner << " ph_num: " << zone_roads.at(izone).at(i).ph_num << " st: " << istation+1 << " ichit: " << ph_diff_pair.first << " ph_diff: " << ph_diff_pair.second << std::endl;
          ++i;
        }
      }
    }
  }


  // Build all tracks in each zone
  zone_tracks.clear();
  zone_tracks.resize(NUM_ZONES);

  for (int izone = 0; izone < NUM_ZONES; ++izone) {
    const EMTFRoadExtraCollection& roads = zone_roads.at(izone);
    const int nroads = roads.size();

    for (int iroad = 0; iroad < nroads; ++iroad) {
      const EMTFRoadExtra& road = roads.at(iroad);

      // Create a track
      EMTFTrackExtra track;
      track.endcap   = road.endcap;
      track.sector   = road.sector;
      track.bx       = road.bx;

      track.mode      = 0;
      track.num_xhits = 0;

      track.xhits        .clear();
      track.xhits_ph_diff.clear();
      track.xhits_valid  .clear();

      track.xhits        .resize(10, EMTFHitExtra());
      track.xhits_ph_diff.resize(10, invalid_ph_diff);
      track.xhits_valid  .resize(10, false);

      for (int istation = 0; istation < NUM_STATIONS; ++istation) {
        const int zs = (izone*4) + istation;

        const EMTFHitExtraCollection& conv_hits = zs_conv_hits.at(zs);
        int ichit   = zs_phi_differences.at(zs).at(iroad).first;
        int ph_diff = zs_phi_differences.at(zs).at(iroad).second;

        if (ph_diff != invalid_ph_diff) {
          insert_hit(istation+1, ichit, ph_diff, conv_hits, track);
        }
      }

      //track.road = static_cast<EMTFRoad>(road);
      track.xroad = road;

      calculate_angles(track);

      // Output track
      zone_tracks.at(izone).push_back(track);

    }  // end loop over roads

    assert(zone_tracks.size() == zone_roads.size());
  }  // end loop over zones

  if (true) {  // debug
    for (const auto& tracks : zone_tracks) {
      for (const auto& track : tracks) {
        int i = 0;
        for (const auto& xhit : track.xhits) {
          if (track.xhits_valid.at(i)) {
            int fs_segment = get_fs_segment(xhit);
            std::cout << "match seg: z: " << track.xroad.zone << " pat: " << track.xroad.winner <<  " st: " << xhit.station
                << " vi: " << to_binary(0b1, 2) << " hi: " << ((fs_segment>>4) & 0x3)
                << " ci: " << ((fs_segment>>1) & 0x7) << " si: " << (fs_segment & 0x1)
                << " ph: " << xhit.phi_fp << " th: " << xhit.theta_fp
                << std::endl;
          }
          ++i;
        }
      }
    }

    for (const auto& tracks : zone_tracks) {
      for (const auto& track : tracks) {
        if (track.rank) {
          std::cout << "deltas: z: " << track.xroad.zone << " pat: " << track.xroad.winner << " rank: " << to_hex(track.rank)
              << " delta_ph: " << array_string(track.ptlut_data.delta_ph)
              << " delta_th: " << array_string(track.ptlut_data.delta_th)
              << " sign_ph: " << array_string(track.ptlut_data.sign_ph)
              << " sign_th: " << array_string(track.ptlut_data.sign_th)
              << " phi: " << track.phi_int << " theta: " << track.theta_int
              << std::endl;
        }
      }
    }
  }

}

void EMTFPrimitiveMatching::match_single_zone_station(
    int station,
    const EMTFRoadExtraCollection& roads,
    const EMTFHitExtraCollection& conv_hits,
    std::vector<std::pair<int, int> >& phi_differences
) const {
  const int max_ph_diff = (station == 1) ? 15 : 7;  // max phi difference
  //const int invalid_ph_diff = (station == 1) ? 31 : 15;

  const int nroads = roads.size();
  const int nchits = conv_hits.size();

  for (int iroad = 0; iroad < nroads; ++iroad) {
    int ph_pat = roads.at(iroad).ph_num;  // ph detected in pattern
    int ph_q   = roads.at(iroad).ph_q;
    assert(ph_pat >= 0 && ph_q > 0);

    std::vector<std::pair<int, int> > tmp_phi_differences;

    for (int ichit = 0; ichit < nchits; ++ichit) {
      int ph_seg     = conv_hits.at(ichit).phi_fp;  // ph from segments
      int ph_seg_red = ph_seg >> (bw_fph-bpow-1);  // remove unused low bits
      assert(ph_seg >= 0);

      // Get abs difference
      int ph_diff = (ph_pat > ph_seg_red) ? (ph_pat - ph_seg_red) : (ph_seg_red - ph_pat);

      if (ph_diff > max_ph_diff) {
        ph_diff = invalid_ph_diff;  // difference is too high, cannot be the same pattern
      }

      tmp_phi_differences.push_back(std::make_pair(ichit, ph_diff));  // make a key-value pair
    }

    if (!tmp_phi_differences.empty()) {
      // Find best phi difference
      sort_ph_diff(tmp_phi_differences);

      // Store the best phi difference
      phi_differences.push_back(tmp_phi_differences.front());

    } else {
      // No segment found
      phi_differences.push_back(std::make_pair(0, invalid_ph_diff));
    }

  }  // end loop over roads
}

void EMTFPrimitiveMatching::sort_ph_diff(std::vector<std::pair<int, int> >& phi_differences) const {
  // Sort by value, but preserving the original order
  struct {
    typedef std::pair<int, int> value_type;
    constexpr bool operator()(const value_type& lhs, const value_type& rhs) {
      return lhs.second < rhs.second;
    }
  } less_ph_diff_cmp;

  std::stable_sort(phi_differences.begin(), phi_differences.end(), less_ph_diff_cmp);

  // Maybe implement the firmware algorithm here?
}

void EMTFPrimitiveMatching::insert_hits(
    int station, int ichit, int ph_diff, const EMTFHitExtraCollection& conv_hits,
    EMTFTrackExtra& track
) const {
  // First, insert the hit
  insert_hit(station, ichit, ph_diff, conv_hits, track);

  const int nchits = conv_hits.size();

  // Second, find possible duplicated hit, insert that too
  for (int jchit = 0; jchit < nchits; ++jchit) {
    if (jchit == ichit)
      continue;

    const EMTFHitExtra& conv_hit_i = conv_hits.at(ichit);
    const EMTFHitExtra& conv_hit_j = conv_hits.at(jchit);

    if (
      (conv_hit_i.pc_station == conv_hit_j.pc_station) &&
      (conv_hit_i.pc_chamber == conv_hit_j.pc_chamber) &&
      (conv_hit_i.strip      == conv_hit_j.strip) &&
      (conv_hit_i.pattern    == conv_hit_j.pattern)
    ) {
      // Must have the same phi_fp
      assert(conv_hit_i.phi_fp == conv_hit_j.phi_fp);

      insert_hit(station, jchit, ph_diff, conv_hits, track);
    }
  }
}

void EMTFPrimitiveMatching::insert_hit(
    int station, int ichit, int ph_diff, const EMTFHitExtraCollection& conv_hits,
    EMTFTrackExtra& track
) const {
  static const int _table[8] = {
    0, 4, 6, 8,  // ibegin for st1,2,3,4
    4, 6, 8, 10  // iend for st1,2,3,4
  };

  const EMTFHitExtra& conv_hit = conv_hits.at(ichit);
  //const EMTFHit& conv_hit = static_cast<EMTFHit>(conv_hits.at(ichit));

  unsigned istation = station-1;
  assert(istation < 4);
  int ibegin = _table[istation];
  int iend   = _table[istation+4];
  bool ins_success = false;

  for (int i = ibegin; i < iend; ++i) {
    if (track.xhits_valid.at(i) == true)
      continue;

    track.xhits.at(i)          = conv_hit;
    track.xhits_ph_diff.at(i)  = ph_diff;
    track.xhits_valid.at(i)    = true;
    track.num_xhits           += 1;
    track.mode                |= (1<<(4-station));

    ins_success = true;
    break;
  }

  assert(ins_success && "Failed to insert EMTFHitExtra into EMTFTrackExtra");
}

void EMTFPrimitiveMatching::calculate_angles(EMTFTrackExtra& track) const {
  static const int _table[8] = {
    0, 4, 6, 8,  // ibegin for st1,2,3,4
    4, 6, 8, 10  // iend for st1,2,3,4
  };

  // Fold track.xhits, a vector of EMTFHits, into a vector of vector of EMTFHits
  // with index [station][num]
  std::vector<EMTFHitExtraCollection> st_conv_hits;

  for (int istation = 0; istation < NUM_STATIONS; ++istation) {
    st_conv_hits.push_back(EMTFHitExtraCollection());
    int ibegin = _table[istation];
    int iend   = _table[istation+NUM_STATIONS];

    for (int i = ibegin; i < iend; ++i) {
      if (track.xhits_valid.at(i))
        st_conv_hits.back().push_back(track.xhits.at(i));
    }
  }
  assert(st_conv_hits.size() == NUM_STATIONS);

  const int invalid_dtheta = (1<<bw_th) - 1;  // = 127
  const int invalid_dphi = (1<<bw_fph) - 1;   // = 8191

  // Best theta deltas and phi deltas
  // from 0 to 5: dphi12, dphi13, dphi14, dphi23, dphi24, dphi34
  std::array<int,  NUM_STATION_PAIRS> best_dtheta_arr;
  std::array<int,  NUM_STATION_PAIRS> best_dtheta_sign_arr;
  std::array<int,  NUM_STATION_PAIRS> best_dphi_arr;
  std::array<int,  NUM_STATION_PAIRS> best_dphi_sign_arr;

  best_dtheta_arr.fill(invalid_dtheta);
  best_dtheta_sign_arr.fill(0);
  best_dphi_arr.fill(invalid_dphi);
  best_dphi_sign_arr.fill(0);

  // For phi and theta assignment
  // from 0 to 5: ME2,  ME3,  ME4,  ME2,  ME2,  ME3
  //              (stB),(stB),(stB),(stA),(stA),(stA)
  std::array<int,  NUM_STATION_PAIRS> best_phi_arr;
  std::array<int,  NUM_STATION_PAIRS> best_theta_arr;
  std::array<bool, NUM_STATION_PAIRS> best_valid_arr;

  best_phi_arr.fill(0);
  best_theta_arr.fill(0);
  best_valid_arr.fill(false);

  // Calculate angles
  int ipair = 0;

  for (int ist1 = 0; ist1 < NUM_STATIONS-1; ++ist1) {  // station A
    for (int ist2 = ist1+1; ist2 < NUM_STATIONS; ++ist2) {  // station B
      const EMTFHitExtraCollection& conv_hitsA = st_conv_hits.at(ist1);
      const EMTFHitExtraCollection& conv_hitsB = st_conv_hits.at(ist2);

      for (const auto& conv_hitA : conv_hitsA) {
        for (const auto& conv_hitB : conv_hitsB) {
          // Calculate theta deltas
          int thA = conv_hitA.theta_fp;
          int thB = conv_hitB.theta_fp;
          int dth = (thA > thB) ? thA - thB : thB - thA;
          int dth_sign = (thA > thB);  // sign

          if (best_dtheta_arr.at(ipair) >= dth) {
            best_dtheta_arr.at(ipair) = dth;
            best_dtheta_sign_arr.at(ipair) = dth_sign;

            best_theta_arr.at(ipair) = (ipair < 3) ? thB : thA;
            best_valid_arr.at(ipair) = true;
          }

          // Calculate phi deltas
          int phA = conv_hitA.phi_fp;
          int phB = conv_hitB.phi_fp;
          int dph = (phA > phB) ? phA - phB : phB - phA;
          int dph_sign = (phA <= phB);  // sign reversed according to Matt's oral request 2016-04-27

          if (best_valid_arr.at(ipair)) {
            best_dphi_arr.at(ipair) = dph;
            best_dphi_sign_arr.at(ipair) = dph_sign;

            best_phi_arr.at(ipair) = (ipair < 3) ? phB : phA;
          }
        }
      }
      ++ipair;
    }  // end loop over station B
  }  // end loop over station A


  // Find valid segments
  int vmask1 = 0;
  int vmask2 = 0;
  int vmask3 = 0;

  // vmask contains valid station mask = {ME4,ME3,ME2,ME1}
  if (best_dtheta_arr.at(0) <= thetaWindow_)
    vmask1 |= 0b0011;  // 12
  if (best_dtheta_arr.at(1) <= thetaWindow_)
    vmask1 |= 0b0101;  // 13
  if (best_dtheta_arr.at(2) <= thetaWindow_)
    vmask1 |= 0b1001;  // 14
  if (best_dtheta_arr.at(3) <= thetaWindow_)
    vmask2 |= 0b0110;  // 23
  if (best_dtheta_arr.at(4) <= thetaWindow_)
    vmask2 |= 0b1010;  // 24
  if (best_dtheta_arr.at(5) <= thetaWindow_)
    vmask3 |= 0b1100;  // 34

  // merge station masks only if they share bits
  int vstat = vmask1;
  if ((vstat & vmask2) != 0 || vstat == 0)
    vstat |= vmask2;
  if ((vstat & vmask3) != 0 || vstat == 0)
    vstat |= vmask3;

  // remove some valid flags if th did not line up
  for (int istation = 0; istation < NUM_STATIONS; ++istation) {
    int ibegin = _table[istation];
    int iend   = _table[istation+4];

    if ((vstat & (1<<istation)) == 0) {  // station bit not set
      //for (int i = ibegin; i < iend; ++i) {
      //  track.xhits_valid.at(i) = false;
      //}
      st_conv_hits.at(istation).clear();
    }
  }

  // assign precise phi and theta
  int phi_int = 0;
  int theta_int = 0;

  if ((vstat & (1<<1)) != 0) {          // ME2 present
    // phi is simple, we have it
    phi_int = st_conv_hits.at(1).front().phi_fp;

    // for theta, select delta to best station
    assert(best_valid_arr.at(0) || best_valid_arr.at(3) || best_valid_arr.at(4));
    if (best_valid_arr.at(0)) {         // 12
      theta_int = best_theta_arr.at(0);
    } else if (best_valid_arr.at(3)) {  // 23
      theta_int = best_theta_arr.at(3);
    } else if (best_valid_arr.at(4)) {  // 24
      theta_int = best_theta_arr.at(4);
    }

  } else if ((vstat & (1<<2)) != 0) {   // ME3 present
    phi_int = st_conv_hits.at(2).front().phi_fp;

    assert(best_valid_arr.at(1) || best_valid_arr.at(5));
    if (best_valid_arr.at(1)) {         // 13
      theta_int = best_theta_arr.at(1);
    } else if (best_valid_arr.at(5)) {  // 34
      theta_int = best_theta_arr.at(5);
    }

  } else if ((vstat & (1<<3)) != 0) {   // ME4 present
    phi_int = st_conv_hits.at(3).front().phi_fp;

    assert(best_valid_arr.at(2));
    if (best_valid_arr.at(2)) {         // 14
      theta_int = best_theta_arr.at(2);
    }
  }

  // update rank taking into account available stations after theta deltas
  // keep straightness as it was
  int rank = (track.xroad.quality_code << 1);  // output rank is one bit longer than input, to accomodate ME4 separately
  int rank2 = (
      (((rank>>6)  & 1) << 6) |  // straightness
      (((rank>>4)  & 1) << 4) |  // straightness
      (((rank>>2)  & 1) << 2) |  // straightness
      (((vstat>>0) & 1) << 5) |  // ME1
      (((vstat>>1) & 1) << 3) |  // ME2
      (((vstat>>2) & 1) << 1) |  // ME3
      (((vstat>>3) & 1) << 0)    // ME4
  );

  // if less than 2 segments, kill rank
  if (vstat == 0b0001 || vstat == 0b0010 || vstat == 0b0100 || vstat == 0b1000 || vstat == 0)
    rank2 = 0;

  // from RecoMuon/DetLayers/src/MuonCSCDetLayerGeometryBuilder.cc
  auto isFront = [](int station, int ring, int chamber) {
    bool result = false;
    bool isOverlapping = !(station == 1 && ring == 3);
    // not overlapping means back
    if(isOverlapping)
    {
      bool isEven = (chamber%2==0);
      // odd chambers are bolted to the iron, which faces
      // forward in 1&2, backward in 3&4, so...
      result = (station<3) ? isEven : !isEven;
    }
    return result;
  };

  // ___________________________________________________________________________
  // Output

  track.rank       = rank2;
  track.phi_int    = phi_int;
  track.theta_int  = theta_int;

  EMTFPtLUTData ptlut_data;
  for (int i = 0; i < NUM_STATION_PAIRS; ++i) {
    ptlut_data.delta_ph[i] = best_dphi_arr.at(i);
    ptlut_data.delta_th[i] = best_dtheta_arr.at(i);
    ptlut_data.sign_ph[i]  = best_dphi_sign_arr.at(i);
    ptlut_data.sign_th[i]  = best_dtheta_sign_arr.at(i);
  }
  for (int i = 0; i < NUM_STATIONS; ++i) {
    const auto& v = st_conv_hits.at(i);
    ptlut_data.cpattern[i] = v.empty() ? 0 : v.front().pattern;
    ptlut_data.fr[i]       = v.empty() ? 0 : isFront(v.front().station, v.front().ring, v.front().chamber);
  }

  track.ptlut_data = ptlut_data;

}

unsigned int EMTFPrimitiveMatching::get_fs_zone_code(const EMTFHitExtra& conv_hit) const {
  static const int _table[4][3] = {  // [station][ring]
    {0b0011, 0b0100, 0b1000},  // st1 r1: [z0,z1], r2: [z2], r3: [z3]
    {0b0011, 0b1100, 0b0000},  // st2 r1: [z0,z1], r2: [z2,z3]
    {0b0001, 0b1110, 0b0000},  // st3 r1: [z0], r2: [z1,z2,z3]
    {0b0001, 0b0110, 0b0000}   // st4 r1: [z0], r2: [z1,z2]
  };

  unsigned istation = conv_hit.station-1;
  unsigned iring    = (conv_hit.station == 1 && conv_hit.ring == 4) ? conv_hit.ring-4 : conv_hit.ring-1;
  assert(istation < 4 && iring < 3);
  return _table[istation][iring];
}

unsigned int EMTFPrimitiveMatching::get_fs_segment(const EMTFHitExtra& conv_hit) const {
  bool is_neighbor = (conv_hit.pc_station == 5);
  bool is_ring1    = (conv_hit.ring == 1);
  bool is_me1      = (conv_hit.station == 1);
  bool is_sub1     = (conv_hit.subsector == 1);

  int fs_history = bx_ - conv_hit.bx;  // history id
  int fs_chamber = -1;                 // chamber id
  int fs_segment = 0;                  // segment id, not emulated

  // For station 1
  //   j = 0 is neighbor sector chamber
  //   j = 1,2,3 are subsector 1 chambers
  //   j = 4,5,6 are subsector 2 chambers
  // For stations 2,3,4:
  //   j = 0 is neighbor sector chamber
  //   j = 1,2,3,4,5,6 are native sector chambers

  if (is_me1) {
    if (conv_hit.ring == 1 || conv_hit.ring == 4) {
      fs_chamber = is_neighbor ? 0 : (is_sub1 ? conv_hit.csc_ID : conv_hit.csc_ID+3);
    } else if (conv_hit.ring == 2) {
      fs_chamber = is_neighbor ? 0 : (is_sub1 ? conv_hit.csc_ID-3 : conv_hit.csc_ID-3+3);
    } else if (conv_hit.ring == 3) {
      fs_chamber = is_neighbor ? 0 : (is_sub1 ? conv_hit.csc_ID-6 : conv_hit.csc_ID-6+3);
    }
  } else {
    fs_chamber = is_neighbor ? 0 : (is_ring1 ? conv_hit.csc_ID : conv_hit.csc_ID-3);
  }
  assert(fs_chamber != -1);

  fs_segment = ((fs_history & 0x3)<<4) | ((fs_chamber & 0x7)<<1) | (fs_segment & 0x1);
  return fs_segment;
}

