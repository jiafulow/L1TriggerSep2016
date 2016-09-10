#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPrimitiveConversion.hh"

#include "DataFormats/MuonDetId/interface/DTChamberId.h"
#include "DataFormats/MuonDetId/interface/CSCDetId.h"
#include "DataFormats/MuonDetId/interface/RPCDetId.h"

using CSCData = TriggerPrimitive::CSCData;
using EMTFHitData = EMTFHitExtra::EMTFHitData;


EMTFPrimitiveConversion::EMTFPrimitiveConversion() {

}

EMTFPrimitiveConversion::~EMTFPrimitiveConversion() {

}

void EMTFPrimitiveConversion::convert(
    int sector, int bx, bool includeNeighbor,
    const TriggerPrimitiveCollection& muon_primitives,
    EMTFHitExtraCollection& conv_hits
) {

  includeNeighbor_ = includeNeighbor;
  sector_          = sector;
  bx_              = bx;

  conv_hits.clear();

  TriggerPrimitiveCollection::const_iterator tp_it  = muon_primitives.begin();
  TriggerPrimitiveCollection::const_iterator tp_end = muon_primitives.end();

  for (; tp_it != tp_end; ++tp_it) {

    if (tp_it->subsystem() == TriggerPrimitive::kCSC) {
      const CSCDetId tp_detId = tp_it->detId<CSCDetId>();
      const CSCData& tp_data  = tp_it->getCSCData();

      int is_neighbor = 0;

      int tp_endcap    = tp_detId.endcap();
      int tp_sector    = tp_detId.triggerSector();
      int tp_station   = tp_detId.station();
      int tp_chamber   = tp_detId.chamber();

      // station 1 --> subsector 1 or 2
      // station 2,3,4 --> subsector 0
      int tp_subsector = (tp_station != 1) ? 0 : ((tp_chamber%6 > 2) ? 1 : 2);

      if (
          is_in_sector_csc(tp_endcap, tp_sector, tp_subsector, tp_station, tp_data.cscID, is_neighbor) &&
          is_in_bx_csc(tp_data.bx)
      ) {

        const EMTFHitExtra& conv_hit = make_emtf_hit(*tp_it, is_neighbor);
        conv_hits.push_back(conv_hit);
      }

    } else if (tp_it->subsystem() == TriggerPrimitive::kRPC) {

    }

  }  // end loop over muon_primitives

}

bool EMTFPrimitiveConversion::is_in_sector_csc(
    int tp_endcap, int tp_sector, int tp_subsector, int tp_station, int tp_csc_ID,
    int& is_neighbor
) const {
  static const std::vector<int> neighboring = {
    5, 0, 1, 2, 3, 4,
    11, 6, 7, 8, 9, 10
  };

  tp_sector = (tp_endcap - 1) * 6 + (tp_sector - 1);
  is_neighbor = false;

  // Match sector
  if (sector_ == tp_sector)
    return true;

  // Match neighbor sector
  if (includeNeighbor_) {
    if (neighboring.at(sector_) == tp_sector) {
      if (tp_station == 1) {
        if ((tp_subsector == 2) && (tp_csc_ID == 3 || tp_csc_ID == 6 || tp_csc_ID == 9))
          is_neighbor = true;

      } else {
        if (tp_csc_ID == 3 || tp_csc_ID == 9)
          is_neighbor = true;
      }

      if (is_neighbor)
        return true;
    }
  }

  return false;
}

bool EMTFPrimitiveConversion::is_in_bx_csc(int tp_bx) const {
  tp_bx -= 6;
  return (bx_ == tp_bx);
}

EMTFHitExtra EMTFPrimitiveConversion::make_emtf_hit(const TriggerPrimitive& muon_primitive, int is_neighbor) const {
  const CSCDetId tp_detId = muon_primitive.detId<CSCDetId>();
  const CSCData& tp_data  = muon_primitive.getCSCData();

  int tp_endcap    = tp_detId.endcap();
  int tp_sector    = tp_detId.triggerSector();
  int tp_station   = tp_detId.station();
  int tp_ring      = tp_detId.ring();
  int tp_chamber   = tp_detId.chamber();

  // station 1 --> subsector 1 or 2
  // station 2,3,4 --> subsector 0
  int tp_subsector = (tp_station != 1) ? 0 : ((tp_chamber%6 > 2) ? 1 : 2);

  EMTFHitData data;
  data.endcap     = tp_endcap;
  data.station    = tp_station;
  data.ring       = tp_ring;
  data.chamber    = tp_chamber;
  data.sector     = tp_sector;
  data.subsector  = tp_subsector;
  data.csc_ID     = tp_data.cscID;

  data.bx         = tp_data.bx;
  data.bx        -= 6;

  data.subsystem  = TriggerPrimitive::kCSC;

  data.neighbor   = is_neighbor;

  data.valid      = tp_data.valid;
  data.quality    = tp_data.quality;
  data.wire       = tp_data.keywire;
  data.strip      = tp_data.strip;
  data.pattern    = tp_data.pattern;

  data.bend       = tp_data.bend;

  EMTFHitExtra conv_hit;
  conv_hit.setData(data);
  return conv_hit;
}
