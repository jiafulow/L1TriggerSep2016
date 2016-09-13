#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPrimitiveSelection.hh"

#include "DataFormats/MuonDetId/interface/DTChamberId.h"
#include "DataFormats/MuonDetId/interface/CSCDetId.h"
#include "DataFormats/MuonDetId/interface/RPCDetId.h"

using CSCData = TriggerPrimitive::CSCData;
using RPCData = TriggerPrimitive::RPCData;


// Specialized for CSC
template<>
int EMTFPrimitiveSelection::select(
    CSCTag tag,
    const TriggerPrimitive& muon_primitive
) {
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

// Specialized for RPC
template<>
int EMTFPrimitiveSelection::select(
    RPCTag tag,
    const TriggerPrimitive& muon_primitive
) {
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

// CSC functions
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
        if ((tp_subsector == 2) && (tp_csc_ID == 3 || tp_csc_ID == 6 || tp_csc_ID == 9 || tp_csc_ID == 12))
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
    if (tp_station == 1 && tp_csc_ID-1 >= 9) {  // ME1/1a
      selected = (tp_subsector-1) * 9 + (tp_csc_ID-1-9);
    } else if (tp_station == 1) {  // ME1/1b, ME1/2, ME1/3
      selected = (tp_subsector-1) * 9 + (tp_csc_ID-1);
    } else {  // ME2,3,4
      selected = (tp_station) * 9 + (tp_csc_ID-1);
    }

  } else {
    if (tp_station == 1 && tp_csc_ID-1 >= 9) {  // ME1/1a
      selected = (5) * 9 + (tp_csc_ID-1-9)/3;
    } else if (tp_station == 1) {  // ME1/1b, ME1/2, ME1/3
      selected = (5) * 9 + (tp_csc_ID-1)/3;
    } else {  // ME2,3,4
      selected = (5) * 9 + (tp_station) * 2 - 1 + (tp_csc_ID-1==6);
    }
  }

  return selected;
}

// RPC functions
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
