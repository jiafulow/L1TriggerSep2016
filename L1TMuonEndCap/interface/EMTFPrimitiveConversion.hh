#ifndef L1TMuonEndCap_EMTFPrimitiveConversion_hh
#define L1TMuonEndCap_EMTFPrimitiveConversion_hh

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFCommon.hh"


class EMTFSectorProcessorLUT;

class EMTFPrimitiveConversion {
public:
  void configure(
      const EMTFSectorProcessorLUT* lut,
      int verbose, int endcap, int sector, int bx,
      const std::vector<int>& zoneBoundaries1, const std::vector<int>& zoneBoundaries2, int zoneOverlap
  );

  template<typename T>
  void process(
      T tag,
      const std::map<int, TriggerPrimitiveCollection>& selected_prim_map,
      EMTFHitExtraCollection& conv_hits
  ) const;

  const EMTFSectorProcessorLUT& lut() const;

  // CSC functions
  EMTFHitExtra convert_prim_csc(int selected, const TriggerPrimitive& muon_primitive) const;
  void convert_csc(EMTFHitExtra& conv_hit) const;

  // RPC functions
  EMTFHitExtra convert_prim_rpc(int selected, const TriggerPrimitive& muon_primitive) const;
  void convert_rpc(EMTFHitExtra& conv_hit) const;

private:
  const EMTFSectorProcessorLUT* lut_;

  int verbose_, endcap_, sector_, bx_;

  std::vector<int> zoneBoundaries1_, zoneBoundaries2_;
  int zoneOverlap_;
};

#endif

