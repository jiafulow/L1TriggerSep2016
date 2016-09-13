#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFSectorProcessor.hh"

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPrimitiveSelection.hh"
#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPrimitiveConversion.hh"

#define NUM_CSC_CHAMBERS 6*9
#define NUM_RPC_CHAMBERS 6*9  // ??


void EMTFSectorProcessor::configure(
    int endcap, int sector,
    int minBX, int maxBX, int bxWindow,
    bool includeNeighbor, bool duplicateWires
) {
  assert(MIN_ENDCAP <= endcap && endcap <= MAX_ENDCAP);
  assert(MIN_TRIGSECTOR <= sector && sector <= MAX_TRIGSECTOR);

  endcap_ = endcap;
  sector_ = sector;

  minBX_    = minBX;
  maxBX_    = maxBX;
  bxWindow_ = bxWindow;

  includeNeighbor_ = includeNeighbor;
  duplicateWires_ = duplicateWires;
}

void EMTFSectorProcessor::process(
    const TriggerPrimitiveCollection& muon_primitives,
    EMTFHitExtraCollection& out_hits,
    EMTFTrackExtraCollection& out_tracks
) {

  int driftBX = bxWindow_ - 1;

  for (int ibx = minBX_; ibx <= maxBX_ + driftBX; ibx++) {
    process_single_bx(ibx, muon_primitives, out_hits, out_tracks);
  }

  return;
}

void EMTFSectorProcessor::process_single_bx(
    int bx,
    const TriggerPrimitiveCollection& muon_primitives,
    EMTFHitExtraCollection& out_hits,
    EMTFTrackExtraCollection& out_tracks
) {

  EMTFPrimitiveSelection prim_sel;
  prim_sel.configure(endcap_, sector_, bx, includeNeighbor_);

  TriggerPrimitiveCollection::const_iterator tp_it  = muon_primitives.begin();
  TriggerPrimitiveCollection::const_iterator tp_end = muon_primitives.end();

  // Select muon primitives that belong to this sector and this BX
  // Assign the muon primitives with an index 0-53 that roughly corresponds to
  // an input link
  std::map<int, std::vector<TriggerPrimitive> > selected_csc_map;

  for (; tp_it != tp_end; ++tp_it) {
    int selected_csc = prim_sel.select(CSCTag(), *tp_it);

    if (selected_csc >= 0) {
      assert(selected_csc < NUM_CSC_CHAMBERS);
      selected_csc_map[selected_csc].push_back(*tp_it);
    }
  }

  std::map<int, std::vector<TriggerPrimitive> > selected_rpc_map;

  for (; tp_it != tp_end; ++tp_it) {
    int selected_rpc = prim_sel.select(RPCTag(), *tp_it);

    if (selected_rpc >= 0) {
      assert(selected_rpc < NUM_RPC_CHAMBERS);
      selected_rpc_map[selected_rpc].push_back(*tp_it);
    }
  }

  // Duplicate CSC muon primitives
  // If there are 2 LCTs in the same chamber with (strip, wire) = (s1, w1) and (s2, w2)
  // make all combinations with (s1, w1), (s2, w2), (s1, w2), (s2, w1)
  if (duplicateWires_) {
    std::map<int, std::vector<TriggerPrimitive> >::iterator map_tp_it  = selected_csc_map.begin();
    std::map<int, std::vector<TriggerPrimitive> >::iterator map_tp_end = selected_csc_map.end();

    for (; map_tp_it != map_tp_end; ++map_tp_it) {
      int selected = map_tp_it->first;
      std::vector<TriggerPrimitive>& tmp_primitives = map_tp_it->second;  // pass by reference

      if (selected < NUM_CSC_CHAMBERS) {  // CSC
        assert(tmp_primitives.size() <= 2);  // at most 2

        if (tmp_primitives.size() == 2) {
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

          tmp_primitives.push_back(tp0);
          tmp_primitives.push_back(tp1);
        }
      }
    }
  }


  EMTFHitExtraCollection conv_hits;

  EMTFPrimitiveConversion prim_conv;
  prim_conv.configure(endcap_, sector_);

  std::map<int, std::vector<TriggerPrimitive> >::const_iterator map_tp_it  = selected_csc_map.begin();
  std::map<int, std::vector<TriggerPrimitive> >::const_iterator map_tp_end = selected_csc_map.end();

  // Convert trigger primitives
  for (; map_tp_it != map_tp_end; ++map_tp_it) {
    int selected = map_tp_it->first;
    TriggerPrimitiveCollection::const_iterator tp_it  = map_tp_it->second.begin();
    TriggerPrimitiveCollection::const_iterator tp_end = map_tp_it->second.end();

    for (; tp_it != tp_end; ++tp_it) {
      const EMTFHitExtra& conv_hit = prim_conv.convert(CSCTag(), selected, *tp_it);
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
      const EMTFHitExtra& conv_hit = prim_conv.convert(RPCTag(), selected, *tp_it);
      conv_hits.push_back(conv_hit);
    }
  }

  out_hits.insert(out_hits.end(), conv_hits.begin(), conv_hits.end());

  return;
}
