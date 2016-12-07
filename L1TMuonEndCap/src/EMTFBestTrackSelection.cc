#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFBestTrackSelection.hh"

#include "helper.hh"  // to_hex, to_binary

#define NUM_CSC_CHAMBERS 6*9   // 18 in ME1, 9 in ME2/3/4, 9 from neighbor sector                                                                   
#define NUM_RPC_CHAMBERS 6*7   // 6+1 neighbor in stations 1/2, 12+2 neighbor in 3/4                                                                

void EMTFBestTrackSelection::configure(
    int verbose, int endcap, int sector, int bx,
    int bxWindow,
    int maxRoadsPerZone, int maxTracks, bool useSecondEarliest
) {
  verbose_ = verbose;
  endcap_  = endcap;
  sector_  = sector;
  bx_      = bx;

  bxWindow_           = bxWindow;
  maxRoadsPerZone_    = maxRoadsPerZone;
  maxTracks_          = maxTracks;  // Set this to 1 to emulate cancel-out bug! or set pT to 0 for winner != 0.  - AWB 05.11.16
  useSecondEarliest_  = useSecondEarliest;
}

void EMTFBestTrackSelection::process(
    const std::deque<EMTFTrackExtraCollection>& extended_best_track_cands,
    EMTFTrackExtraCollection& best_tracks
) const {
  int num_cands = 0;
  for (const auto& cands : extended_best_track_cands) {
    for (const auto& cand : cands) {
      if (cand.rank > 0) {
        num_cands += 1;
      }
    }
  }
  bool early_exit = (num_cands == 0);

  if (early_exit)
    return;


  if (!useSecondEarliest_) {
    cancel_one_bx(extended_best_track_cands, best_tracks);
  } else {
    cancel_multi_bx(extended_best_track_cands, best_tracks);
  }

  if (verbose_ > 0) {  // debug
    for (const auto& track : best_tracks) {
      std::cout << "track: " << track.winner << " rank: " << to_hex(track.rank)
          << " ph_deltas: " << array_as_string(track.ptlut_data.delta_ph)
          << " th_deltas: " << array_as_string(track.ptlut_data.delta_th)
          << " phi: " << track.phi_int << " theta: " << track.theta_int
          << " cpat: " << array_as_string(track.ptlut_data.cpattern)
          << " bx: " << track.bx
          << std::endl;
      for (int i = 0; i < NUM_STATIONS+1; ++i) {  // stations 0-4
        if (track.ptlut_data.bt_vi[i] != 0)
          std::cout << ".. track segments: st: " << i
              << " v: " << track.ptlut_data.bt_vi[i]
              << " h: " << track.ptlut_data.bt_hi[i]
              << " c: " << track.ptlut_data.bt_ci[i]
              << " s: " << track.ptlut_data.bt_si[i]
              << std::endl;
      }
    }
  }

}

void EMTFBestTrackSelection::cancel_one_bx(
    const std::deque<EMTFTrackExtraCollection>& extended_best_track_cands,
    EMTFTrackExtraCollection& best_tracks
) const {
  const int max_z = NUM_ZONES;        // = 4 zones
  const int max_n = maxRoadsPerZone_; // = 3 candidates per zone
  const int max_zn = max_z * max_n;   // = 12 total candidates
  assert(maxTracks_ <= max_zn);

  // Emulate the arrays used in firmware
  typedef std::array<int, 3> segment_ref_t;
  std::vector<std::vector<segment_ref_t> > segments(max_zn, std::vector<segment_ref_t>());  // 2D array [zn][num segments]

  std::vector<std::vector<bool> > larger(max_zn, std::vector<bool>(max_zn, false));  // 2D array [zn][zn]
  std::vector<std::vector<bool> > winner(max_zn, std::vector<bool>(max_zn, false));

  std::vector<bool> exists (max_zn, false);  // 1D array [zn]
  std::vector<bool> killed (max_zn, false);
  std::vector<int>  rank   (max_zn, 0);
  //std::vector<int>  good_bx(max_zn, 0);

  // Initialize arrays: rank, segments
  for (int z = 0; z < max_z; ++z) {
    const EMTFTrackExtraCollection& tracks = extended_best_track_cands.at(z);
    const int ntracks = tracks.size();
    assert(ntracks <= max_n);

    for (int n = 0; n < ntracks; ++n) {
      const int zn = (n * max_z) + z;  // for (i = 0; i < 12; i = i+1) rank[i%4][i/4]
      const EMTFTrackExtra& track = tracks.at(n);

      rank.at(zn) = track.rank;

      for (const auto& conv_hit : track.xhits) {
        assert(conv_hit.valid);
	assert(!conv_hit.vetoed);  // Sanity check for RPCs

        // A segment identifier (chamber, strip, bx)
	int chamber_index = conv_hit.pc_station*9 + conv_hit.pc_chamber;
	int strip_index   = conv_hit.strip;
	// Offset RPCs so they don't cancel with CSCs
	if (conv_hit.subsystem == TriggerPrimitive::kRPC) {
	  chamber_index  = NUM_CSC_CHAMBERS + (conv_hit.pc_station - 1)*36 + conv_hit.pc_chamber*3 + (conv_hit.roll - 1);
	  assert(chamber_index <= NUM_CSC_CHAMBERS);  // Sanity check for RPCs 
	  strip_index    = conv_hit.strip_hi;
	}
        const segment_ref_t segment = {{chamber_index, strip_index, 0}};  // due to GCC bug, use {{}} instead of {}
        segments.at(zn).push_back(segment);
      }
    }  // end loop over n
  }  // end loop over z

  // Simultaneously compare each rank with each other
  int i=0, j=0, ri=0, rj=0, gt=0, eq=0, sum=0;

  for (i = 0; i < max_zn; ++i) {
    for (j = 0; j < max_zn; ++j) {
      larger[i][j] = 0;
    }
    larger[i][i] = 1; // result of comparison with itself
    //ri = rank[i%4][i/4]; // first index loops zone, second loops candidate. Zone loops faster, so we give equal priority to zones
    ri = rank[i];

    for (j = 0; j < max_zn; ++j) {
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
  for (i = 0; i < max_zn-1; ++i) { // candidate loop
    for (j = i+1; j < max_zn; ++j) { // comparison candidate loop
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
  for (i = 0; i < max_zn; ++i) {
    exists[i] = exists[i] & (!killed[i]);
  }

  bool anything_exists = (std::find(exists.begin(), exists.end(), 1) != exists.end());
  if (!anything_exists)
    return;

  // update "larger" array
  for (i = 0; i < max_zn; ++i) {
    for (j = 0; j < max_zn; ++j) {
      //if  (exists[i]) larger[i] = larger[i] | (~exists); // if this track exists make it larger than all non-existing tracks
      //else  larger[i] = 0; // else make it smaller than anything
      if (exists[i])
        larger[i][j] = larger[i][j] | (!exists[j]);
      else
        larger[i][j] = 0;
    }
  }

  if (verbose_ > 0) {  // debug
    std::cout << "exists: ";
    for (i = max_zn-1; i >= 0; --i) {
      std::cout << exists[i];
    }
    std::cout << std::endl;
    std::cout << "killed: ";
    for (i = max_zn-1; i >= 0; --i) {
      std::cout << killed[i];
    }
    std::cout << std::endl;
    for (j = 0; j < max_zn; ++j) {
      std::cout << "larger: ";
      for (i = max_zn-1; i >= 0; --i) {
        std::cout << larger[j][i];
      }
      std::cout << std::endl;
    }
  }

  // count zeros in the comparison results. The best track will have none, the next will have one, the third will have two
  // skip the bits corresponding to the comparison of the track with itself
  for (i = 0; i < max_zn; ++i) {
    sum = 0;
    for (j = 0; j < max_zn; ++j) {
      if (larger[i][j] == 0)
        sum += 1;
    }
    if (sum < maxTracks_)
      winner[sum][i] = 1; // assign positional winner codes
  }

  // Output best tracks according to winner signals
  best_tracks.clear();

  for (int o = 0; o < maxTracks_; ++o) { // output candidate loop
    int z = 0, n = 0;
    for (i = 0; i < max_zn; ++i) { // winner bit loop
      if (winner[o][i]) {
        n = i / max_z;
        z = i % max_z;

        const EMTFTrackExtraCollection& tracks = extended_best_track_cands.at(z);
        const EMTFTrackExtra& track = tracks.at(n);
        best_tracks.push_back(track);

        // Update winner, BX
        best_tracks.back().track_num = best_tracks.size() - 1;
        best_tracks.back().winner = o;
        best_tracks.back().bx = best_tracks.back().first_bx;
      }
    }
  }
}

void EMTFBestTrackSelection::cancel_multi_bx(
    const std::deque<EMTFTrackExtraCollection>& extended_best_track_cands,
    EMTFTrackExtraCollection& best_tracks
) const {
  const int max_h = bxWindow_;        // = 3 bx history
  const int max_z = NUM_ZONES;        // = 4 zones
  const int max_n = maxRoadsPerZone_; // = 3 candidates per zone
  const int max_zn = max_z * max_n;   // = 12 total candidates
  const int max_hzn = max_h * max_zn; // = 36 total candidates
  assert(maxTracks_ <= max_hzn);

  const int delayBX = bxWindow_ - 1;
  const int num_h = extended_best_track_cands.size() / max_z;  // num of bx history so far (??? - AWB 05.11.16)

  // Emulate the arrays used in firmware
  typedef std::array<int, 3> segment_ref_t;
  std::vector<std::vector<segment_ref_t> > segments(max_hzn, std::vector<segment_ref_t>());  // 2D array [hzn][num segments]

  std::vector<std::vector<bool> > larger(max_hzn, std::vector<bool>(max_hzn, false));  // 2D array [hzn][hzn]
  std::vector<std::vector<bool> > winner(max_hzn, std::vector<bool>(max_hzn, false));

  std::vector<bool> exists (max_hzn, false);  // 1D array [hzn]
  std::vector<bool> killed (max_hzn, false);
  std::vector<int>  rank   (max_hzn, 0);
  std::vector<int>  good_bx(max_hzn, 0);

  // Initialize arrays: rank, good_bx, segments
  for (int h = 0; h < num_h; ++h) {
    // extended_best_track_cands[0..3] has 4 zones for road/pattern BX-0 (i.e. current) with possible tracks from [BX-2, BX-1, BX-0]
    // extended_best_track_cands[4..7] has 4 zones for road/pattern BX-1 with possible tracks from [BX-3, BX-2, BX-1]
    // extended_best_track_cands[8..11] has 4 zones for road/pattern BX-2 with possible tracks from [BX-4, BX-3, BX-2]

    for (int z = 0; z < max_z; ++z) {
      const EMTFTrackExtraCollection& tracks = extended_best_track_cands.at(h*max_z + z);
      const int ntracks = tracks.size();
      assert(ntracks <= max_n);

      for (int n = 0; n < ntracks; ++n) {
        const int hzn = (h * max_z * max_n) + (n * max_z) + z;  // for (i = 0; i < 12; i = i+1) rank[i%4][i/4]
        const EMTFTrackExtra& track = tracks.at(n);
        int cand_bx = track.second_bx;
        cand_bx -= (bx_ - delayBX);  // convert track.second_bx=[BX-2, BX-1, BX-0] --> cand_bx=[0,1,2]

        rank.at(hzn) = track.rank;
        if (cand_bx == 0)
          good_bx.at(hzn) = 1;  // kill this rank if it's not the right BX

        for (const auto& conv_hit : track.xhits) {
          assert(conv_hit.valid);

          // A segment identifier (chamber, strip, bx)
	  // Need to redefine to avoid cancelling CSC hits with RPC hits - AWB 05.11.16
          const segment_ref_t segment = {{conv_hit.pc_station*9 + conv_hit.pc_chamber, conv_hit.strip, conv_hit.bx}};  // due to GCC bug, use {{}} instead of {}
          segments.at(hzn).push_back(segment);
        }
      }  // end loop over n
    }  // end loop over z
  }  // end loop over h

  // Simultaneously compare each rank with each other
  int i=0, j=0, ri=0, rj=0, sum=0;

  for (i = 0; i < max_hzn; ++i) {
    larger[i][i] = 1; // result of comparison with itself
    ri = rank[i];

    for (j = i+1; j < max_hzn; ++j) {
      // i and j bits show which rank is larger
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
  for (i = 0; i < max_hzn-1; ++i) { // candidate loop
    for (j = i+1; j < max_hzn; ++j) { // comparison candidate loop
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
  for (i = 0; i < max_hzn; ++i) {
    exists[i] = exists[i] & (!killed[i]);
  }

  // remove tracks that are not at correct BX number
  //exists = exists & good_bx;
  for (i = 0; i < max_hzn; ++i) {
    exists[i] = exists[i] & good_bx[i];
  }

  bool anything_exists = (std::find(exists.begin(), exists.end(), 1) != exists.end());
  if (!anything_exists)
    return;

  // update "larger" array
  for (i = 0; i < max_hzn; ++i) {
    for (j = 0; j < max_hzn; ++j) {
      if (exists[i])
        larger[i][j] = larger[i][j] | (!exists[j]);  // Any existing track is "larger" than any non-existing track
        // Undoes the cancellation with other BX? - AWB 05.11.16
      else
        larger[i][j] = 0;
    }
  }

  if (verbose_ > 0) {  // debug
    std::cout << "exists: ";
    for (i = max_hzn-1; i >= 0; --i) {
      std::cout << exists[i];
      if ((i%max_zn) == 0 && i != 0)  std::cout << "_";
    }
    std::cout << std::endl;
    std::cout << "killed: ";
    for (i = max_hzn-1; i >= 0; --i) {
      std::cout << killed[i];
      if ((i%max_zn) == 0 && i != 0)  std::cout << "_";
    }
    std::cout << std::endl;
    for (j = 0; j < max_hzn; ++j) {
      std::cout << "larger: ";
      for (i = max_hzn-1; i >= 0; --i) {
        std::cout << larger[j][i];
        if ((i%max_zn) == 0 && i != 0)  std::cout << "_";
      }
      std::cout << std::endl;
    }
  }

  // count zeros in the comparison results. The best track will have none, the next will have one, the third will have two
  for (i = 0; i < max_hzn; ++i) {
    sum = 0;
    for (j = 0; j < max_hzn; ++j) {
      if (larger[i][j] == 0)
        sum += 1;
    }
    if (sum < maxTracks_)
      winner[sum][i] = 1; // assign positional winner codes
  }

  // Output best tracks according to winner signals
  best_tracks.clear();

  for (int o = 0; o < maxTracks_; ++o) { // output candidate loop
    int h = 0, n = 0, z = 0;
    for (i = 0; i < max_hzn; ++i) { // winner bit loop
      if (winner[o][i]) {
        h = (i / max_z / max_n);
	if ( h != ((i / max_z) / max_n) )
	  std::cout << "I DON'T UNDERSTAAAAAND!!! - AWB" << std::endl;
        n = (i / max_z) % max_n;
        z = i % max_z;

        const EMTFTrackExtraCollection& tracks = extended_best_track_cands.at(h*max_z + z);
        const EMTFTrackExtra& track = tracks.at(n);
        best_tracks.push_back(track);

        // Update winner, BX
        best_tracks.back().track_num = best_tracks.size() - 1;
        best_tracks.back().winner = o;
        best_tracks.back().bx = best_tracks.back().second_bx;
      }
    }
  }
}
