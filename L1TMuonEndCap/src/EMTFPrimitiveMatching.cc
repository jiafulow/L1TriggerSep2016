#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPrimitiveMatching.hh"

#include "helper.hh"  // to_hex, to_binary

namespace {
  static const int bw_fph = 13;  // bit width of ph, full precision
  static const int bpow = 7;     // (1 << bpow) is count of input ranks
  static const int invalid_ph_diff = 0x1ff;  // 511 (9-bit)
}


void EMTFPrimitiveMatching::configure(
    int verbose, int endcap, int sector, int bx,
    bool fixZonePhi,
    bool bugME11Dupes
) {
  verbose_ = verbose;
  endcap_  = endcap;
  sector_  = sector;
  bx_      = bx;

  fixZonePhi_      = fixZonePhi;
  bugME11Dupes_    = bugME11Dupes;
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
        std::cout << "pattern on match input: z: " << road.zone-1 << " r: " << road.winner
            << " ph_num: " << road.key_zhit << " ph_q: " << to_hex(road.quality_code)
            << " ly: " << to_binary(road.layer_code, 3) << " str: " << to_binary(road.straightness, 3)
            << std::endl;
      }
    }
  }

  // Organize converted hits by (zone, station)
  std::array<EMTFHitExtraCollection, NUM_ZONES*NUM_STATIONS> zs_conv_hits;

  bool use_fs_zone_code = true;  // use zone code as in firmware find_segment module

  std::deque<EMTFHitExtraCollection>::const_iterator ext_conv_hits_it  = extended_conv_hits.begin();
  std::deque<EMTFHitExtraCollection>::const_iterator ext_conv_hits_end = extended_conv_hits.end();

  for (; ext_conv_hits_it != ext_conv_hits_end; ++ext_conv_hits_it) {
    EMTFHitExtraCollection::const_iterator conv_hits_it  = ext_conv_hits_it->begin();
    EMTFHitExtraCollection::const_iterator conv_hits_end = ext_conv_hits_it->end();

    for (; conv_hits_it != conv_hits_end; ++conv_hits_it) {

      // Can we do this at some later stage? Check RPC vs. track timing ... - AWB 18.11.16
      //if (conv_hits_it->subsystem == TriggerPrimitive::kRPC && bx_ != conv_hits_it->bx)
      //  continue;  // Only use RPC clusters in the same BX as the track

      int istation = conv_hits_it->station-1;
      int zone_code = conv_hits_it->zone_code;  // decide based on original zone code
      if (use_fs_zone_code)
        zone_code = conv_hits_it->fs_zone_code;  // decide based on new zone code

      // A hit can go into multiple zones
      for (int izone = 0; izone < NUM_ZONES; ++izone) {
        if (zone_roads.at(izone).size() > 0) {

          if (zone_code & (1<<izone)) {
            const int zs = (izone*NUM_STATIONS) + istation;
            zs_conv_hits.at(zs).push_back(*conv_hits_it);

            // Update fs_history encoded in fs_segment
            // This update only goes into the hits associated to a track, it does not affect the original hit collection
            int fs_history = bx_ - (conv_hits_it->bx);  // 0 for current BX, 1 for previous BX, 2 for BX before that
            zs_conv_hits.at(zs).back().fs_segment |= ((fs_history & 0x3)<<4);

            // Update bt_history encoded in bt_segment
            // This update only goes into the hits associated to a track, it does not affect the original hit collection
            int bt_history = fs_history;
            zs_conv_hits.at(zs).back().bt_segment |= ((bt_history & 0x3)<<5);
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

      // This leaves zone_roads.at(izone) and zs_conv_hits.at(zs) unchanged
      // zs_phi_differences.at(zs) gets filled with a pair of <phi_diff, conv_hit> for the
      // conv_hit with the lowest phi_diff from the pattern in this station and zone
      process_single_zone_station(
          izone+1, istation+1,
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
          std::cout << "find seg: z: " << road.zone-1 << " r: " << road.winner
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
      track.ph_num   = road.key_zhit;
      track.ph_q     = road.quality_code;
      track.rank     = road.quality_code;
      track.winner   = road.winner;

      track.xhits.clear();

      // Insert hits
      for (int istation = 0; istation < NUM_STATIONS; ++istation) {
        const int zs = (izone*NUM_STATIONS) + istation;

        const EMTFHitExtraCollection& conv_hits = zs_conv_hits.at(zs);
        int       ph_diff      = zs_phi_differences.at(zs).at(iroad).first;
        hit_ptr_t conv_hit_ptr = zs_phi_differences.at(zs).at(iroad).second;

        if (ph_diff != invalid_ph_diff) {
          // Inserts the conv_hit with the lowest phi_diff, as well as its duplicate
          // (same strip and phi, different wire and theta), if a duplicate exists
          insert_hits(conv_hit_ptr, conv_hits, track);
        }
      }

      if (fixZonePhi_) {
        assert(track.xhits.size() > 0);
      }

      // Output track
      zone_tracks.at(izone).push_back(track);

    }  // end loop over roads
  }  // end loop over zones

  if (verbose_ > 0) {  // debug
    for (const auto& tracks : zone_tracks) {
      for (const auto& track : tracks) {
        for (const auto& xhit : track.xhits) {
          std::cout << "match seg: z: " << track.zone-1 << " pat: " << track.winner <<  " st: " << xhit.station
              << " vi: " << to_binary(0b1, 2) << " hi: " << ((xhit.fs_segment>>4) & 0x3)
              << " ci: " << ((xhit.fs_segment>>1) & 0x7) << " si: " << (xhit.fs_segment & 0x1)
              << " ph: " << xhit.phi_fp << " th: " << xhit.theta_fp
              << std::endl;
        }
      }
    }
  }

}

void EMTFPrimitiveMatching::process_single_zone_station(
    int zone, int station,
    const EMTFRoadExtraCollection& roads,
    const EMTFHitExtraCollection& conv_hits,
    std::vector<hit_sort_pair_t>& phi_differences
) const {
  // max phi difference between pattern and segment
  // This doesn't depend on the pattern straightness - any hit within the max phi difference may match
  int max_ph_diff = (station == 1) ? 15 : 7;
  //int bw_ph_diff = (station == 1) ? 5 : 4; // ph difference bit width
  //int invalid_ph_diff = (station == 1) ? 31 : 15;  // invalid difference

  if (fixZonePhi_) {
    if (station == 1) {
      max_ph_diff = 496;  // width of pattern in ME1 + rounding error 15*32+16
      //bw_ph_diff = 9;
      //invalid_ph_diff = 0x1ff;
    } else if (station == 2) {
      //max_ph_diff = 16;   // just rounding error for ME2 (pattern must match ME2 hit phi if there was one)
      //max_ph_diff = 32;   // allow neighbor phi bit
      max_ph_diff = 240;  // same as ME3,4
      //bw_ph_diff = 5;
      //invalid_ph_diff = 0x1f;
    } else {
      max_ph_diff = 240;  // width of pattern in ME3,4 + rounding error 7*32+16
      //bw_ph_diff = 8;
      //invalid_ph_diff = 0xff;
    }
  }

  auto abs_diff = [](int a, int b) { return std::abs(a-b); };

  // Simple sort by ph_diff
  struct {
    typedef hit_sort_pair_t value_type;
    bool operator()(const value_type& lhs, const value_type& rhs) const {
      // If different types, prefer CSC over RPC; else prefer the closer hit in dPhi
      if (lhs.second->subsystem != rhs.second->subsystem)
        return (lhs.second->subsystem == TriggerPrimitive::kCSC);
      else
        return lhs.first <= rhs.first;
    }
  } less_ph_diff_cmp;

  // Emulation of FW sorting with 3-way comparator
  struct {
    typedef hit_sort_pair_t value_type;
    int operator()(const value_type& a, const value_type& b, const value_type& c) const {
      int r = 0;
      r |= bool(a.first <= b.first);
      r <<= 1;
      r |= bool(b.first <= c.first);
      r <<= 1;
      r |= bool(c.first <= a.first);

      int rr = 0;
      switch(r) {
      //case 0b000 : rr = 3; break;  // invalid
      case 0b001 : rr = 2; break;  // c
      case 0b010 : rr = 1; break;  // b
      case 0b011 : rr = 1; break;  // b
      case 0b100 : rr = 0; break;  // a
      case 0b101 : rr = 2; break;  // c
      case 0b110 : rr = 0; break;  // a
      //case 0b111 : rr = 0; break;  // invalid
      default    : rr = 0; break;
      }
      return rr;
    }
  } less_ph_diff_cmp3;


  // ___________________________________________________________________________
  // For each road, find the segment with min phi difference in every station

  EMTFRoadExtraCollection::const_iterator roads_it  = roads.begin();
  EMTFRoadExtraCollection::const_iterator roads_end = roads.end();

  for (; roads_it != roads_end; ++roads_it) {
    int ph_pat = roads_it->key_zhit;     // pattern key phi value
    int ph_q   = roads_it->quality_code; // pattern quality code
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

      // Get abs phi difference
      int ph_diff = abs_diff(ph_pat, ph_seg_red);
      if (ph_diff > max_ph_diff)
        ph_diff = invalid_ph_diff;  // difference is too high, cannot be the same pattern

      if (ph_diff != invalid_ph_diff)
        tmp_phi_differences.push_back(std::make_pair(ph_diff, conv_hits_it));  // make a key-value pair
    }

    // _________________________________________________________________________
    // Sort to find the segment with min phi difference

    if (!tmp_phi_differences.empty()) {
      // Find best phi difference
      std::stable_sort(tmp_phi_differences.begin(), tmp_phi_differences.end(), less_ph_diff_cmp);

      // Store the best phi difference
      phi_differences.push_back(tmp_phi_differences.front());

      // Because the sorting is sensitive to FW ordering, use the exact FW sorting.
      // For now, do it only when the min phi difference comes from a CSC hit,
      // because the FW ordering for CSC hits is known. To be updated later to
      // include also the RPC hits.
      // This implementation still differs from FW because I prefer to use a
      // sorting function that is as generic as possible.
      bool use_fw_sorting = true;

      if (use_fw_sorting && (tmp_phi_differences.front().second->subsystem == TriggerPrimitive::kCSC)) {  // only when the min phi diff is from CSC
        // zone_cham = 4 for [fs_01, fs_02, fs_03, fs_11], or 7 otherwise
        // tot_diff = 27 or 45 in FW; it is 27 or 54 in the C++ merge_sort3 impl
        const int max_drift = 3; // should use bxWindow from the config
        const int zone_cham = ((zone == 1 && (2 <= station && station <= 4)) || (zone == 2 && station == 2)) ? 4 : 7;
        const int seg_ch    = 2;
        const int tot_diff  = (max_drift*zone_cham*seg_ch) + ((zone_cham == 4) ? 3 : 12);  // provide padding for 3-input comparators

        std::vector<hit_sort_pair_t> fw_sort_array(tot_diff, std::make_pair(invalid_ph_diff, conv_hits_end));

        std::vector<hit_sort_pair_t>::const_iterator phdiffs_it  = tmp_phi_differences.begin();
        std::vector<hit_sort_pair_t>::const_iterator phdiffs_end = tmp_phi_differences.end();

        for (; phdiffs_it != phdiffs_end; ++phdiffs_it) {
          if (phdiffs_it->second->subsystem != TriggerPrimitive::kCSC)  continue;  // only know the FW ordering for CSC

          //int ph_diff    = phdiffs_it->first;
          int fs_segment = phdiffs_it->second->fs_segment;

          // Calculate the index to put into the fw_sort_array
          int fs_history = ((fs_segment>>4) & 0x3);
          int fs_chamber = ((fs_segment>>1) & 0x7);
          fs_segment = (fs_segment & 0x1);
          unsigned fw_sort_array_index = (fs_history * zone_cham * seg_ch) + (fs_chamber * seg_ch) + fs_segment;

          assert(fs_history < max_drift && fs_chamber < zone_cham && fs_segment < seg_ch);
          assert(fw_sort_array_index < fw_sort_array.size());
          fw_sort_array.at(fw_sort_array_index) = *phdiffs_it;
        }

        // Debug
        //std::cout << "phdiffs" << std::endl;
        //for (unsigned i = 0; i < fw_sort_array.size(); ++i)
        //  std::cout << fw_sort_array.at(i).first << " ";
        //std::cout << std::endl;

        // Debug
        //std::cout << "Before sort" << std::endl;
        //for (unsigned i = 0; i < fw_sort_array.size(); ++i)
        //  std::cout << fw_sort_array.at(i).second->fs_segment << " ";
        //std::cout << std::endl;

        // Find the best phi difference according to FW sorting
        //merge_sort3(fw_sort_array.begin(), fw_sort_array.end(), less_ph_diff_cmp, less_ph_diff_cmp3);
        merge_sort3_with_hint(fw_sort_array.begin(), fw_sort_array.end(), less_ph_diff_cmp, less_ph_diff_cmp3, ((tot_diff == 54) ? tot_diff/2 : tot_diff/3));

        // Replace the best phi difference
        phi_differences.back() = fw_sort_array.front();

        // Debug
        //std::cout << "After sort" << std::endl;
        //for (unsigned i = 0; i < fw_sort_array.size(); ++i)
        //  std::cout << fw_sort_array.at(i).second->fs_segment << " ";
        //std::cout << std::endl;
      }

    } else {
      // No segment found
      phi_differences.push_back(std::make_pair(invalid_ph_diff, conv_hits_end));  // make a key-value pair
    }

  }  // end loop over roads
}

void EMTFPrimitiveMatching::insert_hits(
    hit_ptr_t conv_hit_ptr, const EMTFHitExtraCollection& conv_hits,
    EMTFTrackExtra& track
) const {
  EMTFHitExtraCollection::const_iterator conv_hits_it  = conv_hits.begin();
  EMTFHitExtraCollection::const_iterator conv_hits_end = conv_hits.end();

  const bool is_csc_me11 = (conv_hit_ptr->subsystem == TriggerPrimitive::kCSC) && (conv_hit_ptr->station == 1) && (conv_hit_ptr->ring == 1 || conv_hit_ptr->ring == 4);

  // Find all possible duplicated hits, insert them
  for (; conv_hits_it != conv_hits_end; ++conv_hits_it) {
    const EMTFHitExtra& conv_hit_i = *conv_hits_it;
    const EMTFHitExtra& conv_hit_j = *conv_hit_ptr;

    // All these must match: [bx_history][station][chamber][segment]
    if (
      (conv_hit_i.subsystem  == conv_hit_j.subsystem) &&
      (conv_hit_i.pc_station == conv_hit_j.pc_station) &&
      (conv_hit_i.pc_chamber == conv_hit_j.pc_chamber) &&
      (conv_hit_i.ring       == conv_hit_j.ring) &&  // because of ME1/1
      (conv_hit_i.strip      == conv_hit_j.strip) &&
      //(conv_hit_i.wire       == conv_hit_j.wire) &&
      (conv_hit_i.pattern    == conv_hit_j.pattern) &&
      (conv_hit_i.bx         == conv_hit_j.bx) &&
      (conv_hit_i.strip_low  == conv_hit_j.strip_low) && // For RPC clusters
      (conv_hit_i.strip_hi   == conv_hit_j.strip_hi) &&  // For RPC clusters
      //(conv_hit_i.roll       == conv_hit_j.roll) &&
      true
    ) {
      // All duplicates with the same strip but different wire must have same phi_fp
      assert(conv_hit_i.phi_fp == conv_hit_j.phi_fp);

      track.xhits.push_back(conv_hit_i);

    } else if (
      (bugME11Dupes_ && is_csc_me11) &&  // if reproduce ME1/1 theta duplication bug, do not check 'ring', 'strip' and 'pattern'
      (conv_hit_i.subsystem  == conv_hit_j.subsystem) &&
      (conv_hit_i.pc_station == conv_hit_j.pc_station) &&
      (conv_hit_i.pc_chamber == conv_hit_j.pc_chamber) &&
      //(conv_hit_i.ring       == conv_hit_j.ring) &&  // because of ME1/1
      //(conv_hit_i.strip      == conv_hit_j.strip) &&
      //(conv_hit_i.wire       == conv_hit_j.wire) &&
      //(conv_hit_i.pattern    == conv_hit_j.pattern) &&
      (conv_hit_i.bx         == conv_hit_j.bx) &&
      //(conv_hit_i.strip_low  == conv_hit_j.strip_low) && // For RPC clusters
      //(conv_hit_i.strip_hi   == conv_hit_j.strip_hi) &&  // For RPC clusters
      //(conv_hit_i.roll       == conv_hit_j.roll) &&
      true
    ) {
      // All duplicates with the same strip but different wire must have same phi_fp
      //assert(conv_hit_i.phi_fp == conv_hit_j.phi_fp);

      //track.xhits.push_back(conv_hit_i);

      // Dirty hack
      track.xhits.push_back(conv_hit_j);
      track.xhits.back().theta_fp = conv_hit_i.theta_fp;
    }
  }

  // Sort by station
  struct {
    typedef EMTFHitExtra value_type;
    constexpr bool operator()(const value_type& lhs, const value_type& rhs) {
      return lhs.station < rhs.station;
    }
  } less_station_cmp;

  std::stable_sort(track.xhits.begin(), track.xhits.end(), less_station_cmp);
}
