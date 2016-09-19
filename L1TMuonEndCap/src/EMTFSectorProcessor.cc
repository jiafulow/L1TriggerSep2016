#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFSectorProcessor.hh"

#define NUM_CSC_CHAMBERS 6*9
#define NUM_RPC_CHAMBERS 6*9  // ??


EMTFSectorProcessor::EMTFSectorProcessor() {

}

EMTFSectorProcessor::~EMTFSectorProcessor() {

}

void EMTFSectorProcessor::configure(
    const EMTFSectorProcessorLUT* lut,
    int endcap, int sector,
    int minBX, int maxBX, int bxWindow,
    const std::vector<int>& zoneBoundaries1, const std::vector<int>& zoneBoundaries2, int zoneOverlap,
    const std::vector<std::string>& pattDefinitions,
    bool includeNeighbor, bool duplicateWires
) {
  assert(MIN_ENDCAP <= endcap && endcap <= MAX_ENDCAP);
  assert(MIN_TRIGSECTOR <= sector && sector <= MAX_TRIGSECTOR);
  assert(lut != nullptr);

  lut_ = lut;

  endcap_ = endcap;
  sector_ = sector;

  minBX_           = minBX;
  maxBX_           = maxBX;
  bxWindow_        = bxWindow;
  zoneBoundaries1_ = zoneBoundaries1;
  zoneBoundaries2_ = zoneBoundaries2;
  zoneOverlap_     = zoneOverlap;
  pattDefinitions_ = pattDefinitions;

  includeNeighbor_ = includeNeighbor;
  duplicateWires_ = duplicateWires;
}

void EMTFSectorProcessor::process(
    const TriggerPrimitiveCollection& muon_primitives,
    EMTFHitExtraCollection& out_hits,
    EMTFTrackExtraCollection& out_tracks
) {

  // List of converted hits, extended from previous BXs
  std::deque<EMTFHitExtraCollection> extended_conv_hits;

  // Map of pattern detector --> lifetime, tracked across BXs
  std::map<EMTFPatternId, int> patt_lifetime_map;

  int delayBX = bxWindow_ - 1;  // = 2

  for (int ibx = minBX_; ibx <= maxBX_ + delayBX; ++ibx) {
    process_single_bx(ibx, muon_primitives, out_hits, out_tracks, extended_conv_hits, patt_lifetime_map);

    if (ibx >= minBX_ + delayBX) {
      extended_conv_hits.pop_front();
    }
  }

  return;
}

void EMTFSectorProcessor::process_single_bx(
    int bx,
    const TriggerPrimitiveCollection& muon_primitives,
    EMTFHitExtraCollection& out_hits,
    EMTFTrackExtraCollection& out_tracks,
    std::deque<EMTFHitExtraCollection>& extended_conv_hits,
    std::map<EMTFPatternId, int>& patt_lifetime_map
) {
  // Instantiate stuff
  EMTFPrimitiveSelection prim_sel;
  prim_sel.configure(endcap_, sector_, bx, includeNeighbor_);

  EMTFPrimitiveConversion prim_conv;
  prim_conv.configure(
      lut_,
      endcap_, sector_,
      zoneBoundaries1_, zoneBoundaries2_, zoneOverlap_
  );

  EMTFPatternRecognition patt_recog;
  patt_recog.configure(
      endcap_, sector_, bx,
      minBX_, maxBX_, bxWindow_,
      pattDefinitions_
  );

  std::map<int, std::vector<TriggerPrimitive> > selected_csc_map;
  std::map<int, std::vector<TriggerPrimitive> > selected_rpc_map;

  EMTFHitExtraCollection conv_hits;


  // Select muon primitives that belong to this sector and this BX
  // Assign the muon primitives with an index 0-53 that roughly corresponds to
  // the input link
  TriggerPrimitiveCollection::const_iterator tp_it  = muon_primitives.begin();
  TriggerPrimitiveCollection::const_iterator tp_end = muon_primitives.end();

  for (; tp_it != tp_end; ++tp_it) {
    int selected_csc = prim_sel.select(CSCTag(), *tp_it);  // CSC

    if (selected_csc >= 0) {
      assert(selected_csc < NUM_CSC_CHAMBERS);
      selected_csc_map[selected_csc].push_back(*tp_it);
    }
  }

  for (; tp_it != tp_end; ++tp_it) {
    int selected_rpc = prim_sel.select(RPCTag(), *tp_it);  // RPC

    if (selected_rpc >= 0) {
      assert(selected_rpc < NUM_RPC_CHAMBERS);
      selected_rpc_map[selected_rpc].push_back(*tp_it);
    }
  }

  // Duplicate CSC muon primitives
  // If there are 2 LCTs in the same chamber with (strip, wire) = (s1, w1) and (s2, w2)
  // make all combinations with (s1, w1), (s2, w1), (s1, w2), (s2, w2)
  if (duplicateWires_) {
    std::map<int, std::vector<TriggerPrimitive> >::iterator map_tp_it  = selected_csc_map.begin();
    std::map<int, std::vector<TriggerPrimitive> >::iterator map_tp_end = selected_csc_map.end();

    for (; map_tp_it != map_tp_end; ++map_tp_it) {
      //int selected = map_tp_it->first;
      std::vector<TriggerPrimitive>& tmp_primitives = map_tp_it->second;  // pass by reference
      assert(tmp_primitives.size() <= 2);  // at most 2

      if (tmp_primitives.size() == 2) {
        if (
            (tmp_primitives.at(0).getStrip() != tmp_primitives.at(1).getStrip()) &&
            (tmp_primitives.at(0).getWire() != tmp_primitives.at(1).getWire())
        ) {
          // Swap wire numbers
          TriggerPrimitive tp0 = tmp_primitives.at(0);  // clone
          TriggerPrimitive tp1 = tmp_primitives.at(1);  // clone

          TriggerPrimitive::CSCData tp0_data_tmp = tp0.getCSCData();
          TriggerPrimitive::CSCData tp0_data     = tp0.getCSCData();
          TriggerPrimitive::CSCData tp1_data     = tp1.getCSCData();
          tp0_data.keywire = tp1_data.keywire;
          tp1_data.keywire = tp0_data_tmp.keywire;
          tp0.setCSCData(tp0_data);
          tp1.setCSCData(tp1_data);

          tmp_primitives.insert(tmp_primitives.begin()+1, tp1);
          tmp_primitives.insert(tmp_primitives.begin()+2, tp0);
        }
      }  // end if tmp_primitives.size() == 2
    }  // end loop over selected_csc_map
  }

  // Convert trigger primitives into "converted hits"
  // A converted hit consists of integer representations of phi, theta, and zones
  std::map<int, std::vector<TriggerPrimitive> >::const_iterator map_tp_it  = selected_csc_map.begin();
  std::map<int, std::vector<TriggerPrimitive> >::const_iterator map_tp_end = selected_csc_map.end();

  for (; map_tp_it != map_tp_end; ++map_tp_it) {
    int selected = map_tp_it->first;
    TriggerPrimitiveCollection::const_iterator tp_it  = map_tp_it->second.begin();
    TriggerPrimitiveCollection::const_iterator tp_end = map_tp_it->second.end();

    for (; tp_it != tp_end; ++tp_it) {
      const EMTFHitExtra& conv_hit = prim_conv.convert(CSCTag(), selected, *tp_it);  // CSC
      conv_hits.push_back(conv_hit);
    }
  }

  map_tp_it  = selected_rpc_map.begin();
  map_tp_end = selected_rpc_map.end();

  for (; map_tp_it != map_tp_end; ++map_tp_it) {
    int selected = map_tp_it->first;
    TriggerPrimitiveCollection::const_iterator tp_it  = map_tp_it->second.begin();
    TriggerPrimitiveCollection::const_iterator tp_end = map_tp_it->second.end();

    for (; tp_it != tp_end; ++tp_it) {
      const EMTFHitExtra& conv_hit = prim_conv.convert(RPCTag(), selected, *tp_it);  // RPC
      conv_hits.push_back(conv_hit);
    }
  }

  // Perform pattern recognition
  extended_conv_hits.push_back(conv_hits);

  patt_recog.detect(extended_conv_hits, patt_lifetime_map);


  out_hits.insert(out_hits.end(), conv_hits.begin(), conv_hits.end());

  return;
}
