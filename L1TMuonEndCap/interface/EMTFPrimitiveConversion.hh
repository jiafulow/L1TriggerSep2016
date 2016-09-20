#ifndef L1TMuonEndCap_EMTFPrimitiveConversion_hh
#define L1TMuonEndCap_EMTFPrimitiveConversion_hh

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFCommon.hh"


class EMTFSectorProcessorLUT;

class EMTFPrimitiveConversion {
public:
  void configure(
      const EMTFSectorProcessorLUT* lut,
      int endcap, int sector,
      const std::vector<int>& zoneBoundaries1, const std::vector<int>& zoneBoundaries2, int zoneOverlap
  );

  template<typename T>
  EMTFHitExtra convert(
      T tag,
      int selected,
      const TriggerPrimitive& muon_primitive
  ) const;

  const EMTFSectorProcessorLUT& lut() const;

  // CSC functions
  void convert_csc(EMTFHitExtra& conv_hit) const;

  // RPC functions
  void convert_rpc(EMTFHitExtra& conv_hit) const;

private:
  const EMTFSectorProcessorLUT* lut_;

  int endcap_, sector_;

  std::vector<int> zoneBoundaries1_, zoneBoundaries2_;
  int zoneOverlap_;
};

#endif

