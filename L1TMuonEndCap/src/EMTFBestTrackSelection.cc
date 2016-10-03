#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFBestTrackSelection.hh"

#include "helper.h"  // to_hex, to_binary


void EMTFBestTrackSelection::configure(
    int verbose, int endcap, int sector, int bx,
    int maxRoadsPerZone, int maxTracks, bool useSecondEarliest
) {
  verbose_ = verbose;
  endcap_  = endcap;
  sector_  = sector;
  bx_      = bx;

  maxRoadsPerZone_    = maxRoadsPerZone;
  maxTracks_          = maxTracks;
  useSecondEarliest_  = useSecondEarliest;
}

void EMTFBestTrackSelection::process(
    const std::deque<EMTFTrackExtraCollection>& extended_best_track_cands,
    EMTFTrackExtraCollection& best_tracks
) const {

  if (!useSecondEarliest_) {
    cancel_one_bx(extended_best_track_cands, best_tracks);
  } else {
    cancel_three_bx(extended_best_track_cands, best_tracks);
  }

  if (verbose_ > 0) {  // debug
    for (const auto& track : best_tracks) {
      std::cout << "track: " << track.winner << " rank: " << to_hex(track.rank)
          << " ph_deltas: " << array_as_string(track.ptlut_data.delta_ph)
          << " th_deltas: " << array_as_string(track.ptlut_data.delta_th)
          << " phi: " << track.phi_int << " theta: " << track.theta_int
          << " cpat: " << array_as_string(track.ptlut_data.cpattern)
          << std::endl;
      for (const auto& conv_hit : track.xhits) {
        std::cout << ".. track segments: st: " << conv_hit.pc_station << " ch: " << conv_hit.pc_chamber << " ph: " << conv_hit.phi_fp << " th: " << conv_hit.theta_fp << " cscid: " << (conv_hit.cscn_ID-1) << std::endl;
      }
    }
  }
}

void EMTFBestTrackSelection::cancel_one_bx(
    const std::deque<EMTFTrackExtraCollection>& extended_best_track_cands,
    EMTFTrackExtraCollection& best_tracks
) const {
  const int num_h = extended_best_track_cands.size() / NUM_ZONES;  // num of bx history
  assert((int) extended_best_track_cands.size() == num_h * NUM_ZONES);

  const int max_z = NUM_ZONES;        // = 4 zones
  const int max_n = maxRoadsPerZone_; // = 3 candidates per zone
  const int max_zn = max_z * max_n;   // = 12 total candidates
  assert(maxTracks_ <= max_zn);

  // Emulate the arrays used in firmware
  typedef std::array<int, 2> segment_ref_t;
  std::vector<std::vector<segment_ref_t> > segments(max_zn, std::vector<segment_ref_t>());  // 2D array [zn][num segments]

  std::vector<std::vector<bool> > larger(max_zn, std::vector<bool>(max_zn, false));  // 2D array [zn][zn]
  std::vector<std::vector<bool> > winner(max_zn, std::vector<bool>(max_zn, false));

  std::vector<bool> exists(max_zn, false);  // 1D array [zn]
  std::vector<bool> killed(max_zn, false);
  std::vector<int>  rank  (max_zn, 0);

  std::deque<EMTFTrackExtraCollection>::const_iterator zone_tracks_begin = extended_best_track_cands.end();
  zone_tracks_begin -= NUM_ZONES;

  // Initialize arrays
  for (int z = 0; z < max_z; ++z) {
    const EMTFTrackExtraCollection& tracks = *(zone_tracks_begin + z);
    const int ntracks = tracks.size();
    assert(ntracks <= max_n);

    for (int n = 0; n < ntracks; ++n) {
      const int zn = (n * max_z) + z;  // for (i = 0; i < 12; i = i+1) rank[i%4][i/4]
      const EMTFTrackExtra& track = tracks.at(n);

      rank[zn] = track.rank;

      for (const auto& conv_hit : track.xhits) {
        assert(conv_hit.valid);

        // A segment identifier (chamber, strip)
        const segment_ref_t segment = {{conv_hit.pc_station*9 + conv_hit.pc_chamber, conv_hit.strip}};  // due to GCC bug, use {{}} instead of {}
        segments.at(zn).push_back(segment);
      }
    }  // end loop over n
  }  // end loop over z

  bool no_tracks = true;
  for (int i = 0; i < max_zn; ++i) {
    if (rank[i] > 0)
      no_tracks = false;
  }
  if (no_tracks)  // early exit
    return;


  // Copied from firmware
  bool use_fw_algo = true;

  if (use_fw_algo) {
    int ri=0, rj=0, gt=0, eq=0;

    // Simultaneously compare each rank with each other
    for (int i = 0; i < max_zn; ++i) {
      for (int j = 0; j < max_zn; ++j) {
        larger[i][j] = 0;
      }
      larger[i][i] = 1; // result of comparison with itself
      //ri = rank[i%4][i/4]; // first index loops zone, second loops candidate. Zone loops faster, so we give equal priority to zones
      ri = rank[i];

      for (int j = 0; j < max_zn; ++j) {
        // i&j bits show which rank is larger
        // the comparison scheme below avoids problems
        // when there are two or more tracks with the same rank
        //rj = rank[j%4][j/4];
        rj = rank[j];
        gt = ri > rj;
        eq = ri == rj;
        if ((i < j && (gt || eq)) || (i > j && gt))
          larger[i][j] = 1;
      }

      // "larger" array shows the result of comparison for each rank

      // track exists if quality != 0
      exists[i] = (ri != 0);
    }

    // ghost cancellation, only in the current BX so far
    for (int i = 0; i < max_zn-1; ++i) { // candidate loop
      for (int j = i+1; j < max_zn; ++j) { // comparison candidate loop
        int shared_segs = 0;

        // count shared segments
        for (const auto& isegment : segments.at(i)) {  // loop over all pairs of hits
          for (const auto& jsegment : segments.at(j)) {
            if (isegment == jsegment) {  // same chamber and same segment
              shared_segs += 1;
            }
          }
        }

        if (shared_segs > 0) {  // a single shared segment means it's ghost
          // kill candidate that has lower rank
          if (larger[i][j])
            killed[j] = 1;
          else
            killed[i] = 1;
        }
      }
    }

    // remove ghosts according to kill mask
    //exists = exists & (~kill1);
    for (int i = 0; i < max_zn; ++i) {
      exists[i] = exists[i] & (!killed[i]);
    }

    for (int i = 0; i < max_zn; ++i) {
      for (int j = 0; j < max_zn; ++j) {
        //if  (exists[i]) larger[i] = larger[i] | (~exists); // if this track exists make it larger than all non-existing tracks
        //else  larger[i] = 0; // else make it smaller than anything
        if (exists[i])
          larger[i][j] = larger[i][j] | (!exists[j]);
        else
          larger[i][j] = 0;
      }

      // count zeros in the comparison results. The best track will have none, the next will have one, the third will have two.
      // skip the bits corresponding to the comparison of the track with itself
      int sum = 0;
      for (int j = 0; j < max_zn; ++j) {
        if (larger[i][j] == 0)
          sum += 1;
      }

      if (sum < maxTracks_)  winner[sum][i] = 1; // assign positional winner codes
    }

  } else {
    // Not implemented
  }

  // Output best tracks according to winner signals
  best_tracks.clear();

  for (int o = 0; o < maxTracks_; ++o) { // output candidate loop
    for (int i = 0; i < max_zn; ++i) { // winner bit loop
      if (winner[o][i]) {
        const EMTFTrackExtraCollection& tracks = *(zone_tracks_begin + (i%max_z));
        const EMTFTrackExtra& track = tracks.at(i/max_z);
        best_tracks.push_back(track);
        best_tracks.back().winner = o;
      }
    }
  }
}

void EMTFBestTrackSelection::cancel_three_bx(
    const std::deque<EMTFTrackExtraCollection>& extended_best_track_cands,
    EMTFTrackExtraCollection& best_tracks
) const {
  const int num_h = extended_best_track_cands.size() / NUM_ZONES;  // num of bx history
  assert((int) extended_best_track_cands.size() == num_h * NUM_ZONES);

  const int max_h = 3;                       // = 3 bx history
  const int max_z = NUM_ZONES;               // = 4 zones
  const int max_n = maxRoadsPerZone_;        // = 3 candidates per zone
  //const int max_zn = max_z * max_n;          // = 12 total candidates
  const int max_hzn = max_h * max_z * max_n; // = 36 total candidates
  assert(maxTracks_ <= max_hzn);

  // Emulate the arrays used in firmware
  typedef std::array<int, 3> segment_ref_t;
  std::vector<std::vector<segment_ref_t> > segments(max_hzn, std::vector<segment_ref_t>());  // 2D array [hzn][num segments]

  std::vector<std::vector<bool> > larger(max_hzn, std::vector<bool>(max_hzn, false));  // 2D array [hzn][hzn]
  std::vector<std::vector<bool> > winner(max_hzn, std::vector<bool>(max_hzn, false));

  std::vector<bool> exists (max_hzn, false);  // 1D array [hzn]
  std::vector<bool> killed (max_hzn, false);
  std::vector<int>  rank   (max_hzn, 0);
  std::vector<int>  good_bx(max_hzn, 0);

  std::deque<EMTFTrackExtraCollection>::const_iterator zone_tracks_begin;

  // Initialize arrays: rank, segments
  for (int h = 0; h < num_h; ++h) {
    if (h == 0)
      zone_tracks_begin = extended_best_track_cands.end();
    zone_tracks_begin -= NUM_ZONES;

    for (int z = 0; z < max_z; ++z) {
      const EMTFTrackExtraCollection& tracks = *(zone_tracks_begin + z);
      const int ntracks = tracks.size();
      assert(ntracks <= max_n);

      for (int n = 0; n < ntracks; ++n) {
        const int hzn = (h * max_z * max_n) + (n * max_z) + z;  // for (i = 0; i < 12; i = i+1) rank[i%4][i/4]
        const EMTFTrackExtra& track = tracks.at(n);
        int cand_bx = track.second_bx;
        cand_bx = cand_bx - bx_ + 2;  // convert into 0..2 --> -2..0 relative to bx_

        rank.at(hzn) = track.rank;
        if (cand_bx == h)
          good_bx.at(hzn) = 1;  // kill this rank if it's not the right BX

        for (const auto& conv_hit : track.xhits) {
          assert(conv_hit.valid);

          // A segment identifier (chamber, strip, bx)
          const segment_ref_t segment = {{conv_hit.pc_station*9 + conv_hit.pc_chamber, conv_hit.strip, conv_hit.bx}};  // due to GCC bug, use {{}} instead of {}
          segments.at(hzn).push_back(segment);
        }
      }  // end loop over n
    }  // end loop over z
  }  // end loop over e

  bool no_tracks = true;
  for (int i = 0; i < max_hzn; ++i) {
    if (rank[i] > 0)
      no_tracks = false;
  }
  if (no_tracks)  // early exit
    return;


  // Copied from firmware
  bool use_fw_algo = true;

  if (use_fw_algo) {
    int ri=0, rj=0;

    // Simultaneously compare each rank with each other
    for (int i = 0; i < max_hzn; ++i) {
      //for (int j = 0; j < max_hzn; ++j) {
      //  larger[i][j] = 0;
      //}
      larger[i][i] = 1; // result of comparison with itself
      //ri = rank[i%4][i/4]; // first index loops zone, second loops candidate. Zone loops faster, so we give equal priority to zones
      ri = rank[i];

      for (int j = i+1; j < max_hzn; ++j) {
        // i&j bits show which rank is larger
        //rj = rank[j%4][j/4];
        rj = rank[j];
        if (ri >= rj)
          larger[i][j] = 1;
        else
          larger[j][i] = 1;
      }

      // "larger" array shows the result of comparison for each rank

      // track exists if quality != 0
      exists[i] = (ri != 0);
    }

    // ghost cancellation, over 3 BXs
    for (int i = 0; i < max_hzn-1; ++i) { // candidate loop
      for (int j = i+1; j < max_hzn; ++j) { // comparison candidate loop
        int shared_segs = 0;

        // count shared segments
        for (const auto& isegment : segments.at(i)) {  // loop over all pairs of hits
          for (const auto& jsegment : segments.at(j)) {
            if (isegment == jsegment) {  // same chamber and same segment
              shared_segs += 1;
            }
          }
        }

        if (shared_segs > 0) {  // a single shared segment means it's ghost
          // kill candidate that has lower rank
          if (larger[i][j])
            killed[j] = 1;
          else
            killed[i] = 1;
        }
      }
    }

    // remove ghosts according to kill mask
    //exists = exists & (~kill1);
    for (int i = 0; i < max_hzn; ++i) {
      exists[i] = exists[i] & !killed[i];
    }

    // remove tracks that are not at correct BX number
    //exists = exists & good_bx;
    for (int i = 0; i < max_hzn; ++i) {
      exists[i] = exists[i] & good_bx[i];
    }

    for (int i = 0; i < max_hzn; ++i) {
      for (int j = 0; j < max_hzn; ++j) {
        //if  (exists[i]) larger[i] = larger[i] | (~exists); // if this track exists make it larger than all non-existing tracks
        //else  larger[i] = 0; // else make it smaller than anything
        if (exists[i])
          larger[i][j] = larger[i][j] | (!exists[j]);
        else
          larger[i][j] = 0;
      }

      // count zeros in the comparison results. The best track will have none, the next will have one, the third will have two.
      int sum = 0;
      for (int j = 0; j < max_hzn; ++j) {
        if (larger[i][j] == 0)
          sum += 1;
      }

      if (sum < maxTracks_)  winner[sum][i] = 1; // assign positional winner codes
    }

  } else {
    // Not implemented
  }

  // Output best tracks according to winner signals
  best_tracks.clear();

  for (int o = 0; o < maxTracks_; ++o) { // output candidate loop
    for (int i = 0; i < max_hzn; ++i) { // winner bit loop
      if (winner[o][i]) {
        zone_tracks_begin = extended_best_track_cands.end();
        zone_tracks_begin -= (i/(max_z * max_n)) * NUM_ZONES;

        const EMTFTrackExtraCollection& tracks = *(zone_tracks_begin + (i%max_z));
        const EMTFTrackExtra& track = tracks.at(i/max_z);
        best_tracks.push_back(track);
        best_tracks.back().winner = o;
      }
    }
  }
}
