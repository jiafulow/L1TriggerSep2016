#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFSectorProcessor.hh"

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPrimitiveSelection.hh"
#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPrimitiveConversion.hh"


void EMTFSectorProcessor::configure(
    int endcap, int sector,
    int minBX, int maxBX, int bxWindow,
    bool includeNeighbor
) {
  assert(MIN_ENDCAP <= endcap && endcap <= MAX_ENDCAP);
  assert(MIN_TRIGSECTOR <= sector && sector <= MAX_TRIGSECTOR);

  endcap_ = endcap;
  sector_ = sector;

  minBX_    = minBX;
  maxBX_    = maxBX;
  bxWindow_ = bxWindow;

  includeNeighbor_ = includeNeighbor;
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

  EMTFHitExtraCollection conv_hits;

  EMTFPrimitiveSelection prim_sel;
  prim_sel.configure(endcap_, sector_, bx, includeNeighbor_);

  EMTFPrimitiveConversion prim_conv;
  prim_conv.configure(endcap_, sector_);

  TriggerPrimitiveCollection::const_iterator tp_it  = muon_primitives.begin();
  TriggerPrimitiveCollection::const_iterator tp_end = muon_primitives.end();

  for (; tp_it != tp_end; ++tp_it) {
    int selected_csc = prim_sel.select(CSCTag(), *tp_it);
    int selected_rpc = prim_sel.select(RPCTag(), *tp_it);

    if (selected_csc) {
      bool is_neighbor = (selected_csc == 2);
      const EMTFHitExtra& conv_hit = prim_conv.convert(CSCTag(), is_neighbor, *tp_it);
      conv_hits.push_back(conv_hit);

    } else if (selected_rpc) {
      bool is_neighbor = (selected_rpc == 2);
      const EMTFHitExtra& conv_hit = prim_conv.convert(RPCTag(), is_neighbor, *tp_it);
      conv_hits.push_back(conv_hit);
    }
  }




  out_hits.insert(out_hits.end(), conv_hits.begin(), conv_hits.end());

  return;
}
