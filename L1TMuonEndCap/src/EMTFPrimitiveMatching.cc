#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPrimitiveMatching.hh"


void EMTFPrimitiveMatching::configure(
    int endcap, int sector, int bx
) {
  endcap_ = endcap;
  sector_ = sector;
  bx_     = bx;
}

void EMTFPrimitiveMatching::match(
    const std::deque<EMTFHitExtraCollection>& extended_conv_hits,
    const std::vector<EMTFRoadExtraCollection>& zone_roads,
    std::vector<EMTFTrackExtraCollection>& zone_tracks
) const {

  // Organize converted hits by (zone, station)
  std::array<EMTFHitExtraCollection, NUM_ZONES*NUM_STATIONS> zs_conv_hits;

  bool use_zone_code_pm = true;

  // Loop over converted hits and fill the map
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
      if (use_zone_code_pm)
        zone_code = get_zone_code_pm(conv_hit);  // decide based on new zone code

      for (int izone = 0; izone < NUM_ZONES; ++izone) {
        if (zone_code & (1<<izone)) {
          int zs = (izone*4) + istation;
          zs_conv_hits.at(zs).push_back(conv_hit);
        }
      }

    }  // end loop over conv_hits
  }  // end loop over extended_conv_hits


  zone_tracks.clear();
  zone_tracks.resize(NUM_ZONES);

  // Get the best-matching hits by comparing phi difference between
  // pattern and segment
  for (int izone = 0; izone < NUM_ZONES; ++izone) {
    const EMTFRoadExtraCollection& roads = zone_roads.at(izone);
    int nroads = roads.size();

    for (int istation = 0; istation < NUM_STATIONS; ++istation) {
      const int zs = (izone*4) + istation;
      const int bpow = 7;  // (1 << bpow) is count of input ranks
      const int max_ph_diff = (istation == 0) ? 15 : 7;  // max phi difference
      const int invalid_ph_diff = (istation == 0) ? 31 : 15;  // invalid difference

      const EMTFHitExtraCollection& conv_hits = zs_conv_hits.at(zs);
      int nchits = conv_hits.size();

      for (int iroad = 0; iroad < nroads; ++iroad) {
        int ph_pat = roads.at(iroad).ph_num;  // ph detected in pattern
        int ph_q   = roads.at(iroad).ph_q;
        assert(ph_pat >= 0 && ph_q > 0);

        std::vector<std::pair<int, int> > phi_differences;

        for (int ichit = 0; ichit < nchits; ++ichit) {
          int ph_seg     = conv_hits.at(ichit).phi_fp;  // ph from segments
          int ph_seg_red = ph_seg >> bpow;  // remove unused low bits
          assert(ph_seg >= 0);

          // get abs difference
          int ph_diff = (ph_pat > ph_seg_red) ? (ph_pat - ph_seg_red) : (ph_seg_red - ph_pat);

          if (ph_diff > max_ph_diff) {
            ph_diff = invalid_ph_diff;  // difference is too high, cannot be the same pattern
          }

          phi_differences.push_back(std::make_pair(ichit, ph_diff));  // make a key-value pair

        }  // end loop over conv_hits

        if (!phi_differences.empty()) {
          // Find best phi difference
          sort_ph_diff(phi_differences);

          EMTFTrackExtra track = make_track(conv_hits, phi_differences.front());
          zone_tracks.at(izone).push_back(track);
        }

      }  // end loop over roads

    }  // end loop over stations
  }  // end loop over zones

}

unsigned int EMTFPrimitiveMatching::get_zone_code_pm(const EMTFHitExtra& conv_hit) const {
  static const int zone_code_pm_table[4][3] = {  // [station][ring]
    {0b0011, 0b0100, 0b1000},  // st1 r1: [z0,z1], r2: [z2], r3: [z3]
    {0b0011, 0b1100, 0b0000},  // st2 r1: [z0,z1], r2: [z2,z3]
    {0b0001, 0b1110, 0b0000},  // st3 r1: [z0], r2: [z1,z2,z3]
    {0b0001, 0b0110, 0b0000}   // st4 r1: [z0], r2: [z1,z2]
  };

  unsigned istation = conv_hit.station-1;
  unsigned iring    = (conv_hit.station == 1 && conv_hit.ring == 4) ? conv_hit.ring-4 : conv_hit.ring-1;
  assert(istation < 4 && iring < 3);
  return zone_code_pm_table[istation][iring];
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

EMTFTrackExtra EMTFPrimitiveMatching::make_track(
    const EMTFHitExtraCollection& conv_hits,
    std::pair<int, int> best_ph_diff
) const {

  EMTFTrackExtra track;

  return track;
}
