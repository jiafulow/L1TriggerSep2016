#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPrimitiveMatching.hh"

#include "helper.hh"  // to_hex, to_binary

namespace {
  static const int bw_fph = 13;  // bit width of ph, full precision
  static const int bpow = 7;     // (1 << bpow) is count of input ranks
  static const int invalid_ph_diff = 0x1ff;  // 512 (9-bit)
}


void EMTFPrimitiveMatching::configure(
    int verbose, int endcap, int sector, int bx,
    bool fixZonePhi
) {
  verbose_ = verbose;
  endcap_  = endcap;
  sector_  = sector;
  bx_      = bx;

  fixZonePhi_      = fixZonePhi;
}

void EMTFPrimitiveMatching::process(
    const std::deque<EMTFHitExtraCollection>& extended_conv_hits,
    const zone_array<EMTFRoadExtraCollection>& zone_roads,
    zone_array<EMTFTrackExtraCollection>& zone_tracks
) const {
  int num_roads = 0;
  for (const auto& roads : zone_roads)
    num_roads += roads.size();
  bool early_exit = (num_roads == 0);

  if (early_exit)
    return;


  if (verbose_ > 0) {  // debug
    for (const auto& roads : zone_roads) {
      for (const auto& road : roads) {
        std::cout << "pattern on match input: z: " << road.zone << " r: " << road.winner
            << " ph_num: " << road.ph_num << " ph_q: " << to_hex(road.quality_code)
            << " ly: " << to_binary(road.layer_code, 3) << " str: " << to_binary(road.straightness, 3)
            << std::endl;
      }
    }
  }

  // Organize converted hits by (zone, station)
  std::array<EMTFHitExtraCollection, NUM_ZONES*NUM_STATIONS> zs_conv_hits;

  bool use_fs_zone_code = true;  // use zone code as in firmware find_segment module (Why? - AWB 07.10.16)
  use_fs_zone_code = false;

  std::deque<EMTFHitExtraCollection>::const_iterator ext_conv_hits_it  = extended_conv_hits.begin();
  std::deque<EMTFHitExtraCollection>::const_iterator ext_conv_hits_end = extended_conv_hits.end();

  for (; ext_conv_hits_it != ext_conv_hits_end; ++ext_conv_hits_it) {
    EMTFHitExtraCollection::const_iterator conv_hits_it  = ext_conv_hits_it->begin();
    EMTFHitExtraCollection::const_iterator conv_hits_end = ext_conv_hits_it->end();

    for (; conv_hits_it != conv_hits_end; ++conv_hits_it) {
      int istation = conv_hits_it->station-1;
      int zone_code = conv_hits_it->zone_code;  // decide based on original zone code
      if (use_fs_zone_code)
        zone_code = get_fs_zone_code(*conv_hits_it);  // decide based on new zone code

      // A hit can go into multiple zones
      for (int izone = 0; izone < NUM_ZONES; ++izone) {
        if (zone_roads.at(izone).size() > 0) {

          if (zone_code & (1<<izone)) {
            const int zs = (izone*NUM_STATIONS) + istation;
            zs_conv_hits.at(zs).push_back(*conv_hits_it);
          }
        }
      }

    }  // end loop over conv_hits
  }  // end loop over extended_conv_hits

  if (verbose_ > 1) {  // debug
    for (int izone = 0; izone < NUM_ZONES; ++izone) {
      for (int istation = 0; istation < NUM_STATIONS; ++istation) {
        const int zs = (izone*NUM_STATIONS) + istation;
        for (const auto& conv_hit : zs_conv_hits.at(zs)) {
          std::cout << "z: " << izone << " st: " << istation+1 << " cscid: " << conv_hit.csc_ID
              << " ph_zone_phi: " << conv_hit.zone_hit << " ph_low_prec: " << (conv_hit.zone_hit<<5)
              << " ph_high_prec: " << conv_hit.phi_fp << " ph_high_low_diff: " << (conv_hit.phi_fp - (conv_hit.zone_hit<<5))
              << std::endl;
        }
      }
    }
  }

  // Keep the best phi difference for every road by (zone, station)
  std::array<std::vector<hit_sort_pair_t>, NUM_ZONES*NUM_STATIONS> zs_phi_differences;

  // Get the best-matching hits by comparing phi difference between
  // pattern and segment
  for (int izone = 0; izone < NUM_ZONES; ++izone) {
    for (int istation = 0; istation < NUM_STATIONS; ++istation) {
      const int zs = (izone*NUM_STATIONS) + istation;

      process_single_zone_station(
          istation + 1,
          zone_roads.at(izone),
          zs_conv_hits.at(zs),
          zs_phi_differences.at(zs)
      );

      assert(zone_roads.at(izone).size() == zs_phi_differences.at(zs).size());
    }  // end loop over stations
  }  // end loop over zones

  if (verbose_ > 1) {  // debug
    for (int izone = 0; izone < NUM_ZONES; ++izone) {
      const auto& roads = zone_roads.at(izone);
      for (unsigned iroad = 0; iroad < roads.size(); ++iroad) {
        const auto& road = roads.at(iroad);
        for (int istation = 0; istation < NUM_STATIONS; ++istation) {
          const int zs = (izone*NUM_STATIONS) + istation;
          int ph_diff = zs_phi_differences.at(zs).at(iroad).first;
          std::cout << "find seg: z: " << road.zone << " r: " << road.winner
              << " st: " << istation << " ph_diff: " << ph_diff
              << std::endl;
        }
      }
    }
  }


  // Build all tracks in each zone
  for (int izone = 0; izone < NUM_ZONES; ++izone) {
    const EMTFRoadExtraCollection& roads = zone_roads.at(izone);

    for (unsigned iroad = 0; iroad < roads.size(); ++iroad) {
      const EMTFRoadExtra& road = roads.at(iroad);

      // Create a track
      EMTFTrackExtra track;
      track.endcap   = road.endcap;
      track.sector   = road.sector;
      track.bx       = road.bx;
      track.zone     = road.zone;

      track.xhits.clear();

      // Insert hits
      for (int istation = 0; istation < NUM_STATIONS; ++istation) {
        const int zs = (izone*NUM_STATIONS) + istation;

        const EMTFHitExtraCollection& conv_hits = zs_conv_hits.at(zs);
        int       ph_diff      = zs_phi_differences.at(zs).at(iroad).first;
        hit_ptr_t conv_hit_ptr = zs_phi_differences.at(zs).at(iroad).second;

        if (ph_diff != invalid_ph_diff) {
          insert_hits(conv_hit_ptr, conv_hits, track);
        }
      }

      if (fixZonePhi_) {
        assert(track.xhits.size() > 0);
      }

      //track.road = static_cast<EMTFRoad>(road);
      track.xroad = road;

      // Output track
      zone_tracks.at(izone).push_back(track);

    }  // end loop over roads
  }  // end loop over zones

  if (verbose_ > 0) {  // debug
    for (const auto& tracks : zone_tracks) {
      for (const auto& track : tracks) {
        for (const auto& xhit : track.xhits) {
          int fs_segment = get_fs_segment(xhit);
          std::cout << "match seg: z: " << track.xroad.zone << " pat: " << track.xroad.winner <<  " st: " << xhit.station
              << " vi: " << to_binary(0b1, 2) << " hi: " << ((fs_segment>>4) & 0x3)
              << " ci: " << ((fs_segment>>1) & 0x7) << " si: " << (fs_segment & 0x1)
              << " ph: " << xhit.phi_fp << " th: " << xhit.theta_fp
              << std::endl;
        }
      }
    }
  }

}

void EMTFPrimitiveMatching::process_single_zone_station(
    int station,
    const EMTFRoadExtraCollection& roads,
    const EMTFHitExtraCollection& conv_hits,
    std::vector<hit_sort_pair_t>& phi_differences
) const {
  // max phi difference between pattern and segment
  int max_ph_diff = (station == 1) ? 15 : 7;
  //int bw_ph_diff = (station == 1) ? 5 : 4; // ph difference bit width
  //int invalid_ph_diff = (station == 1) ? 31 : 15;  // invalid difference

  if (fixZonePhi_) {
    if (station == 1) {
      max_ph_diff = 496;  // width of pattern in ME1 + rounding error 15*32+16
      //bw_ph_diff = 9;
      //invalid_ph_diff = 0x1ff;
    } else if (station == 2) {
      // max_ph_diff = 16;   // just rounding error for ME2 (pattern must match ME2 hit phi if there was one)
      // max_ph_diff = 32;   // allow neighbor phi bit
      max_ph_diff = 240;  // same as stations 3 & 4
      //bw_ph_diff = 5;
      //invalid_ph_diff = 0x1f;
    } else {
      max_ph_diff = 240;  // width of pattern in ME3,4 + rounding error 7*32+16
      //bw_ph_diff = 8;
      //invalid_ph_diff = 0xff;
    }
  }

  auto abs_diff = [](int a, int b) { return std::abs(a-b); };

  EMTFRoadExtraCollection::const_iterator roads_it  = roads.begin();
  EMTFRoadExtraCollection::const_iterator roads_end = roads.end();

  for (; roads_it != roads_end; ++roads_it) {
    int ph_pat = roads_it->ph_num;  // ph detected in pattern
    int ph_q   = roads_it->ph_q;
    assert(ph_pat >= 0 && ph_q > 0);

    if (fixZonePhi_) {
      ph_pat <<= 5;  // add missing 5 lower bits to pattern phi
    }

    std::vector<hit_sort_pair_t> tmp_phi_differences;

    EMTFHitExtraCollection::const_iterator conv_hits_it  = conv_hits.begin();
    EMTFHitExtraCollection::const_iterator conv_hits_end = conv_hits.end();

    for (; conv_hits_it != conv_hits_end; ++conv_hits_it) {
      int ph_seg     = conv_hits_it->phi_fp;  // ph from segments
      int ph_seg_red = ph_seg >> (bw_fph-bpow-1);  // remove unused low bits
      assert(ph_seg >= 0);

      if (fixZonePhi_) {
        ph_seg_red = ph_seg;  // use full-precision phi
      }

      // Get abs difference
      int ph_diff = abs_diff(ph_pat, ph_seg_red);
      if (ph_diff > max_ph_diff)
        ph_diff = invalid_ph_diff;  // difference is too high, cannot be the same pattern

      tmp_phi_differences.push_back(std::make_pair(ph_diff, conv_hits_it));  // make a key-value pair
    }

    if (!tmp_phi_differences.empty()) {
      // Find best phi difference
      sort_ph_diff(tmp_phi_differences);

      // Store the best phi difference
      phi_differences.push_back(tmp_phi_differences.front());

    } else {
      // No segment found
      phi_differences.push_back(std::make_pair(invalid_ph_diff, conv_hits_end));  // make a key-value pair
    }

  }  // end loop over roads
}

void EMTFPrimitiveMatching::sort_ph_diff(
    std::vector<hit_sort_pair_t>& phi_differences
) const {
  // Sort by key, but preserving the original order
  struct {
    typedef hit_sort_pair_t value_type;
    bool operator()(const value_type& lhs, const value_type& rhs) const {
      //return lhs.first < rhs.first;

      // The firmware inputs later BX (history id = 0) before earlier BX (history id = 1,2)
      // This seems to make the firmware sorter preferably pick later BX, but not necessarily
      return std::make_pair(lhs.first, -lhs.second->bx) < std::make_pair(rhs.first, -rhs.second->bx);
    }
  } less_ph_diff_cmp;

  std::stable_sort(phi_differences.begin(), phi_differences.end(), less_ph_diff_cmp);
}

void EMTFPrimitiveMatching::insert_hits(
    hit_ptr_t conv_hit_ptr, const EMTFHitExtraCollection& conv_hits,
    EMTFTrackExtra& track
) const {
  EMTFHitExtraCollection::const_iterator conv_hits_it  = conv_hits.begin();
  EMTFHitExtraCollection::const_iterator conv_hits_end = conv_hits.end();

  // Find all possible duplicated hits, insert them
  for (; conv_hits_it != conv_hits_end; ++conv_hits_it) {
    const EMTFHitExtra& conv_hit_i = *conv_hits_it;
    const EMTFHitExtra& conv_hit_j = *conv_hit_ptr;

    // All these must match: [bx_history][station][chamber][segment]
    if (
      (conv_hit_i.pc_station == conv_hit_j.pc_station) &&
      (conv_hit_i.pc_chamber == conv_hit_j.pc_chamber) &&
      (conv_hit_i.ring       == conv_hit_j.ring) &&  // because of ME1/1
      (conv_hit_i.strip      == conv_hit_j.strip) &&
      //(conv_hit_i.wire       == conv_hit_j.wire) &&
      (conv_hit_i.pattern    == conv_hit_j.pattern) &&
      (conv_hit_i.bx         == conv_hit_j.bx)
    ) {
      // Must have the same phi_fp
      assert(conv_hit_i.phi_fp == conv_hit_j.phi_fp);

      insert_hit(conv_hits_it, track);
    }
  }
}

void EMTFPrimitiveMatching::insert_hit(
    hit_ptr_t conv_hit_ptr,
    EMTFTrackExtra& track
) const {
  struct {
    typedef EMTFHitExtra value_type;
    constexpr bool operator()(const value_type& lhs, const value_type& rhs) {
      //return lhs.station < rhs.station;

      // The firmware inputs later BX (history id = 0) before earlier BX (history id = 1,2)
      // This seems to make the firmware sorter preferably pick later BX, but not necessarily
      return std::make_pair(lhs.station, -lhs.bx) < std::make_pair(rhs.station, -rhs.bx);
    }
  } less_station_cmp;

  // Sorted insert by station
  EMTFHitExtraCollection::const_iterator upper = std::upper_bound(track.xhits.begin(), track.xhits.end(), *conv_hit_ptr, less_station_cmp);
  track.xhits.insert(upper, *conv_hit_ptr);
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
