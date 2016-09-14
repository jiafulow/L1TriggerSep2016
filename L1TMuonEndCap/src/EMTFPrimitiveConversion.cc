#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPrimitiveConversion.hh"

#include "DataFormats/MuonDetId/interface/DTChamberId.h"
#include "DataFormats/MuonDetId/interface/CSCDetId.h"
#include "DataFormats/MuonDetId/interface/RPCDetId.h"

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFSectorProcessorLUT.hh"

using CSCData = TriggerPrimitive::CSCData;
using RPCData = TriggerPrimitive::RPCData;


// Specialized for CSC
template<>
EMTFHitExtra EMTFPrimitiveConversion::convert(
    CSCTag tag,
    int selected,
    const TriggerPrimitive& muon_primitive
) {
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

  if (tp_station == 1 && tp_ring == 1)
    assert(tp_data.strip <= 128);  // using ME1/1a --> ring 4 convention

  int pcs_station  = selected / 9;
  int pcs_chamber  = selected % 9;

  bool is_me11     = (tp_station == 1 && (tp_ring == 1 || tp_ring == 4));


  EMTFHitExtra conv_hit;
  conv_hit.endcap      = tp_endcap;
  conv_hit.station     = tp_station;
  conv_hit.ring        = tp_ring;
  conv_hit.chamber     = tp_chamber;
  conv_hit.sector      = tp_sector;
  conv_hit.subsector   = tp_subsector;
  conv_hit.csc_ID      = tp_csc_ID;

  conv_hit.bx          = tp_bx - 6;
  conv_hit.subsystem   = TriggerPrimitive::kCSC;
  conv_hit.pcs_station = pcs_station;
  conv_hit.pcs_chamber = pcs_chamber;

  conv_hit.valid       = tp_data.valid;
  conv_hit.strip       = tp_data.strip;
  conv_hit.wire        = tp_data.keywire;
  conv_hit.quality     = tp_data.quality;
  conv_hit.pattern     = tp_data.pattern;
  conv_hit.bend        = tp_data.bend;

  if (is_me11) {
    convert_csc_me11(conv_hit);
  } else {
    convert_csc(conv_hit);
  }

  return conv_hit;
}

// Specialized for RPC
template<>
EMTFHitExtra EMTFPrimitiveConversion::convert(
    RPCTag tag,
    int selected,
    const TriggerPrimitive& muon_primitive
) {
  //const RPCDetId tp_detId = muon_primitive.detId<RPCDetId>();
  //const RPCData& tp_data  = muon_primitive.getRPCData();

  EMTFHitExtra conv_hit;

  convert_rpc(conv_hit);

  return conv_hit;
}

// CSC functions
void EMTFPrimitiveConversion::convert_csc_me11(EMTFHitExtra& conv_hit) {
  assert(conv_hit.station == 1);
  assert(conv_hit.subsector == 1 || conv_hit.subsector == 2);
  assert(
      (conv_hit.ring == 1 && (1 <= conv_hit.csc_ID && conv_hit.csc_ID <= 3)) ||
      (conv_hit.ring == 4 && (10 <= conv_hit.csc_ID && conv_hit.csc_ID <= 12))
  );
  assert(conv_hit.strip < 128);
  assert(conv_hit.wire < 112);
  assert(conv_hit.valid == true);
  assert(conv_hit.pattern <= 10);

  bool is_me11a = (conv_hit.station == 1 && conv_hit.ring == 4);
  bool is_neighbor = (conv_hit.pcs_station == 5);

  // Defined as in firmware
  int fw_endcap  = (conv_hit.endcap-1);
  int fw_sector  = (conv_hit.sector-1);
  int fw_station = (conv_hit.subsector-1);
  int fw_cscid   = is_neighbor ? 12 : (is_me11a ? (conv_hit.csc_ID-1-9) : (conv_hit.csc_ID-1));
  int fw_hstrip  = conv_hit.strip;  // it is half-strip, despite the name
  int fw_wg      = conv_hit.wire;   // it is wiregroup, despite the name

  int pcs_station = conv_hit.pcs_station;
  int pcs_chamber = conv_hit.pcs_chamber;

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

  // CLCT pattern correction array
  //{0.0, 0.0, -0.60,  0.60, -0.64,  0.64, -0.23,  0.23, -0.21,  0.21, 0.0}
  // 0    0    -5      +5    -5      +5    -2      +2    -2      +2    0
  static const int clct_pat_corr_lut[11] = {
    0, 0, -5, +5, -5, +5, -2, +2, -2, +2, 0
  };

  // Programmable parameters for initial position and displacement
  // [0] = ph_init_b
  // [1] = ph_disp_b
  // [2] = ph_init_a
  // [3] = ph_disp_a
  // [4] = th_init
  // [5] = th_disp
  static const int ph_th_params[6] = {
    0, 0, 0, 0, 0, 0  //FIXME
  };

  // Theta LUT, takes wiregroup, returns theta
  static const int th_mem[128] = {0};

  // Theta correction LUT, for ME1/1 only
  // takes index: {wiregroup(2 MS bits), half-strip(5-bit for these chambers)}
  // returns theta correction for tilted wires
  static const int th_corr_mem[128] = {0};

  int th_negative = 50;
  int th_coverage = 45;

  // ___________________________________________________________________________
  // ph conversion

  // Convert half-strip into 1/8-strip
  int eighth_strip = fw_hstrip << 2;

  // Apply phi correction from CLCT pattern number
  int clct_pat_corr = clct_pat_corr_lut[conv_hit.pattern];
  eighth_strip += (clct_pat_corr >> 1);  // uses only 2 bits of pattern correction

  // Multiplicative factor for phi
  int factor = 1024;
  if (is_me11a)
    factor = 1707;  // ME1/1a
  else
    factor = 1301;  // ME1/1b

  // full-precision phi, but local to chamber (counted from strp 1)
  // zone phi precision: 0.53333 deg
  // full phi precision: 0.01666 deg
  int ph_tmp = (eighth_strip * factor) >> 10;
  int fph = 0;     // initial ph for this chamber scaled to 0.1333 deg
  int ph_hit = 0;

  if (ph_reverse)
    ph_tmp = -ph_tmp;

  if (is_me11a)
    fph = ph_th_params[2] + ph_tmp;
  else
    fph = ph_th_params[0] + ph_tmp;

  if (is_me11a)
    ph_hit = (ph_th_params[2+1] >> 1) + (ph_tmp >> 5) + ph_coverage;
  else
    ph_hit = (ph_th_params[0+1] >> 1) + (ph_tmp >> 5) + ph_coverage;


  // ___________________________________________________________________________
  // th conversion

  int th_orig_index = (fw_wg & 0x7f);
  int th_orig = th_mem[th_orig_index];

  int th_corr_index = (((fw_wg >> 4) & 0x3) << 5) + ((eighth_strip >> 4) & 0x1f);
  int th_corr = th_corr_mem[th_corr_index];
  if (ph_reverse)
    th_corr = -th_corr;

  int th_tmp = th_orig + th_corr;

  // Check that correction did not make invalid value outside chamber coverage
  if (th_tmp > th_negative || fw_wg == 0)
    th_tmp = 0;  // limit at the bottom

  if (th_tmp > th_coverage)
    th_tmp = th_coverage;  // limit at the top

  // theta precision: 0.285 degree
  int th = th_tmp + ph_th_params[4];

  // Protect against invalid value
  if (th == 0)
    th = 1;

  // ___________________________________________________________________________
  // zones

  // ph zone boundaries for chambers that cover more than one zone
  // hardcoded boundaries must match boundaries in ph_th_match module
  int ph_zone_bnd1 = 127;
  if (fw_station <= 1 && (fw_cscid <= 2 || fw_cscid == 12))  // ME1/1
    ph_zone_bnd1 = 41;
  else if (fw_station == 2 && (fw_cscid <= 2 || fw_cscid == 9))  // ME2/1
    ph_zone_bnd1 = 41;
  else if (fw_station == 2 && ((fw_cscid >= 3 && fw_cscid <= 8) || fw_cscid == 10))  // ME2/2
    ph_zone_bnd1 = 87;
  else if (fw_station == 3 && ((fw_cscid >= 3 && fw_cscid <= 8) || fw_cscid == 10))  // ME3/2
    ph_zone_bnd1 = 49;
  else if (fw_station == 4 && ((fw_cscid >= 3 && fw_cscid <= 8) || fw_cscid == 10))  // ME4/2
    ph_zone_bnd1 = 49;

  int ph_zone_bnd2 = 127;
  if (fw_station == 3 && ((fw_cscid >= 3 && fw_cscid <= 8) || fw_cscid == 10))  // ME3/2
    ph_zone_bnd2 = 87;

  int zone_overlap = 2;

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

  int zone_contrib = 0;

  if (fw_station <= 1) {  // station 1
    if (fw_cscid <= 2 || fw_cscid == 12) {  // ring 1
      if (phzvl & (1<<0))
        zone_contrib |= (1<<0);  // zone 0

      if (phzvl & (1<<1))
        zone_contrib |= (1<<1);  // zone 1

    } else if (fw_cscid <= 5 || fw_cscid == 13) {  // ring 2
      if (phzvl & (1<<0))
        zone_contrib |= (1<<2);  // zone 2

    } else if (fw_cscid <= 8 || fw_cscid == 14) {  // ring 3
      if (true)
        zone_contrib |= (1<<3);  // zone 3
    }

  } else if (fw_station == 2) {  // station 2
    if (fw_cscid <= 2 || fw_cscid == 9) {  // ring 1
      if (phzvl & (1<<0))
        zone_contrib |= (1<<0);  // zone 0

      if (phzvl & (1<<1))
        zone_contrib |= (1<<1);  // zone 1

    } else if (fw_cscid <= 8 || fw_cscid == 10) {  // ring 2
      if (phzvl & (1<<0))
        zone_contrib |= (1<<2);  // zone 2

      if (phzvl & (1<<1))
        zone_contrib |= (1<<3);  // zone 3
    }

  } else if (fw_station == 3) {  // station 3
    if (fw_cscid <= 2 || fw_cscid == 9) {  // ring 1
      if (phzvl & (1<<0))
        zone_contrib |= (1<<0);  // zone 0

    } else if (fw_cscid <= 8 || fw_cscid == 10) {  // ring 2
      if (phzvl & (1<<0))
        zone_contrib |= (1<<1);  // zone 1

      if (phzvl & (1<<1))
        zone_contrib |= (1<<2);  // zone 2

      if (phzvl & (1<<2))
        zone_contrib |= (1<<3);  // zone 3
    }

  } else if (fw_station == 4) {  // station 4
    if (fw_cscid <= 2 || fw_cscid == 9) {  // ring 1
      if (phzvl & (1<<0))
        zone_contrib |= (1<<0);  // zone 0

    } else if (fw_cscid <= 8 || fw_cscid == 10) {  // ring 2
      if (phzvl & (1<<0))
        zone_contrib |= (1<<1);  // zone 1

      if (phzvl & (1<<1))
        zone_contrib |= (1<<2);  // zone 2
    }
  }

  static const int ph_offsets[6][9] = {
    {39,57,76,39,58,76,41,60,79},
    {95,114,132,95,114,133,98,116,135},
    {38,76,113,39,58,76,95,114,132},
    {38,76,113,39,58,76,95,114,132},
    {38,76,113,38,57,76,95,113,132},
    {21,21,23,1,21,1,21,1,20}
  };

  int zone_hit = ph_offsets[pcs_station][pcs_chamber] + ph_hit;

  // ___________________________________________________________________________
  // Output

  conv_hit.phi_fp          = fph;
  conv_hit.theta_fp        = th;
  conv_hit.phzvl           = phzvl;
  conv_hit.ph_hit          = ph_hit;
  conv_hit.ph_zone_hit     = zone_hit;
  conv_hit.ph_zone_contrib = zone_contrib;
}

void EMTFPrimitiveConversion::convert_csc(EMTFHitExtra& conv_hit) {
  assert(
      (conv_hit.station == 1 && (conv_hit.ring == 2 || conv_hit.ring == 3)) ||
      (2 <= conv_hit.station && conv_hit.station <= 4)
  );
  assert(1 <= conv_hit.csc_ID && conv_hit.csc_ID <= 9);
  assert(conv_hit.strip < 160);
  assert(conv_hit.wire < 112);
  assert(conv_hit.valid == true);
  assert(conv_hit.pattern <= 10);

  bool is_neighbor = (conv_hit.pcs_station == 5);

  // Defined as in firmware
  int fw_endcap  = (conv_hit.endcap-1);
  int fw_sector  = (conv_hit.sector-1);
  int fw_station = (conv_hit.station);
  int fw_cscid   = is_neighbor ? (fw_station == 1 ? conv_hit.csc_ID-1+12 : ((conv_hit.csc_ID-1-1)%2) + 9) : (conv_hit.csc_ID-1);
  int fw_hstrip  = conv_hit.strip;  // it is half-strip, despite the name
  int fw_wg      = conv_hit.wire;   // it is wiregroup, despite the name

  int pcs_station = conv_hit.pcs_station;
  int pcs_chamber = conv_hit.pcs_chamber;

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

  bool is_10degree = false;
  if (
      (fw_station <= 1) || // ME1
      (fw_station >= 2 && ((fw_cscid >= 3 && fw_cscid <= 8) || fw_cscid == 10))  // ME2,3,4/2
  )
    is_10degree = true;

  // CLCT pattern correction array
  //{0.0, 0.0, -0.60,  0.60, -0.64,  0.64, -0.23,  0.23, -0.21,  0.21, 0.0}
  // 0    0    -5      +5    -5      +5    -2      +2    -2      +2    0
  static const int clct_pat_corr_lut[11] = {
    0, 0, -5, +5, -5, +5, -2, +2, -2, +2, 0
  };

  // Programmable parameters for initial position and displacement
  // [0] = ph_init
  // [1] = th_init
  // [2] = ph_disp
  // [3] = th_disp
  static const int ph_th_params[4] = {
    0, 0, 0, 0 //FIXME
  };

  // Theta LUT, takes wiregroup, returns theta
  static const int th_mem[128] = {0};

  // ___________________________________________________________________________
  // ph conversion

  // Convert half-strip into 1/8-strip
  int eighth_strip = 0;

  // Apply phi correction from CLCT pattern number
  int clct_pat_corr = clct_pat_corr_lut[conv_hit.pattern];

  if (is_10degree) {
    eighth_strip = fw_hstrip << 2;  // full precision, uses only 2 bits of pattern correction
    eighth_strip += (clct_pat_corr >> 1);
  } else {
    eighth_strip = fw_hstrip << 3;  // full precision, uses all 3 bits of pattern correction
    eighth_strip += (clct_pat_corr >> 0);
  }

  // Multiplicative factor for strip
  int factor = 1024;
  if (fw_station <= 1 && ((fw_cscid >= 6 && fw_cscid <= 8) || (fw_cscid == 14)))  // ME1/3
    factor = 947;

  // full-precision phi, but local to chamber (counted from strp 1)
  // zone phi precision: 0.53333 deg
  // full phi precision: 0.01666 deg
  int ph_tmp = (eighth_strip * factor) >> 10;
  int fph = 0;     // initial ph for this chamber scaled to 0.1333 deg
  int ph_hit = 0;

  if (ph_reverse)
    ph_tmp = -ph_tmp;

  fph = ph_th_params[0] + ph_tmp;
  ph_hit = (ph_th_params[2] >> 1) + (ph_tmp >> 5) + ph_coverage;

  // ___________________________________________________________________________
  // th conversion

  int th_orig_index = (fw_wg & 0x7f);
  int th_orig = th_mem[th_orig_index];

  int th_tmp = th_orig;

  // theta precision: 0.285 degree
  int th = th_tmp + ph_th_params[1];

  // Protect against invalid value
  if (th == 0)
    th = 1;

  // ___________________________________________________________________________
  // zones

  // ph zone boundaries for chambers that cover more than one zone
  // hardcoded boundaries must match boundaries in ph_th_match module
  int ph_zone_bnd1 = 127;
  if (fw_station <= 1 && (fw_cscid <= 2 || fw_cscid == 12))  // ME1/1
    ph_zone_bnd1 = 41;
  else if (fw_station == 2 && (fw_cscid <= 2 || fw_cscid == 9))  // ME2/1
    ph_zone_bnd1 = 41;
  else if (fw_station == 2 && ((fw_cscid >= 3 && fw_cscid <= 8) || fw_cscid == 10))  // ME2/2
    ph_zone_bnd1 = 87;
  else if (fw_station == 3 && ((fw_cscid >= 3 && fw_cscid <= 8) || fw_cscid == 10))  // ME3/2
    ph_zone_bnd1 = 49;
  else if (fw_station == 4 && ((fw_cscid >= 3 && fw_cscid <= 8) || fw_cscid == 10))  // ME4/2
    ph_zone_bnd1 = 49;

  int ph_zone_bnd2 = 127;
  if (fw_station == 3 && ((fw_cscid >= 3 && fw_cscid <= 8) || fw_cscid == 10))  // ME3/2
    ph_zone_bnd2 = 87;

  int zone_overlap = 2;

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

  int zone_contrib = 0;

  if (fw_station <= 1) {  // station 1
    if (fw_cscid <= 2 || fw_cscid == 12) {  // ring 1
      if (phzvl & (1<<0))
        zone_contrib |= (1<<0);  // zone 0

      if (phzvl & (1<<1))
        zone_contrib |= (1<<1);  // zone 1

    } else if (fw_cscid <= 5 || fw_cscid == 13) {  // ring 2
      if (phzvl & (1<<0))
        zone_contrib |= (1<<2);  // zone 2

    } else if (fw_cscid <= 8 || fw_cscid == 14) {  // ring 3
      if (true)
        zone_contrib |= (1<<3);  // zone 3
    }

  } else if (fw_station == 2) {  // station 2
    if (fw_cscid <= 2 || fw_cscid == 9) {  // ring 1
      if (phzvl & (1<<0))
        zone_contrib |= (1<<0);  // zone 0

      if (phzvl & (1<<1))
        zone_contrib |= (1<<1);  // zone 1

    } else if (fw_cscid <= 8 || fw_cscid == 10) {  // ring 2
      if (phzvl & (1<<0))
        zone_contrib |= (1<<2);  // zone 2

      if (phzvl & (1<<1))
        zone_contrib |= (1<<3);  // zone 3
    }

  } else if (fw_station == 3) {  // station 3
    if (fw_cscid <= 2 || fw_cscid == 9) {  // ring 1
      if (phzvl & (1<<0))
        zone_contrib |= (1<<0);  // zone 0

    } else if (fw_cscid <= 8 || fw_cscid == 10) {  // ring 2
      if (phzvl & (1<<0))
        zone_contrib |= (1<<1);  // zone 1

      if (phzvl & (1<<1))
        zone_contrib |= (1<<2);  // zone 2

      if (phzvl & (1<<2))
        zone_contrib |= (1<<3);  // zone 3
    }

  } else if (fw_station == 4) {  // station 4
    if (fw_cscid <= 2 || fw_cscid == 9) {  // ring 1
      if (phzvl & (1<<0))
        zone_contrib |= (1<<0);  // zone 0

    } else if (fw_cscid <= 8 || fw_cscid == 10) {  // ring 2
      if (phzvl & (1<<0))
        zone_contrib |= (1<<1);  // zone 1

      if (phzvl & (1<<1))
        zone_contrib |= (1<<2);  // zone 2
    }
  }

  static const int ph_offsets[6][9] = {
    {39,57,76,39,58,76,41,60,79},
    {95,114,132,95,114,133,98,116,135},
    {38,76,113,39,58,76,95,114,132},
    {38,76,113,39,58,76,95,114,132},
    {38,76,113,38,57,76,95,113,132},
    {21,21,23,1,21,1,21,1,20}
  };

  int zone_hit = ph_offsets[pcs_station][pcs_chamber] + ph_hit;

  // ___________________________________________________________________________
  // Output

  conv_hit.phi_fp          = fph;
  conv_hit.theta_fp        = th;
  conv_hit.phzvl           = phzvl;
  conv_hit.ph_hit          = ph_hit;
  conv_hit.ph_zone_hit     = zone_hit;
  conv_hit.ph_zone_contrib = zone_contrib;
}

// RPC functions
void EMTFPrimitiveConversion::convert_rpc(EMTFHitExtra& conv_hit) {

}
