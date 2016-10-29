#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPrimitiveConversion.hh"

#include "DataFormats/MuonDetId/interface/DTChamberId.h"
#include "DataFormats/MuonDetId/interface/CSCDetId.h"
#include "DataFormats/MuonDetId/interface/RPCDetId.h"

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFSectorProcessorLUT.hh"

#define NUM_CSC_CHAMBERS 6*9   // 18 in ME1, 9 in ME2/3/4, 9 from neighbor sector
#define NUM_RPC_CHAMBERS 6*21  // 18+3 neighbor in stations 1/2, 36+6 neighbor in 3/4

using CSCData = TriggerPrimitive::CSCData;
using RPCData = TriggerPrimitive::RPCData;


void EMTFPrimitiveConversion::configure(
    const EMTFSectorProcessorLUT* lut,
    int verbose, int endcap, int sector, int bx,
    int bxShiftCSC, int bxShiftRPC,
    const std::vector<int>& zoneBoundaries, int zoneOverlap, bool duplicateTheta, bool fixZonePhi, bool useNewZones
) {
  assert(lut != nullptr);

  lut_      = lut;

  verbose_ = verbose;
  endcap_  = endcap;
  sector_  = sector;
  bx_      = bx;

  bxShiftCSC_      = bxShiftCSC;
  bxShiftRPC_      = bxShiftRPC;

  zoneBoundaries_  = zoneBoundaries;
  zoneOverlap_     = zoneOverlap;
  duplicateTheta_  = duplicateTheta;
  fixZonePhi_      = fixZonePhi;
  useNewZones_     = useNewZones;
}


// Specialized for CSC
template<>
void EMTFPrimitiveConversion::process(
    CSCTag tag,
    const std::map<int, TriggerPrimitiveCollection>& selected_csc_map,
    EMTFHitExtraCollection& conv_hits,
    const std::unique_ptr<L1TMuonEndCap::GeometryTranslator>& tp_geom
) const {
  std::map<int, TriggerPrimitiveCollection>::const_iterator map_tp_it  = selected_csc_map.begin();
  std::map<int, TriggerPrimitiveCollection>::const_iterator map_tp_end = selected_csc_map.end();

  for (; map_tp_it != map_tp_end; ++map_tp_it) {
    // Unique chamber ID in FW, {0, 53} as defined in get_index_csc in src/EMTFPrimitiveSelection.cc
    int selected   = map_tp_it->first; 
    // "Primitive Conversion" sector/station/chamber ID scheme used in FW
    int pc_sector  = sector_;
    int pc_station = selected / 9;  // {0, 5} = {ME1 sub 1, ME1 sub 2, ME2, ME3, ME4, neighbor}
    int pc_chamber = selected % 9;  // Equals CSC ID - 1 for all except neighbor chambers
    int pc_segment = 0;             // Counts hits in a single chamber
    TriggerPrimitiveCollection::const_iterator tp_it  = map_tp_it->second.begin();
    TriggerPrimitiveCollection::const_iterator tp_end = map_tp_it->second.end();

    for (; tp_it != tp_end; ++tp_it) {
      EMTFHitExtra conv_hit;
      convert_csc(pc_sector, pc_station, pc_chamber, pc_segment, *tp_it, conv_hit);
      conv_hits.push_back(conv_hit);
      pc_segment += 1;
    }

    assert(pc_segment <= 4);  // With 2 unique LCTs, 4 possible strip/wire combinations
  }
}

// Specialized for RPC
template<>
void EMTFPrimitiveConversion::process(
    RPCTag tag,
    const std::map<int, TriggerPrimitiveCollection>& selected_rpc_map,
    EMTFHitExtraCollection& conv_hits,
    const std::unique_ptr<L1TMuonEndCap::GeometryTranslator>& tp_geom
) const {

  std::map<int, TriggerPrimitiveCollection>::const_iterator map_tp_it  = selected_rpc_map.begin();
  std::map<int, TriggerPrimitiveCollection>::const_iterator map_tp_end = selected_rpc_map.end();

  // Find all fired strips
  bool strips_with_hits[NUM_RPC_CHAMBERS][32] = {{false}};
  for (; map_tp_it != map_tp_end; ++map_tp_it) {
    int selected   = map_tp_it->first;
    TriggerPrimitiveCollection::const_iterator tp_it  = map_tp_it->second.begin();
    TriggerPrimitiveCollection::const_iterator tp_end = map_tp_it->second.end();

    for (; tp_it != tp_end; ++tp_it) {
      int iStrip = tp_it->getRPCData().strip - 1;
      strips_with_hits[selected][iStrip] = true;
    }
  }

  // Find all contiguous clusters of strips in each station/ring/roll/subsector
  std::map<int, std::vector<std::pair<uint, uint>>> cluster_map;
  for (uint selected = 0; selected < NUM_RPC_CHAMBERS; selected ++) {
    for (uint iStrip = 0; iStrip < 32; iStrip++) {
      if (strips_with_hits[selected][iStrip]) {
	if (iStrip > 0 && strips_with_hits[selected][iStrip - 1])
	  cluster_map[selected].back().second = iStrip + 1;
	else if (cluster_map[selected].size() < 2)
	  cluster_map[selected].push_back(std::make_pair(iStrip + 1, iStrip + 1));
	// else
	//   std::cout << "Selected RPC " << selected << " already has 2 clusters.  Can't add more." << std::endl;
      }
    }
  }

  // Create converted hits from clusters
  map_tp_it  = selected_rpc_map.begin();
  map_tp_end = selected_rpc_map.end();
  for (; map_tp_it != map_tp_end; ++map_tp_it) {
    int selected   = map_tp_it->first;
    int pc_sector  = sector_;
    int pc_station = (selected > 17 || (selected % 6) > 2) + (selected > 17) + (selected > 35) + 
                     (selected > 71) + (selected > 107);  // {0, 5} = {RPC1 left, RPC1 right, RPC2, RPC3, RPC4, neighbor}
    int pc_chamber = (selected < 36 ? (selected % 18) : (selected % 36));  // Unique identifier per station
    int pc_segment = 0;

    for (uint iClust = 0; iClust < cluster_map[selected].size(); iClust++) {
      
      TriggerPrimitiveCollection::const_iterator tp1_it  = map_tp_it->second.begin();
      TriggerPrimitiveCollection::const_iterator tp1_end = map_tp_it->second.end();
      TriggerPrimitiveCollection::const_iterator tp2_it  = map_tp_it->second.begin();
      TriggerPrimitiveCollection::const_iterator tp2_end = map_tp_it->second.end();
      
      for (; tp1_it != tp1_end; ++tp1_it) {
	if (tp1_it->getRPCData().strip != cluster_map[selected].at(iClust).first)
	  continue;
	for (; tp2_it != tp2_end; ++tp2_it) {
	  if (tp2_it->getRPCData().strip != cluster_map[selected].at(iClust).second)
	    continue;
	  
	  EMTFHitExtra conv_hit;
	  convert_rpc(pc_sector, pc_station, pc_chamber, pc_segment, *tp1_it, *tp2_it, conv_hit, tp_geom);  // RPC
	  conv_hits.push_back(conv_hit);
	  pc_segment += 1;
	} // End loop over tp2_it
      } // End loop over tp1_it
    } // End loop over iClust
  } // End loop over map_tp_it

} // End EMTFPrimitiveConversion::process

const EMTFSectorProcessorLUT& EMTFPrimitiveConversion::lut() const {
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
  int tp_subsector = (tp_station != 1) ? 0 : ((tp_chamber % 6 > 2) ? 1 : 2);

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

  const bool is_neighbor = (pc_station == 5);

  int cscn_ID      = tp_csc_ID;  // modify csc_ID if coming from neighbor sector
  if (is_neighbor) {
    // station 1 has 3 neighbor chambers: 13,14,15 in rings 1,2,3
    // (where are chambers 10,11,12 in station 1? they used to be for ME1/1a, but not anymore)
    // station 2,3,4 have 2 neighbor chambers: 10,11 in rings 1,2
    cscn_ID = (pc_chamber < 3) ? (pc_chamber + 12) : ( ((pc_chamber - 1) % 2) + 9);
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

  conv_hit.bc0         = 0; // Not used anywhere, but part of EMTF DAQ output
  conv_hit.mpc_link    = tp_data.mpclink; // Used? Delete from class? - AWB 29.09.16
  conv_hit.sync_err    = tp_data.syncErr; // Used? Delete from class? - AWB 29.09.16
  conv_hit.track_num   = tp_data.trknmb; // Used? Delete from class? - AWB 29.09.16
  conv_hit.stub_num    = 0; // Should define in same way as firmware - AWB 29.09.16
  conv_hit.bx0         = tp_data.bx0; // Used? Delete from class? - AWB 29.09.16
  conv_hit.layer       = 0; // Used? Delete from class? - AWB 29.09.16

  convert_csc_details(conv_hit);
}

void EMTFPrimitiveConversion::convert_csc_details(EMTFHitExtra& conv_hit) const {
  const bool is_neighbor = (conv_hit.pc_station == 5);

  // Defined as in firmware
  // endcap : 0-1 for ME+,ME-
  // sector : 0-5
  // station: 0-4 for st1 sub1 or st1 sub2 from neighbor, st1 sub2, st2, st3, st4
  // cscid  : 0-14 (excluding 11), including neighbors
  const int fw_endcap  = (endcap_-1);
  const int fw_sector  = (sector_-1);
  const int fw_station = (conv_hit.station == 1) ? (is_neighbor ? 0 : (conv_hit.subsector-1)) : conv_hit.station;
  const int fw_cscid   = (conv_hit.cscn_ID-1);
  const int fw_hstrip  = conv_hit.strip;  // it is half-strip, despite the name
  const int fw_wg      = conv_hit.wire;   // it is wiregroup, despite the name

  // Primitive converter unit
  // station: 0-5 for st1 sub1, st1 sub2, st2, st3, st4, neigh all st*
  // chamber: 0-8
  const int pc_station = conv_hit.pc_station;
  const int pc_chamber = conv_hit.pc_chamber;

  const bool is_me11a = (conv_hit.station == 1 && conv_hit.ring == 4);
  const bool is_me11b = (conv_hit.station == 1 && conv_hit.ring == 1);
  const bool is_me13  = (conv_hit.station == 1 && conv_hit.ring == 3);

  // Is this chamber mounted in reverse direction?  (i.e., phi vs. strip number is reversed)
  bool ph_reverse = false;
  if ((fw_endcap == 0 && fw_station >= 3) || (fw_endcap == 1 && fw_station < 3))
    ph_reverse = true;

  // Chamber coverage if phi_reverse = true
  int ph_coverage = 0; // Offset for coordinate conversion
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
  // phi conversion

  // Convert half-strip into 1/8-strip
  int eighth_strip = 0;

  // Apply phi correction from CLCT pattern number (from src/EMTFSectorProcessorLUT.cc)
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
  // zone_hit_fixed = ((fph + 16) >> 5);

  if (fixZonePhi_)
    zone_hit = zone_hit_fixed;

  // ___________________________________________________________________________
  // theta conversion

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

  // theta precision = 0.285 degrees, starts at 8.5 deg: {1, 127} <--> {8.785, 44.695}
  int th = lut().get_th_init(fw_endcap, fw_sector, pc_lut_id);
  th = th + th_tmp;

  // Protect against invalid value
  if (th == 0)
    th = 1;

  // ___________________________________________________________________________
  // zones

  static const unsigned int zone_code_table[4][3] = {  // map (station,ring) to zone_code
    {0b0011, 0b0100, 0b1000},  // st1 r1: [z0,z1], r2: [z2],      r3: [z3]
    {0b0011, 0b1100, 0b0000},  // st2 r1: [z0,z1], r2: [z2,z3]
    {0b0001, 0b1110, 0b0000},  // st3 r1: [z0],    r2: [z1,z2,z3]
    {0b0001, 0b0110, 0b0000}   // st4 r1: [z0],    r2: [z1,z2]
  };

  static const unsigned int zone_code_table_new[4][3] = {  // map (station,ring) to zone_code
    {0b0011, 0b0110, 0b1000},  // st1 r1: [z0,z1], r2: [z1,z2],   r3: [z3]
    {0b0011, 0b1110, 0b0000},  // st2 r1: [z0,z1], r2: [z1,z2,z3]
    {0b0011, 0b1110, 0b0000},  // st3 r1: [z0,z1], r2: [z1,z2,z3]
    {0b0001, 0b0110, 0b0000}   // st4 r1: [z0],    r2: [z1,z2]
  };

  struct {
    constexpr unsigned int operator()(int tp_station, int tp_ring, bool use_new_table) {
      unsigned int istation = (tp_station-1);
      unsigned int iring = (tp_ring == 4) ? 0 : (tp_ring-1);
      assert(istation < 4 && iring < 3);
      unsigned int zone_code = (use_new_table) ? zone_code_table_new[istation][iring] : zone_code_table[istation][iring];
      return zone_code;
    }
  } zone_code_func;

  // ph zone boundaries for chambers that cover more than one zone
  // bnd1 is the lower boundary, bnd2 the upper boundary
  int zone_code = 0;
  for (int izone = 0; izone < NUM_ZONES; ++izone) {
    int zone_code_tmp = zone_code_func(conv_hit.station, conv_hit.ring, useNewZones_);
    if (zone_code_tmp & (1<<izone)) {
      bool no_use_bnd1 = ((izone==0) || ((zone_code_tmp & (1<<(izone-1))) == 0) || is_me13);  // first possible zone for this hit
      bool no_use_bnd2 = (((zone_code_tmp & (1<<(izone+1))) == 0) || is_me13);  // last possible zone for this hit

      int ph_zone_bnd1 = no_use_bnd1 ? zoneBoundaries_.at(0) : zoneBoundaries_.at(izone);
      int ph_zone_bnd2 = no_use_bnd2 ? zoneBoundaries_.at(NUM_ZONES) : zoneBoundaries_.at(izone+1);
      int zone_overlap = zoneOverlap_;

      if ((th > (ph_zone_bnd1 - zone_overlap)) && (th <= (ph_zone_bnd2 + zone_overlap))) {
        zone_code |= (1<<izone);
      }
    }
  }
  assert(zone_code > 0);

  // For backward compatibility, no longer needed (only explicitly used in FW)
  // phzvl: each chamber overlaps with at most 3 zones, so this "local" zone word says
  // which of the possible zones contain the hit: 1 for lower, 2 for middle, 4 for upper
  int phzvl = 0;
  if (conv_hit.ring == 1 || conv_hit.ring == 4) {
    phzvl = (zone_code >> 0);
  } else if (conv_hit.ring == 2) {
    if (conv_hit.station == 3 || conv_hit.station == 4) {
      phzvl = (zone_code >> 1);
    } else if (conv_hit.station == 1 || conv_hit.station == 2) {
      phzvl = (zone_code >> 2);
    }
  } else if (conv_hit.ring == 3) {
    phzvl = (zone_code >> 3);
  }

  // ___________________________________________________________________________
  // For later use in primitive matching
  // (in the firmware, this happens in the find_segment module)

  int fs_history = 0;                       // history id: not set here, to be set in primitive matching
  int fs_chamber = -1;                      // chamber id
  int fs_segment = conv_hit.pc_segment % 2; // segment id
  int fs_zone_code = zone_code_func(conv_hit.station, conv_hit.ring, useNewZones_);

  // For ME1
  //   j = 0 is neighbor sector chamber
  //   j = 1,2,3 are native subsector 1 chambers
  //   j = 4,5,6 are native subsector 2 chambers
  // For ME2,3,4:
  //   j = 0 is neighbor sector chamber
  //   j = 1,2,3,4,5,6 are native sector chambers
  if (fw_station <= 1) {  // ME1
    int n = (conv_hit.csc_ID-1) % 3;
    fs_chamber = is_neighbor ? 0 : ((conv_hit.subsector == 1) ? 1+n : 4+n);
  } else {  // ME2,3,4
    int n = (conv_hit.ring == 1) ? (conv_hit.csc_ID-1) : (conv_hit.csc_ID-1-3);
    fs_chamber = is_neighbor ? 0 : 1+n;
  }

  assert(fs_history >= 0 && fs_chamber != -1 && fs_segment < 2);
  fs_segment = ((fs_history & 0x3)<<4) | ((fs_chamber & 0x7)<<1) | (fs_segment & 0x1);

  // ___________________________________________________________________________
  // Output

    conv_hit.phi_fp     = fph;        // Full-precision integer phi
    conv_hit.theta_fp   = th;         // Full-precision integer theta
    conv_hit.phzvl      = phzvl;      // Local zone word: (1*low) + (2*mid) + (4*low) - used in FW debugging
    conv_hit.ph_hit     = ph_hit;     // Intermediate quantity in phi calculation - used in FW debugging
    conv_hit.zone_hit   = zone_hit;   // Phi value for building patterns (0.53333 deg precision)
    conv_hit.zone_code  = zone_code;  // Full zone word: 1*(zone 0) + 2*(zone 1) + 4*(zone 2) + 8*(zone 3)

    conv_hit.fs_segment   = fs_segment;    // How is this used? - AWB 18.10.16
    conv_hit.fs_zone_code = fs_zone_code;  // Zone word used in primitive matching (why not use zone_code in FW? - AWB 18.10.16)
}

// RPC functions
void EMTFPrimitiveConversion::convert_rpc(
    int pc_sector, int pc_station, int pc_chamber, int pc_segment,
    const TriggerPrimitive& muon_primitive1,
    const TriggerPrimitive& muon_primitive2,
    EMTFHitExtra& conv_hit,
    const std::unique_ptr<L1TMuonEndCap::GeometryTranslator>& tp_geom
) const {

  const RPCDetId tp_detId  = muon_primitive1.detId<RPCDetId>();
  const RPCData& tp_data1  = muon_primitive1.getRPCData();
  const RPCData& tp_data2  = muon_primitive2.getRPCData();

  assert(tp_detId.region() != 0);      // We shouldn't have barrel RPCs at this point
  assert(tp_data1.bx == tp_data2.bx);  // We shouldn't have any clusters with multiple BX 
  
  int tp_endcap    = (tp_detId.region() == 1 ? 1 : 2);
  int tp_sector    = tp_detId.sector();         // 1 - 6 (60 degrees in phi, sector 1 begins at -5 deg)
  int tp_station   = tp_detId.station();        // 1 - 4 (Same as in CSC)
  int tp_ring      = tp_detId.ring();           // 2 - 3 (increasing theta) 
  int tp_roll      = tp_detId.roll();           // 1 - 3 (decreasing theta; aka A-B-C; space between rolls is 9 - 15 in theta_fp)
  int tp_subsector = tp_detId.subsector();      // 1 - 6 (10 degrees in phi)  (Staggered in z? theta shifts by 1.0 deg in RPC1/2, 0.2 deg in RPC 1/3 - AWB 28.10.16)

  int tp_bx        = tp_data1.bx;

  // Set properties
  conv_hit.endcap      = tp_endcap;
  conv_hit.station     = tp_station;
  conv_hit.ring        = tp_ring;
  conv_hit.roll        = tp_roll;
  // conv_hit.chamber     = tp_chamber;
  conv_hit.sector      = tp_sector;
  conv_hit.subsector   = tp_subsector;
  // conv_hit.csc_ID      = tp_csc_ID;
  // conv_hit.cscn_ID     = cscn_ID;

  conv_hit.bx          = tp_bx + bxShiftRPC_;
  conv_hit.subsystem   = TriggerPrimitive::kRPC;

  conv_hit.pc_sector   = pc_sector;
  conv_hit.pc_station  = pc_station; // Not defined in FW yet
  conv_hit.pc_chamber  = pc_chamber; // Not defined in FW yet
  conv_hit.pc_segment  = pc_segment;

  conv_hit.valid       = true;  // No "valid" bit from getRPCData()
  conv_hit.strip       = int((tp_data1.strip + tp_data2.strip) / 2);
  conv_hit.strip_low   = tp_data1.strip;
  conv_hit.strip_hi    = tp_data2.strip;
  // conv_hit.wire        = tp_data.keywire;
  // conv_hit.quality     = tp_data.quality;
  // conv_hit.pattern     = tp_data.pattern;
  // conv_hit.bend        = tp_data.bend;

  // conv_hit.bc0         = 0; // Not used anywhere, but part of EMTF DAQ output
  // conv_hit.mpc_link    = tp_data.mpclink; // Used? Delete from class? - AWB 29.09.16
  // conv_hit.sync_err    = tp_data.syncErr; // Used? Delete from class? - AWB 29.09.16
  // conv_hit.track_num   = tp_data.trknmb; // Used? Delete from class? - AWB 29.09.16
  // conv_hit.stub_num    = 0; // Should define in same way as firmware - AWB 29.09.16
  // conv_hit.bx0         = tp_data.bx0; // Used? Delete from class? - AWB 29.09.16
  // conv_hit.layer       = 0; // Used? Delete from class? - AWB 29.09.16

  convert_rpc_details(conv_hit, muon_primitive1, muon_primitive2, tp_geom);
}

void EMTFPrimitiveConversion::convert_rpc_details(EMTFHitExtra& conv_hit, 
						  const TriggerPrimitive& muon_primitive1,
						  const TriggerPrimitive& muon_primitive2,
						  const std::unique_ptr<L1TMuonEndCap::GeometryTranslator>& tp_geom) const {

  bool is_neighbor = (conv_hit.subsector == 2) && (conv_hit.sector == conv_hit.pc_sector);
  
  // std::unique_ptr<const RPCRoll> roll(rpc_geom_->roll( tp_detId ));
  // const LocalPoint  loc_pt = roll->centreOfStrip( conv_hit.strip );
  // const GlobalPoint glb_pt = roll->toGlobal( loc_pt );
  // roll.release();

  // float glob_phi   = glb_pt.phi();
  // float glob_theta = glb_pt.theta();

  float glob_phi1 = tp_geom->calculateGlobalPhi(muon_primitive1);
  float glob_eta1 = tp_geom->calculateGlobalEta(muon_primitive1);
  float glob_phi2 = tp_geom->calculateGlobalPhi(muon_primitive2);
  float glob_eta2 = tp_geom->calculateGlobalEta(muon_primitive2);

  float dPhi = glob_phi2 - glob_phi1;
  if (fabs(dPhi) > Geom::pi())
    dPhi += (Geom::pi() * (dPhi > 0 ? -2. : 2.));
  float dEta = glob_eta2 - glob_eta1;
  assert(fabs(dPhi) < (Geom::pi() / 15.)); // Shouldn't be > 12 degrees
  assert(fabs(dEta) < 0.1);

  float glob_phi = glob_phi1 + (dPhi / 2.);
  float glob_eta = glob_eta1 + (dEta / 2.);
  float glob_theta = 2*atan( exp(-1*glob_eta) );

  // Convert to degrees
  glob_phi   *= (180. / Geom::pi());
  glob_phi   -= 15.;    // Shift so phi = 0 corresponds to the sector 1 boundary 
  if ( glob_phi < (is_neighbor ? -22. : 0.) )  // Convert to [-22, 360]
    glob_phi += 360.;
  glob_theta *= (180. / Geom::pi());
  if (glob_theta > 90)
    glob_theta = 180. - glob_theta;

  float loc_phi = glob_phi - ((conv_hit.pc_sector - 1) * 60.);
  int fph       = ((loc_phi + 22.) * 60.0);  // Rounds down - probably not the best
  int th        = ((glob_theta - 8.5) / 0.285); 
  int zone_hit  = ((fph + 16) >> 5);

  glob_phi   +=  15.;   // Shift back to true value
  if (glob_phi > 180.)  // Convert back to [-180, 180]
    glob_phi -= 360.;

  assert(fph >= 0 && fph <= 4920);

  // Compute the zone code based only on theta, with overlap of 8 for RPCs
  int zone_code = 0;
  for (int izone = 1; izone < NUM_ZONES; ++izone) {
    if ( (th >  (zoneBoundaries_.at(izone) - 8)) && 
	 (th <= (zoneBoundaries_.at(izone + 1) + 8)) )
      zone_code |= (1 << izone);
  }
  assert(zone_code > 0);

  // Needed later for primitive matching ... define arbitrarily for RPCs
  int fs_history = 0;                       // history id: not set here, to be set in primitive matching
  int fs_chamber = -1;                      // chamber id
  int fs_segment = conv_hit.pc_segment % 2; // segment id

  // For ME1
  //   j = 0 is neighbor sector subsector
  //   j = 1,2,3,4,5,6 are native subsectors
  fs_chamber = is_neighbor ? 0 : conv_hit.subsector;

  assert(fs_history >= 0 && fs_chamber != -1 && fs_segment < 2);
  fs_segment = ((fs_history & 0x3)<<4) | ((fs_chamber & 0x7)<<1) | (fs_segment & 0x1);


  conv_hit.phi_fp     = fph;        // Full-precision integer phi
  conv_hit.theta_fp   = th;         // Full-precision integer theta
  // conv_hit.phzvl      = phzvl;      // Local zone word: (1*low) + (2*mid) + (4*low) - used in FW debugging
  // conv_hit.ph_hit     = ph_hit;     // Intermediate quantity in phi calculation - used in FW debugging
  conv_hit.zone_hit   = zone_hit;   // Phi value for building patterns (0.53333 deg precision)
  conv_hit.zone_code  = zone_code;  // Full zone word: 1*(zone 0) + 2*(zone 1) + 4*(zone 2) + 8*(zone 3)
  
  conv_hit.fs_segment   = fs_segment;  // Need to define properly for RPCs? - AWB 29.10.16
  conv_hit.fs_zone_code = zone_code;   // Same as zone_code for now - AWB 29.10.16

  conv_hit.phi_glob_deg = glob_phi;
  conv_hit.phi_loc_deg  = loc_phi;
  conv_hit.theta_deg    = glob_theta;
  conv_hit.eta          = glob_eta;

}
