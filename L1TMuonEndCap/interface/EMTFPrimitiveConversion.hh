#ifndef L1TMuonEndCap_EMTFPrimitiveConversion_hh
#define L1TMuonEndCap_EMTFPrimitiveConversion_hh

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFCommon.hh"


class EMTFPrimitiveConversion {
public:
  EMTFPrimitiveConversion();
  ~EMTFPrimitiveConversion();

  void convert(
      int sector, int bx, bool includeNeighbor,
      const TriggerPrimitiveCollection& muon_primitives,
      EMTFHitExtraCollection& conv_hits
  );

  bool is_in_sector_csc(
      int tp_endcap, int tp_sector, int tp_subsector, int tp_station, int tp_csc_ID,
      int& is_neighbor
  ) const;

  bool is_in_bx_csc(int tp_bx) const;

  bool is_in_sector_rpc(
      int tp_endcap, int tp_sector
  ) const;

  bool is_in_bx_rpc(int tp_bx) const;

  EMTFHitExtra make_emtf_hit(const TriggerPrimitive& muon_primitive, int is_neighbor) const;

private:
  bool includeNeighbor_;

  int sector_;
  int bx_;
};

#endif

