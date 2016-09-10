#ifndef L1TMuonEndCap_EMTFPrimitiveSelection_hh
#define L1TMuonEndCap_EMTFPrimitiveSelection_hh

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFCommon.hh"


class EMTFPrimitiveSelection {
public:
  EMTFPrimitiveSelection();
  ~EMTFPrimitiveSelection();

  // Return
  //   1 if in sector and in bx
  //   2 if in neighbor sector and in bx
  //   0 else

  template<typename T>
  int select(
      T tag,
      int sector, int bx, bool includeNeighbor,
      const TriggerPrimitive& muon_primitive
  );

  // CSC functions
  bool is_in_sector_csc(int tp_sector) const;

  bool is_in_neighbor_sector_csc(
      int tp_sector, int tp_subsector, int tp_station, int tp_csc_ID
  ) const;

  bool is_in_bx_csc(int tp_bx) const;

  // RPC functions
  bool is_in_sector_rpc(int tp_sector) const;

  bool is_in_neighbor_sector_rpc(
      int tp_sector
  ) const;

  bool is_in_bx_rpc(int tp_bx) const;

private:
  bool includeNeighbor_;

  int sector_;
  int bx_;
};

#endif
