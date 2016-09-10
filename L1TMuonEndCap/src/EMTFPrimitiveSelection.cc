#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPrimitiveSelection.hh"

#include "DataFormats/MuonDetId/interface/DTChamberId.h"
#include "DataFormats/MuonDetId/interface/CSCDetId.h"
#include "DataFormats/MuonDetId/interface/RPCDetId.h"

using CSCData = TriggerPrimitive::CSCData;
using RPCData = TriggerPrimitive::RPCData;


EMTFPrimitiveSelection::EMTFPrimitiveSelection() {

}

EMTFPrimitiveSelection::~EMTFPrimitiveSelection() {

}

// Specialized for CSC
template<>
int EMTFPrimitiveSelection::select(
    CSCTag tag,
    int sector, int bx, bool includeNeighbor,
    const TriggerPrimitive& muon_primitive
) {
  includeNeighbor_ = includeNeighbor;
  sector_          = sector;
  bx_              = bx;

  int selected = 0;

  if (muon_primitive.subsystem() == TriggerPrimitive::kCSC) {
    const CSCDetId tp_detId = muon_primitive.detId<CSCDetId>();
    const CSCData& tp_data  = muon_primitive.getCSCData();

    int tp_endcap  = tp_detId.endcap();
    int tp_sector  = tp_detId.triggerSector();
    int tp_station = tp_detId.station();
    int tp_chamber = tp_detId.chamber();

    int tp_bx      = tp_data.bx;
    int tp_csc_ID  = tp_data.cscID;

    // station 1 --> subsector 1 or 2
    // station 2,3,4 --> subsector 0
    int tp_subsector = (tp_station != 1) ? 0 : ((tp_chamber%6 > 2) ? 1 : 2);

    // positive endcap --> sectors 0-5
    // negative endcap --> sectors 6-11
    tp_sector = (tp_endcap - 1) * 6 + (tp_sector - 1);

    if (is_in_bx_csc(tp_bx)) {
      if (is_in_sector_csc(tp_sector)) {
        selected = 1;
      } else if (is_in_neighbor_sector_csc(tp_sector, tp_subsector, tp_station, tp_csc_ID)) {
        selected = 2;
      }
    }
  }

  return selected;
}

// Specialized for RPC
template<>
int EMTFPrimitiveSelection::select(
    RPCTag tag,
    int sector, int bx, bool includeNeighbor,
    const TriggerPrimitive& muon_primitive
) {
  includeNeighbor_ = includeNeighbor;
  sector_          = sector;
  bx_              = bx;

  int selected = 0;

  if (muon_primitive.subsystem() == TriggerPrimitive::kRPC) {
    const RPCDetId tp_detId = muon_primitive.detId<RPCDetId>();
    const RPCData& tp_data  = muon_primitive.getRPCData();

    int tp_region  = tp_detId.region();  // 0 for Barrel, +/-1 for +/- Endcap
    int tp_sector  = tp_detId.sector();
    int tp_endcap  = (tp_region == 1) ? 1 : 2;

    int tp_bx      = tp_data.bx;

    // positive endcap --> sectors 0-5
    // negative endcap --> sectors 6-11
    tp_sector = (tp_endcap - 1) * 6 + (tp_sector - 1);

    if (is_in_bx_rpc(tp_bx)) {
      if (is_in_sector_rpc(tp_sector)) {
        selected = 1;
      } else if (is_in_neighbor_sector_rpc(tp_sector)) {
        selected = 2;
      }
    }
  }

  return selected;
}

// CSC functions
bool EMTFPrimitiveSelection::is_in_sector_csc(int tp_sector) const {
  return (sector_ == tp_sector);
}

bool EMTFPrimitiveSelection::is_in_neighbor_sector_csc(
    int tp_sector, int tp_subsector, int tp_station, int tp_csc_ID
) const {
  static const std::vector<int> neighboring = {
    5, 0, 1, 2, 3, 4,
    11, 6, 7, 8, 9, 10
  };

  if (includeNeighbor_) {
    if (neighboring.at(sector_) == tp_sector) {
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

// RPC functions
bool EMTFPrimitiveSelection::is_in_sector_rpc(int tp_sector) const {
  return (sector_ == tp_sector);
}

bool EMTFPrimitiveSelection::is_in_neighbor_sector_rpc(
    int tp_sector
) const {
  return false;
}

bool EMTFPrimitiveSelection::is_in_bx_rpc(int tp_bx) const {
  return (bx_ == tp_bx);
}
