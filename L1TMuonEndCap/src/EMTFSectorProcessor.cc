#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFSectorProcessor.hh"


EMTFSectorProcessor::EMTFSectorProcessor() {

}

EMTFSectorProcessor::~EMTFSectorProcessor() {

}

void EMTFSectorProcessor::configure(
    const EMTFSectorProcessorLUT* lut,
    const EMTFPtAssignmentEngine* pt_assign_engine,
    int verbose, int endcap, int sector,
    int minBX, int maxBX, int bxWindow, int bxShiftCSC,
    bool includeNeighbor, bool duplicateTheta, bool fixZonePhi,
    const std::vector<int>& zoneBoundaries1, const std::vector<int>& zoneBoundaries2, int zoneOverlap,
    const std::vector<std::string>& pattDefinitions, const std::vector<std::string>& symPattDefinitions,
    int maxRoadsPerZone, int thetaWindow, int maxTracks,
    bool useSecondEarliest, bool useSymPatterns,
    bool readPtLUTFile, bool fixMode15HighPt, bool fix9bDPhi
) {
  assert(MIN_ENDCAP <= endcap && endcap <= MAX_ENDCAP);
  assert(MIN_TRIGSECTOR <= sector && sector <= MAX_TRIGSECTOR);

  assert(lut != nullptr);
  assert(pt_assign_engine != nullptr);

  lut_ = lut;

  pt_assign_engine_ = pt_assign_engine;

  verbose_  = verbose;
  endcap_   = endcap;
  sector_   = sector;

  minBX_       = minBX;
  maxBX_       = maxBX;
  bxWindow_    = bxWindow;
  bxShiftCSC_  = bxShiftCSC;

  includeNeighbor_ = includeNeighbor;
  duplicateTheta_  = duplicateTheta;
  fixZonePhi_      = fixZonePhi;

  zoneBoundaries1_    = zoneBoundaries1;
  zoneBoundaries2_    = zoneBoundaries2;
  zoneOverlap_        = zoneOverlap;
  pattDefinitions_    = pattDefinitions;
  symPattDefinitions_ = symPattDefinitions;
  maxRoadsPerZone_    = maxRoadsPerZone;
  thetaWindow_        = thetaWindow;
  maxTracks_          = maxTracks;
  useSecondEarliest_  = useSecondEarliest;
  useSymPatterns_     = useSymPatterns;

  readPtLUTFile_   = readPtLUTFile;
  fixMode15HighPt_ = fixMode15HighPt;
  fix9bDPhi_       = fix9bDPhi;
}

void EMTFSectorProcessor::process(
    EventNumber_t ievent,
    const TriggerPrimitiveCollection& muon_primitives,
    EMTFHitExtraCollection& out_hits,
    EMTFTrackExtraCollection& out_tracks
) const {

  //if (!(endcap_ == 1 && sector_ == 2))  return;  // debug

  // List of converted hits, extended from previous BXs
  std::deque<EMTFHitExtraCollection> extended_conv_hits;

  // List of best track candidates, extended from previous BXs
  std::deque<EMTFTrackExtraCollection> extended_best_track_cands;

  // Map of pattern detector --> lifetime, tracked across BXs
  std::map<pattern_ref_t, int> patt_lifetime_map;

  int delayBX = bxWindow_ - 1;

  for (int bx = minBX_; bx <= maxBX_ + delayBX; ++bx) {
    if (verbose_ > 0) {  // debug
      std::cout << "Endcap: " << endcap_ << " Sector: " << sector_ << " Event: " << ievent << " BX: " << bx << std::endl;
    }

    process_single_bx(
        bx,
        muon_primitives,
        out_hits,
        out_tracks,
        extended_conv_hits,
        extended_best_track_cands,
        patt_lifetime_map
    );

    // Drop earliest BX outside of BX window
    if (bx >= minBX_ + delayBX) {
      extended_conv_hits.pop_front();

      int n = zone_array<int>().size();
      extended_best_track_cands.erase(extended_best_track_cands.begin(), extended_best_track_cands.begin() + n);
    }
  }  // end loop over bx

  return;
}

void EMTFSectorProcessor::process_single_bx(
    int bx,
    const TriggerPrimitiveCollection& muon_primitives,
    EMTFHitExtraCollection& out_hits,
    EMTFTrackExtraCollection& out_tracks,
    std::deque<EMTFHitExtraCollection>& extended_conv_hits,
    std::deque<EMTFTrackExtraCollection>& extended_best_track_cands,
    std::map<pattern_ref_t, int>& patt_lifetime_map
) const {

  // ___________________________________________________________________________
  // Configure

  EMTFPrimitiveSelection prim_sel;
  prim_sel.configure(
      verbose_, endcap_, sector_, bx,
      includeNeighbor_, duplicateTheta_
  );

  EMTFPrimitiveConversion prim_conv;
  prim_conv.configure(
      lut_,
      verbose_, endcap_, sector_, bx,
      bxShiftCSC_,
      duplicateTheta_, fixZonePhi_,
      zoneBoundaries1_, zoneBoundaries2_, zoneOverlap_
  );

  EMTFPatternRecognition patt_recog;
  patt_recog.configure(
      verbose_, endcap_, sector_, bx,
      bxWindow_,
      pattDefinitions_, symPattDefinitions_,
      maxRoadsPerZone_, useSecondEarliest_, useSymPatterns_
  );

  EMTFPrimitiveMatching prim_match;
  prim_match.configure(
      verbose_, endcap_, sector_, bx,
      fixZonePhi_
  );

  EMTFAngleCalculation angle_calc;
  angle_calc.configure(
      verbose_, endcap_, sector_, bx,
      bxWindow_,
      thetaWindow_
  );

  EMTFBestTrackSelection btrack_sel;
  btrack_sel.configure(
      verbose_, endcap_, sector_, bx,
      bxWindow_,
      maxRoadsPerZone_, maxTracks_, useSecondEarliest_
  );

  EMTFPtAssignment pt_assign;
  pt_assign.configure(
      pt_assign_engine_,
      verbose_, endcap_, sector_, bx,
      readPtLUTFile_, fixMode15HighPt_, fix9bDPhi_
  );

  std::map<int, TriggerPrimitiveCollection> selected_csc_map;
  std::map<int, TriggerPrimitiveCollection> selected_rpc_map;

  EMTFHitExtraCollection conv_hits;

  zone_array<EMTFRoadExtraCollection> zone_roads;  // each zone has its road collection

  zone_array<EMTFTrackExtraCollection> zone_tracks;  // each zone has its track collection

  EMTFTrackExtraCollection best_tracks;

  // ___________________________________________________________________________
  // Process

  // Select muon primitives that belong to this sector and this BX.
  // Put them into maps with an index that roughly corresponds to
  // each input link
  prim_sel.process(CSCTag(), muon_primitives, selected_csc_map);
  prim_sel.process(RPCTag(), muon_primitives, selected_rpc_map);

  // Convert trigger primitives into "converted hits"
  // A converted hit consists of integer representations of phi, theta, and zones
  prim_conv.process(CSCTag(), selected_csc_map, conv_hits);
  prim_conv.process(RPCTag(), selected_rpc_map, conv_hits);
  extended_conv_hits.push_back(conv_hits);

  // Detect patterns in all zones, find 3 best roads in each zone
  patt_recog.process(extended_conv_hits, patt_lifetime_map, zone_roads);

  // Match the trigger primitives to the roads, create tracks
  prim_match.process(extended_conv_hits, zone_roads, zone_tracks);

  // Calculate deflection angles and other track variables
  angle_calc.process(zone_tracks);
  extended_best_track_cands.insert(extended_best_track_cands.end(), zone_tracks.begin(), zone_tracks.end());

  // Identify 3 best tracks
  btrack_sel.process(extended_best_track_cands, best_tracks);

  // Construct pT address, assign pT
  pt_assign.process(best_tracks);

  // ___________________________________________________________________________
  // Output

  out_hits.insert(out_hits.end(), conv_hits.begin(), conv_hits.end());
  out_tracks.insert(out_tracks.end(), best_tracks.begin(), best_tracks.end());

  return;
}
