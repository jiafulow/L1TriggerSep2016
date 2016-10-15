#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPrimitiveConversion.hh"

#include "DataFormats/MuonDetId/interface/DTChamberId.h"
#include "DataFormats/MuonDetId/interface/CSCDetId.h"
#include "DataFormats/MuonDetId/interface/RPCDetId.h"

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFSectorProcessorLUT.hh"

using CSCData = TriggerPrimitive::CSCData;
using RPCData = TriggerPrimitive::RPCData;


void EMTFPrimitiveConversion::configure(
    const EMTFSectorProcessorLUT* lut,
    int verbose, int endcap, int sector, int bx,
    int bxShiftCSC,
    bool duplicateTheta, bool fixZonePhi,
    const std::vector<int>& zoneBoundaries1, const std::vector<int>& zoneBoundaries2, int zoneOverlap
) {
  lut_ = lut;

  verbose_ = verbose;
  endcap_  = endcap;
  sector_  = sector;
  bx_      = bx;

  bxShiftCSC_      = bxShiftCSC;

  duplicateTheta_  = duplicateTheta;
  fixZonePhi_      = fixZonePhi;

  zoneBoundaries1_ = zoneBoundaries1;  // not used, can be removed
  zoneBoundaries2_ = zoneBoundaries2;
  zoneOverlap_     = zoneOverlap;
}


// Specialized for CSC
template<>
void EMTFPrimitiveConversion::process(
    CSCTag tag,
    const std::map<int, TriggerPrimitiveCollection>& selected_csc_map,
    EMTFHitExtraCollection& conv_hits
) const {
  std::map<int, TriggerPrimitiveCollection>::const_iterator map_tp_it  = selected_csc_map.begin();
  std::map<int, TriggerPrimitiveCollection>::const_iterator map_tp_end = selected_csc_map.end();

  for (; map_tp_it != map_tp_end; ++map_tp_it) {
    int selected   = map_tp_it->first;
    int pc_sector  = sector_;
    int pc_station = selected / 9;
    int pc_chamber = selected % 9;
    int pc_segment = 0;
    TriggerPrimitiveCollection::const_iterator tp_it  = map_tp_it->second.begin();
    TriggerPrimitiveCollection::const_iterator tp_end = map_tp_it->second.end();

    for (; tp_it != tp_end; ++tp_it) {
      EMTFHitExtra conv_hit;
      convert_csc(pc_sector, pc_station, pc_chamber, pc_segment, *tp_it, conv_hit);  // CSC
      conv_hits.push_back(conv_hit);
      pc_segment += 1;
    }
  }
}

// Specialized for RPC
template<>
void EMTFPrimitiveConversion::process(
    RPCTag tag,
    const std::map<int, TriggerPrimitiveCollection>& selected_rpc_map,
    EMTFHitExtraCollection& conv_hits
) const {
  std::map<int, TriggerPrimitiveCollection>::const_iterator map_tp_it  = selected_rpc_map.begin();
  std::map<int, TriggerPrimitiveCollection>::const_iterator map_tp_end = selected_rpc_map.end();

  for (; map_tp_it != map_tp_end; ++map_tp_it) {
    int selected   = map_tp_it->first;
    int pc_sector  = sector_;
    int pc_station = selected / 9;
    int pc_chamber = selected % 9;
    int pc_segment = 0;
    TriggerPrimitiveCollection::const_iterator tp_it  = map_tp_it->second.begin();
    TriggerPrimitiveCollection::const_iterator tp_end = map_tp_it->second.end();

    for (; tp_it != tp_end; ++tp_it) {
      EMTFHitExtra conv_hit;
      convert_rpc(pc_sector, pc_station, pc_chamber, pc_segment, *tp_it, conv_hit);  // RPC
      conv_hits.push_back(conv_hit);
      pc_segment += 1;
    }
  }
}

const EMTFSectorProcessorLUT& EMTFPrimitiveConversion::lut() const {
  assert(lut_ != nullptr);
  return *lut_;
}

// CSC functions
void EMTFPrimitiveConversion::convert_csc(
    int pc_sector, int pc_station, int pc_chamber, int pc_segment,
    const TriggerPrimitive& muon_primitive,
    EMTFHitExtra& conv_hit
) const {
  const CSCDetId tp_detId = muon_primitive.detId<CSCDetId>();
  const CSCData& tp_data  = muon_primitive.getCSCData();

  int tp_endcap    = tp_detId.endcap();
  int tp_sector    = tp_detId.triggerSector();
  int tp_station   = tp_detId.station();
  int tp_ring      = tp_detId.ring();
  int tp_chamber   = tp_detId.chamber();

  int tp_bx        = tp_data.bx;
  int tp_csc_ID    = tp_data.cscID;

  // station 1 --> subsector 1 or 2
  // station 2,3,4 --> subsector 0
  int tp_subsector = (tp_station != 1) ? 0 : ((tp_chamber%6 > 2) ? 1 : 2);

  // Check using ME1/1a --> ring 4 convention
  if (tp_station == 1 && tp_ring == 1) {
    assert(tp_data.strip < 128);
    assert(1 <= tp_csc_ID && tp_csc_ID <= 3);
  }
  if (tp_station == 1 && tp_ring == 4) {
    assert(tp_data.strip < 128);
    assert(1 <= tp_csc_ID && tp_csc_ID <= 3);
  }

  // Check input data
  assert(tp_data.strip < 160);
  assert(tp_data.keywire < 112);
  assert(tp_data.valid == true);
  assert(tp_data.pattern <= 10);
  assert(tp_data.quality > 0);

  bool is_neighbor = (pc_station == 5);

  int cscn_ID      = tp_csc_ID;  // modify csc_ID if coming from neighbor sector
  if (is_neighbor) {
    // station 1 has 3 neighbor chambers: 13,14,15 in rings 1,2,3
    // (where are chambers 10,11,12 in station 1? they used to be for ME1/1a, but not anymore)
    // station 2,3,4 have 2 neighbor chambers: 10,11 in rings 1,2
    cscn_ID = (pc_chamber < 3) ? (pc_chamber + 12) : (((pc_chamber-1)%2) + 9);
    cscn_ID += 1;

    if (tp_station == 1) {  // neighbor ME1
      assert(tp_subsector == 2);
    }
  }

  // Set properties
  conv_hit.endcap      = tp_endcap;
  conv_hit.station     = tp_station;
  conv_hit.ring        = tp_ring;
  conv_hit.chamber     = tp_chamber;
  conv_hit.sector      = tp_sector;
  conv_hit.subsector   = tp_subsector;
  conv_hit.csc_ID      = tp_csc_ID;
  conv_hit.cscn_ID     = cscn_ID;

  conv_hit.bx          = tp_bx + bxShiftCSC_;
  conv_hit.subsystem   = TriggerPrimitive::kCSC;

  conv_hit.pc_sector   = pc_sector;
  conv_hit.pc_station  = pc_station;
  conv_hit.pc_chamber  = pc_chamber;
  conv_hit.pc_segment  = pc_segment;

  conv_hit.valid       = tp_data.valid;
  conv_hit.strip       = tp_data.strip;
  conv_hit.wire        = tp_data.keywire;
  conv_hit.quality     = tp_data.quality;
  conv_hit.pattern     = tp_data.pattern;
  conv_hit.bend        = tp_data.bend;

  conv_hit.bc0         = 0;
  conv_hit.mpc_link    = tp_data.mpclink;
  conv_hit.sync_err    = tp_data.syncErr;
  conv_hit.track_num   = tp_data.trknmb;
  conv_hit.stub_num    = 0;
  conv_hit.bx0         = tp_data.bx0;
  conv_hit.layer       = 0;

  convert_csc_details(conv_hit);
}

void EMTFPrimitiveConversion::convert_csc_details(EMTFHitExtra& conv_hit) const {
  bool is_neighbor = (conv_hit.pc_station == 5);

  // Defined as in firmware
  // endcap : 0-1 for ME+,ME-
  // sector : 0-5
  // station: 0-4 for st1 sub1, st1 sub2, st2, st3, st4
  // cscid  : 0-14 including neighbors
  int fw_endcap  = (endcap_-1);
  int fw_sector  = (sector_-1);
  int fw_station = (conv_hit.station == 1) ? (is_neighbor ? 0 : (conv_hit.subsector-1)) : conv_hit.station;
  int fw_cscid   = (conv_hit.cscn_ID-1);
  int fw_hstrip  = conv_hit.strip;  // it is half-strip, despite the name
  int fw_wg      = conv_hit.wire;   // it is wiregroup, despite the name

  // Primitive converter unit
  // station: 0-5 for st1 sub1, st1 sub2, st2, st3, st4, neigh all st*
  // chamber: 0-8
  int pc_station = conv_hit.pc_station;
  int pc_chamber = conv_hit.pc_chamber;

  bool is_me11a = (conv_hit.station == 1 && conv_hit.ring == 4);
  bool is_me11b = (conv_hit.station == 1 && conv_hit.ring == 1);
  bool is_me13  = (conv_hit.station == 1 && conv_hit.ring == 3);

  // Is this chamber mounted in reverse direction?
  bool ph_reverse = false;
  if ((fw_endcap == 0 && fw_station >= 3) || (fw_endcap == 1 && fw_station < 3))
    ph_reverse = true;

  // Chamber coverage if phi_reverse = true
  int ph_coverage = 0;
  if (ph_reverse) {
    if (fw_station <= 1 && ((fw_cscid >= 6 && fw_cscid <= 8) || fw_cscid == 14))  // ME1/3
      ph_coverage = 15;
    else if (fw_station >= 2 && (fw_cscid <= 2 || fw_cscid == 9))  // ME2,3,4/1
      ph_coverage = 40;
    else  // all others
      ph_coverage = 20;
  }

  // Is this 10-deg or 20-deg chamber?
  bool is_10degree = false;
  if (
      (fw_station <= 1) || // ME1
      (fw_station >= 2 && ((fw_cscid >= 3 && fw_cscid <= 8) || fw_cscid == 10))  // ME2,3,4/2
  ) {
    is_10degree = true;
  }

  // LUT index
  // There are 54 CSC chambers including the neighbors in a sector, but 61 LUT indices
  // This comes from dividing the 6 chambers + 1 neighbor in ME1/1 into ME1/1a and ME1/1b
  int pc_lut_id = pc_chamber;
  if (pc_station == 0) {         // ME1 sub 1: 0 - 11
    pc_lut_id = is_me11a ? pc_lut_id + 9 : pc_lut_id;
  } else if (pc_station == 1) {  // ME1 sub 2: 16 - 27
    pc_lut_id += 16;
    pc_lut_id = is_me11a ? pc_lut_id + 9 : pc_lut_id;
  } else if (pc_station == 2) {  // ME2: 28 - 36
    pc_lut_id += 28;
  } else if (pc_station == 3) {  // ME3: 39 - 47
    pc_lut_id += 39;
  } else if (pc_station == 4) {  // ME4 : 50 - 58
    pc_lut_id += 50;
  } else if (pc_station == 5 && pc_chamber < 3) {  // neighbor ME1: 12 - 15
    pc_lut_id = is_me11a ? pc_lut_id + 15 : pc_lut_id + 12;
  } else if (pc_station == 5 && pc_chamber < 5) {  // neighbor ME2: 37 - 38
    pc_lut_id += 28 + 9 - 3;
  } else if (pc_station == 5 && pc_chamber < 7) {  // neighbor ME3: 48 - 49
    pc_lut_id += 39 + 9 - 5;
  } else if (pc_station == 5 && pc_chamber < 9) {  // neighbor ME4: 59 - 60
    pc_lut_id += 50 + 9 - 7;
  }
  assert(pc_lut_id < 61);

  if (verbose_ > 1) {  // debug
    std::cout << "pc_station: " << pc_station << " pc_chamber: " << pc_chamber
        << " fw_station: " << fw_station << " fw_cscid: " << fw_cscid
        << " lut_id: " << pc_lut_id
        << " ph_init: " << lut().get_ph_init(fw_endcap, fw_sector, pc_lut_id)
        << " ph_disp: " << lut().get_ph_disp(fw_endcap, fw_sector, pc_lut_id)
        << " th_init: " << lut().get_th_init(fw_endcap, fw_sector, pc_lut_id)
        << " th_disp: " << lut().get_th_disp(fw_endcap, fw_sector, pc_lut_id)
        << " ph_init_hard: " << lut().get_ph_init_hard(fw_station, fw_cscid)
        << std::endl;
  }

  // ___________________________________________________________________________
  // ph conversion

  // Convert half-strip into 1/8-strip
  int eighth_strip = 0;

  // Apply phi correction from CLCT pattern number
  int clct_pat_corr = lut().get_ph_patt_corr(conv_hit.pattern);
  int clct_pat_corr_sign = (lut().get_ph_patt_corr_sign(conv_hit.pattern) == 0) ? 1 : -1;

  if (is_10degree) {
    eighth_strip = fw_hstrip << 2;  // full precision, uses only 2 bits of pattern correction
    eighth_strip += clct_pat_corr_sign * (clct_pat_corr >> 1);
  } else {
    eighth_strip = fw_hstrip << 3;  // full precision, uses all 3 bits of pattern correction
    eighth_strip += clct_pat_corr_sign * (clct_pat_corr >> 0);
  }

  // Multiplicative factor for strip
  int factor = 1024;
  if (is_me11a)
    factor = 1707;  // ME1/1a
  else if (is_me11b)
    factor = 1301;  // ME1/1b
  else if (is_me13)
    factor = 947;   // ME1/3

  // full-precision phi, but local to chamber (counted from strp 1)
  // zone phi precision: 0.53333 deg
  // full phi precision: 0.01666 deg
  int ph_tmp = (eighth_strip * factor) >> 10;
  int ph_tmp_sign = (ph_reverse == 0) ? 1 : -1;

  int fph = lut().get_ph_init(fw_endcap, fw_sector, pc_lut_id);
  fph = fph + ph_tmp_sign * ph_tmp;

  int ph_hit = lut().get_ph_disp(fw_endcap, fw_sector, pc_lut_id);
  ph_hit = (ph_hit >> 1) + ph_tmp_sign * (ph_tmp >> 5) + ph_coverage;

  // Full phi +16 to put the rounded value into the middle of error range
  // Divide full phi by 32, subtract chamber start
  int ph_hit_fixed = -1 * lut().get_ph_init_hard(fw_station, fw_cscid);
  ph_hit_fixed = ph_hit_fixed + ((fph + 16) >> 5);

  if (fixZonePhi_)
    ph_hit = ph_hit_fixed;

  // Zone phi
  int zone_hit = lut().get_ph_zone_offset(pc_station, pc_chamber);
  zone_hit += ph_hit;

  int zone_hit_fixed = lut().get_ph_init_hard(fw_station, fw_cscid);
  zone_hit_fixed += ph_hit_fixed;
  // Since ph_hit_fixed = ((fph + 16) >> 5) - lut().get_ph_init_hard(), the following is equivalent:
  //zone_hit_fixed = ((fph + 16) >> 5);

  if (fixZonePhi_)
    zone_hit = zone_hit_fixed;

  // ___________________________________________________________________________
  // th conversion

  int pc_wire_id = (fw_wg & 0x7f);
  int th_orig = lut().get_th_lut(fw_endcap, fw_sector, pc_lut_id, pc_wire_id);

  int th_tmp = th_orig;

  if (is_me11a || is_me11b) {
    int pc_wire_strip_id = (((fw_wg >> 4) & 0x3) << 5) | ((eighth_strip >> 4) & 0x1f);
    int th_corr = lut().get_th_corr_lut(fw_endcap, fw_sector, pc_lut_id, pc_wire_strip_id);
    int th_corr_sign = (ph_reverse == 0) ? 1 : -1;

    th_tmp = th_tmp + th_corr_sign * th_corr;

    // Check that correction did not make invalid value outside chamber coverage
    const int th_negative = 50;
    const int th_coverage = 45;

    if (th_tmp > th_negative || fw_wg == 0)
      th_tmp = 0;  // limit at the bottom
    if (th_tmp > th_coverage)
      th_tmp = th_coverage;  // limit at the top
  }

  // theta precision: 0.285 degree
  int th = lut().get_th_init(fw_endcap, fw_sector, pc_lut_id);
  th = th + th_tmp;

  // Protect against invalid value
  if (th == 0)
    th = 1;

  // ___________________________________________________________________________
  // zones

  // ph zone boundaries for chambers that cover more than one zone
  // bnd1 is the lower boundary, bnd2 the upper boundary
  int ph_zone_bnd1 = zoneBoundaries2_.at(3);  // = 127
  if (fw_station <= 1 && (fw_cscid <= 2 || fw_cscid == 12))  // ME1/1
    ph_zone_bnd1 = zoneBoundaries2_.at(0);  // = 41
  else if (fw_station == 2 && (fw_cscid <= 2 || fw_cscid == 9))  // ME2/1
    ph_zone_bnd1 = zoneBoundaries2_.at(0);  // = 41
  else if (fw_station == 2 && ((fw_cscid >= 3 && fw_cscid <= 8) || fw_cscid == 10))  // ME2/2
    ph_zone_bnd1 = zoneBoundaries2_.at(2);  // = 87
  else if (fw_station == 3 && ((fw_cscid >= 3 && fw_cscid <= 8) || fw_cscid == 10))  // ME3/2
    ph_zone_bnd1 = zoneBoundaries2_.at(1);  // = 49
  else if (fw_station == 4 && ((fw_cscid >= 3 && fw_cscid <= 8) || fw_cscid == 10))  // ME4/2
    ph_zone_bnd1 = zoneBoundaries2_.at(1);  // = 49

  int ph_zone_bnd2 = zoneBoundaries2_.at(3);  // = 127
  if (fw_station == 3 && ((fw_cscid >= 3 && fw_cscid <= 8) || fw_cscid == 10))  // ME3/2
    ph_zone_bnd2 = zoneBoundaries2_.at(2);  // = 87

  bool new_zones_AWB = false;
  int ph_zone_bnd1_AWB = ph_zone_bnd1;
  if (fw_station == 3 && (fw_cscid <= 2 || fw_cscid == 9))  // ME3/1
    ph_zone_bnd1_AWB = zoneBoundaries2_.at(0);  // = 36
  else if (fw_station <= 1 && ((fw_cscid >= 3 && fw_cscid <= 5) || fw_cscid == 13))  // ME1/2
    ph_zone_bnd1_AWB = zoneBoundaries2_.at(1);  // = 54
  else if (fw_station == 2 && ((fw_cscid >= 3 && fw_cscid <= 8) || fw_cscid == 10))  // ME2/2
    ph_zone_bnd1_AWB = zoneBoundaries2_.at(1);  // = 54

  int ph_zone_bnd2_AWB = ph_zone_bnd2;
  if (fw_station == 2 && ((fw_cscid >= 3 && fw_cscid <= 8) || fw_cscid == 10))  // ME2/2
    ph_zone_bnd2_AWB = zoneBoundaries2_.at(2);  // = 96

  if (new_zones_AWB) {
    ph_zone_bnd1 = ph_zone_bnd1_AWB;
    ph_zone_bnd2 = ph_zone_bnd2_AWB;
  }

  int zone_overlap = zoneOverlap_;

  // Check which zones ph hits should be applied to
  int phzvl = 0;
  if (th <= (ph_zone_bnd1 + zone_overlap)) {
    phzvl |= (1<<0);
  }
  if (th >  (ph_zone_bnd2 - zone_overlap)) {
    phzvl |= (1<<2);
  }
  if (
      (th >  (ph_zone_bnd1 - zone_overlap)) &&
      (th <= (ph_zone_bnd2 + zone_overlap))
  ) {
    phzvl |= (1<<1);
  }

  int zone_code = 0;
  if (fw_station <= 1) {  // station 1
    if (fw_cscid <= 2 || fw_cscid == 12) {  // ring 1
      if (phzvl & (1<<0))
        zone_code |= (1<<0);  // zone 0: [-,41+2]
      if (phzvl & (1<<1))
        zone_code |= (1<<1);  // zone 1: [41-1,127+2]

    } else if (fw_cscid <= 5 || fw_cscid == 13) {  // ring 2
      if (phzvl & (1<<0))
        zone_code |= (1<<2);  // zone 2: [-,127+2]

    } else if (fw_cscid <= 8 || fw_cscid == 14) {  // ring 3
      if (true)  // ME1/3 does not need phzvl
        zone_code |= (1<<3);  // zone 3: [-,-]
    }

  } else if (fw_station == 2) {  // station 2
    if (fw_cscid <= 2 || fw_cscid == 9) {  // ring 1
      if (phzvl & (1<<0))
        zone_code |= (1<<0);  // zone 0: [-,41+2]
      if (phzvl & (1<<1))
        zone_code |= (1<<1);  // zone 1: [41-1,127+2]

    } else if (fw_cscid <= 8 || fw_cscid == 10) {  // ring 2
      if (phzvl & (1<<0))
        zone_code |= (1<<2);  // zone 2: [-,87+2]
      if (phzvl & (1<<1))
        zone_code |= (1<<3);  // zone 3: [87-1,127+2]
    }

  } else if (fw_station == 3) {  // station 3
    if (fw_cscid <= 2 || fw_cscid == 9) {  // ring 1
      if (phzvl & (1<<0))
        zone_code |= (1<<0);  // zone 0: [-,127+2]

    } else if (fw_cscid <= 8 || fw_cscid == 10) {  // ring 2
      if (phzvl & (1<<0))
        zone_code |= (1<<1);  // zone 1: [-,49+2]
      if (phzvl & (1<<1))
        zone_code |= (1<<2);  // zone 2: [49-1,87+2]
      if (phzvl & (1<<2))
        zone_code |= (1<<3);  // zone 3: [87-1,-]
    }

  } else if (fw_station == 4) {  // station 4
    if (fw_cscid <= 2 || fw_cscid == 9) {  // ring 1
      if (phzvl & (1<<0))
        zone_code |= (1<<0);  // zone 0: [-,127+2]

    } else if (fw_cscid <= 8 || fw_cscid == 10) {  // ring 2
      if (phzvl & (1<<0))
        zone_code |= (1<<1);  // zone 1: [-,49+2]
      if (phzvl & (1<<1))
        zone_code |= (1<<2);  // zone 2: [49-1,127+2]
    }
  }
  assert(zone_code > 0);

  int zone_code_AWB = 0;
  if (fw_station <= 1) {  // station 1
    if (fw_cscid <= 2 || fw_cscid == 12) {  // ring 1
      if (phzvl & (1<<0))
        zone_code_AWB |= (1<<0);  // zone 0: [-,36+2]
      if (phzvl & (1<<1))
        zone_code_AWB |= (1<<1);  // zone 1: [36-1,127+2]

    } else if (fw_cscid <= 5 || fw_cscid == 13) {  // ring 2
      if (phzvl & (1<<0))
        zone_code_AWB |= (1<<1);  // zone 1: [-,54+2]
      if (phzvl & (1<<1))
        zone_code_AWB |= (1<<2);  // zone 2: [54-1,127+2]

    } else if (fw_cscid <= 8 || fw_cscid == 14) {  // ring 3
      if (true)  // ME1/3 does not need phzvl
        zone_code_AWB |= (1<<3);  // zone 3: [-,-]
    }

  } else if (fw_station == 2) {  // station 2
    if (fw_cscid <= 2 || fw_cscid == 9) {  // ring 1
      if (phzvl & (1<<0))
        zone_code_AWB |= (1<<0);  // zone 0: [-,36+2]
      if (phzvl & (1<<1))
        zone_code_AWB |= (1<<1);  // zone 1: [36-1,127+2]

    } else if (fw_cscid <= 8 || fw_cscid == 10) {  // ring 2
      if (phzvl & (1<<0))
        zone_code_AWB |= (1<<1);  // zone 1: [-,54+2]
      if (phzvl & (1<<1))
        zone_code_AWB |= (1<<2);  // zone 2: [54-1,96+2]
      if (phzvl & (1<<2))
        zone_code_AWB |= (1<<3);  // zone 3: [96-1,127+2]
    }

  } else if (fw_station == 3) {  // station 3
    if (fw_cscid <= 2 || fw_cscid == 9) {  // ring 1
      if (phzvl & (1<<0))
        zone_code_AWB |= (1<<0);  // zone 0: [-,36+2]
      if (phzvl & (1<<1))
        zone_code_AWB |= (1<<1);  // zone 1: [36-1,127+2]

    } else if (fw_cscid <= 8 || fw_cscid == 10) {  // ring 2
      if (phzvl & (1<<0))
        zone_code_AWB |= (1<<1);  // zone 1: [-,54+2]
      if (phzvl & (1<<1))
        zone_code_AWB |= (1<<2);  // zone 2: [54-1,96+2]
      if (phzvl & (1<<2))
        zone_code_AWB |= (1<<3);  // zone 3: [96-1,127+2]
    }

  } else if (fw_station == 4) {  // station 4
    if (fw_cscid <= 2 || fw_cscid == 9) {  // ring 1
      if (phzvl & (1<<0))
        zone_code_AWB |= (1<<0);  // zone 0: [-,127+2]

    } else if (fw_cscid <= 8 || fw_cscid == 10) {  // ring 2
      if (phzvl & (1<<0))
        zone_code_AWB |= (1<<1);  // zone 1: [-,54+2]
      if (phzvl & (1<<1))
        zone_code_AWB |= (1<<2);  // zone 2: [54-1,127+2]
    }
  }
  assert(zone_code_AWB > 0);

  if (new_zones_AWB)
    zone_code = zone_code_AWB;


  // ___________________________________________________________________________
  // Output

  conv_hit.phi_fp     = fph;
  conv_hit.theta_fp   = th;
  conv_hit.phzvl      = phzvl;
  conv_hit.ph_hit     = ph_hit;
  conv_hit.zone_hit   = zone_hit;
  conv_hit.zone_code  = zone_code;
}

// RPC functions
void EMTFPrimitiveConversion::convert_rpc(
    int pc_sector, int pc_station, int pc_chamber, int pc_segment,
    const TriggerPrimitive& muon_primitive,
    EMTFHitExtra& conv_hit
) const {
  //const RPCDetId tp_detId = muon_primitive.detId<RPCDetId>();
  //const RPCData& tp_data  = muon_primitive.getRPCData();

  convert_rpc_details(conv_hit);
}

void EMTFPrimitiveConversion::convert_rpc_details(EMTFHitExtra& conv_hit) const {

}
