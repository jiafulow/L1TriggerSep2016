#ifndef L1TMuonEndCap_EMTFPrimitiveSelection_hh
#define L1TMuonEndCap_EMTFPrimitiveSelection_hh

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFCommon.hh"


class EMTFPrimitiveSelection {
public:
  void configure(int endcap, int sector, int bx, bool includeNeighbor) {
    endcap_ = endcap;
    sector_ = sector;
    bx_ = bx;
    includeNeighbor_ = includeNeighbor;
  }

  // If selected, return an index 0-53, else return -1
  // The index 0-53 roughly corresponds to an input link. It maps to the
  // 2D index [station][chamber] used in the firmware, with size [5:0][8:0].
  // Station 5 = neighbor sector, all stations.
  template<typename T>
  int select(
      T tag,
      const TriggerPrimitive& muon_primitive
  );

  // CSC functions
  bool is_in_sector_csc(int tp_endcap, int tp_sector) const;

  bool is_in_neighbor_sector_csc(
      int tp_endcap, int tp_sector, int tp_subsector, int tp_station, int tp_csc_ID
  ) const;

  bool is_in_bx_csc(int tp_bx) const;

  int get_index_csc(int tp_subsector, int tp_station, int tp_csc_ID, bool is_neighbor) const;

  // RPC functions
  bool is_in_sector_rpc(int tp_endcap, int tp_sector) const;

  bool is_in_neighbor_sector_rpc(
      int tp_endcap, int tp_sector, int tp_subsector, int tp_station, int tp_ring, int tp_roll
  ) const;

  bool is_in_bx_rpc(int tp_bx) const;

  int get_index_rpc(int tp_subsector, int tp_station, int tp_ring, int tp_roll, bool is_neighbor) const;

private:
  int endcap_, sector_;
  int bx_;
  bool includeNeighbor_;
};

#endif
