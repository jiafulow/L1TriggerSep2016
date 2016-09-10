#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPrimitiveConversion.hh"

#include "DataFormats/MuonDetId/interface/DTChamberId.h"
#include "DataFormats/MuonDetId/interface/CSCDetId.h"
#include "DataFormats/MuonDetId/interface/RPCDetId.h"

using CSCData = TriggerPrimitive::CSCData;
using RPCData = TriggerPrimitive::RPCData;
using EMTFHitData = EMTFHitExtra::EMTFHitData;


EMTFPrimitiveConversion::EMTFPrimitiveConversion() {

}

EMTFPrimitiveConversion::~EMTFPrimitiveConversion() {

}

// Specialized for CSC
template<>
EMTFHitExtra EMTFPrimitiveConversion::convert(
    CSCTag tag,
    int sector, bool is_neighbor,
    const TriggerPrimitive& muon_primitive
) {
  sector_      = sector;
  is_neighbor_ = is_neighbor;

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

// Specialized for CSC
template<>
EMTFHitExtra EMTFPrimitiveConversion::convert(
    RPCTag tag,
    int sector, bool is_neighbor,
    const TriggerPrimitive& muon_primitive
) {
  sector_      = sector;
  is_neighbor_ = is_neighbor;

  //const RPCDetId tp_detId = muon_primitive.detId<RPCDetId>();
  //const RPCData& tp_data  = muon_primitive.getRPCData();

  //FIXME: implement this

  EMTFHitExtra conv_hit;
  //conv_hit.setData(data);
  return conv_hit;
}
