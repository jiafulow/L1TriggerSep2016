#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPrimitiveSelection.hh"

#include "DataFormats/MuonDetId/interface/DTChamberId.h"
#include "DataFormats/MuonDetId/interface/CSCDetId.h"
#include "DataFormats/MuonDetId/interface/RPCDetId.h"

#define NUM_CSC_CHAMBERS 6*9
#define NUM_RPC_CHAMBERS 6*9  // not implemented

using CSCData = TriggerPrimitive::CSCData;
using RPCData = TriggerPrimitive::RPCData;


void EMTFPrimitiveSelection::configure(
      int verbose, int endcap, int sector, int bx,
      bool includeNeighbor, bool duplicateTheta
) {
  verbose_ = verbose;
  endcap_  = endcap;
  sector_  = sector;
  bx_      = bx;

  includeNeighbor_ = includeNeighbor;
  duplicateTheta_  = duplicateTheta;
}

// Specialized for CSC
template<>
void EMTFPrimitiveSelection::process(
    CSCTag tag,
    const TriggerPrimitiveCollection& muon_primitives,
    std::map<int, TriggerPrimitiveCollection>& selected_csc_map
) const {
  TriggerPrimitiveCollection::const_iterator tp_it  = muon_primitives.begin();
  TriggerPrimitiveCollection::const_iterator tp_end = muon_primitives.end();

  for (; tp_it != tp_end; ++tp_it) {
    int selected_csc = select_csc(*tp_it);  // CSC

    if (selected_csc >= 0) {
      assert(selected_csc < NUM_CSC_CHAMBERS);
      selected_csc_map[selected_csc].push_back(*tp_it);
    }
  }

  // Duplicate CSC muon primitives
  // If there are 2 LCTs in the same chamber with (strip, wire) = (s1, w1) and (s2, w2)
  // make all combinations with (s1, w1), (s2, w1), (s1, w2), (s2, w2)
  if (duplicateTheta_) {
    std::map<int, TriggerPrimitiveCollection>::iterator map_tp_it  = selected_csc_map.begin();
    std::map<int, TriggerPrimitiveCollection>::iterator map_tp_end = selected_csc_map.end();

    for (; map_tp_it != map_tp_end; ++map_tp_it) {
      //int selected = map_tp_it->first;
      TriggerPrimitiveCollection& tmp_primitives = map_tp_it->second;  // pass by reference
      assert(tmp_primitives.size() <= 2);  // at most 2

      if (tmp_primitives.size() == 2) {
        if (
            (tmp_primitives.at(0).getStrip() != tmp_primitives.at(1).getStrip()) &&
            (tmp_primitives.at(0).getWire() != tmp_primitives.at(1).getWire())
        ) {
          // Swap wire numbers
          TriggerPrimitive tp0 = tmp_primitives.at(0);  // (s1,w1)
          TriggerPrimitive tp1 = tmp_primitives.at(1);  // (s2,w2)

          TriggerPrimitive::CSCData tp0_data_tmp = tp0.getCSCData();
          TriggerPrimitive::CSCData tp0_data     = tp0.getCSCData();
          TriggerPrimitive::CSCData tp1_data     = tp1.getCSCData();
          tp0_data.keywire = tp1_data.keywire;
          tp1_data.keywire = tp0_data_tmp.keywire;
          tp0.setCSCData(tp0_data);  // (s1,w2)
          tp1.setCSCData(tp1_data);  // (s2,w1)

          tmp_primitives.insert(tmp_primitives.begin()+1, tp1);  // (s2,w1) at 2nd pos
          tmp_primitives.insert(tmp_primitives.begin()+2, tp0);  // (s1,w2) at 3rd pos
        }
      }  // end if tmp_primitives.size() == 2
    }  // end loop over selected_csc_map
  }
}

// Specialized for RPC
template<>
void EMTFPrimitiveSelection::process(
    RPCTag tag,
    const TriggerPrimitiveCollection& muon_primitives,
    std::map<int, TriggerPrimitiveCollection>& selected_rpc_map
) const {
  TriggerPrimitiveCollection::const_iterator tp_it  = muon_primitives.begin();
  TriggerPrimitiveCollection::const_iterator tp_end = muon_primitives.end();

  for (; tp_it != tp_end; ++tp_it) {
    int selected_rpc = select_rpc(*tp_it);  // RPC

    if (selected_rpc >= 0) {
      assert(selected_rpc < NUM_RPC_CHAMBERS);
      selected_rpc_map[selected_rpc].push_back(*tp_it);
    }
  }
}


// CSC functions
int EMTFPrimitiveSelection::select_csc(const TriggerPrimitive& muon_primitive) const {
  int selected = -1;

  if (muon_primitive.subsystem() == TriggerPrimitive::kCSC) {
    const CSCDetId tp_detId = muon_primitive.detId<CSCDetId>();
    const CSCData& tp_data  = muon_primitive.getCSCData();

    int tp_endcap    = tp_detId.endcap();
    int tp_sector    = tp_detId.triggerSector();
    int tp_station   = tp_detId.station();
    int tp_chamber   = tp_detId.chamber();

    int tp_bx        = tp_data.bx;
    int tp_csc_ID    = tp_data.cscID;

    // station 1 --> subsector 1 or 2
    // station 2,3,4 --> subsector 0
    int tp_subsector = (tp_station != 1) ? 0 : ((tp_chamber%6 > 2) ? 1 : 2);

    assert(MIN_ENDCAP <= tp_endcap && tp_endcap <= MAX_ENDCAP);
    assert(MIN_TRIGSECTOR <= tp_sector && tp_sector <= MAX_TRIGSECTOR);
    assert(1 <= tp_station && tp_station <= 4);
    assert(1 <= tp_csc_ID && tp_csc_ID <= 9);

    if (is_in_bx_csc(tp_bx)) {
      if (is_in_sector_csc(tp_endcap, tp_sector)) {
        selected = get_index_csc(tp_subsector, tp_station, tp_csc_ID, false);

      } else if (is_in_neighbor_sector_csc(tp_endcap, tp_sector, tp_subsector, tp_station, tp_csc_ID)) {
        selected = get_index_csc(tp_subsector, tp_station, tp_csc_ID, true);

      }
    }
  }

  return selected;
}

bool EMTFPrimitiveSelection::is_in_sector_csc(int tp_endcap, int tp_sector) const {
  return ((endcap_ == tp_endcap) && (sector_ == tp_sector));
}

bool EMTFPrimitiveSelection::is_in_neighbor_sector_csc(
    int tp_endcap, int tp_sector, int tp_subsector, int tp_station, int tp_csc_ID
) const {
  auto get_neighbor = [](int sector) {
    return (sector == 1) ? 6 : sector - 1;
  };

  if (includeNeighbor_) {
    if ((endcap_ == tp_endcap) && (get_neighbor(sector_) == tp_sector)) {
      if (tp_station == 1) {
        if ((tp_subsector == 2) && (tp_csc_ID == 3 || tp_csc_ID == 6 || tp_csc_ID == 9))
          return true;

      } else {
        if (tp_csc_ID == 3 || tp_csc_ID == 9)
          return true;
      }
    }
  }
  return false;
}

bool EMTFPrimitiveSelection::is_in_bx_csc(int tp_bx) const {
  tp_bx -= 6;
  return (bx_ == tp_bx);
}

int EMTFPrimitiveSelection::get_index_csc(int tp_subsector, int tp_station, int tp_csc_ID, bool is_neighbor) const {
  int selected = -1;

  if (!is_neighbor) {
    if (tp_station == 1) {  // ME1
      selected = (tp_subsector-1) * 9 + (tp_csc_ID-1);
    } else {  // ME2,3,4
      selected = (tp_station) * 9 + (tp_csc_ID-1);
    }

  } else {
    if (tp_station == 1) {  // ME1
      selected = (5) * 9 + (tp_csc_ID-1)/3;
    } else {  // ME2,3,4
      selected = (5) * 9 + (tp_station) * 2 - 1 + (tp_csc_ID-1 < 3 ? 0 : 1);
    }
  }

  return selected;
}

// RPC functions
int EMTFPrimitiveSelection::select_rpc(const TriggerPrimitive& muon_primitive) const {
  int selected = -1;

  if (muon_primitive.subsystem() == TriggerPrimitive::kRPC) {
    const RPCDetId tp_detId = muon_primitive.detId<RPCDetId>();
    const RPCData& tp_data  = muon_primitive.getRPCData();

    int tp_region    = tp_detId.region();  // 0 for Barrel, +/-1 for +/- Endcap
    int tp_endcap    = (tp_region == -1) ? 2 : tp_region;
    int tp_sector    = tp_detId.sector();
    int tp_subsector = tp_detId.subsector();
    int tp_station   = tp_detId.station();
    int tp_ring      = tp_detId.ring();
    int tp_roll      = tp_detId.roll();

    int tp_bx        = tp_data.bx;

    if (is_in_bx_rpc(tp_bx)) {
      if (is_in_sector_rpc(tp_endcap, tp_sector)) {
        selected = get_index_rpc(tp_subsector, tp_station, tp_ring, tp_roll, false);

      } else if (is_in_neighbor_sector_rpc(tp_endcap, tp_sector, tp_subsector, tp_station, tp_ring, tp_roll)) {
        selected = get_index_rpc(tp_subsector, tp_station, tp_ring, tp_roll, true);
      }
    }
  }

  return selected;
}

bool EMTFPrimitiveSelection::is_in_sector_rpc(int tp_endcap, int tp_sector) const {
  return ((endcap_ == tp_endcap) && (sector_ == tp_sector));
}

bool EMTFPrimitiveSelection::is_in_neighbor_sector_rpc(
    int tp_endcap, int tp_sector, int tp_subsector, int tp_station, int tp_ring, int tp_roll
) const {
  return false;
}

bool EMTFPrimitiveSelection::is_in_bx_rpc(int tp_bx) const {
  return (bx_ == tp_bx);
}

int EMTFPrimitiveSelection::get_index_rpc(int tp_subsector, int tp_station, int tp_ring, int tp_roll, bool is_neighbor) const {
  int selected = -1;

  selected = 0;  //TODO: implement this

  return selected;
}
