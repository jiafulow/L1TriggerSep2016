#ifndef L1TMuonEndCap_EMTFPrimitiveConversion_hh
#define L1TMuonEndCap_EMTFPrimitiveConversion_hh

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFCommon.hh"


class EMTFSectorProcessorLUT;

class EMTFPrimitiveConversion {
public:
  void configure(const EMTFSectorProcessorLUT* lut, int endcap, int sector) {
    lut_    = lut;
    endcap_ = endcap;
    sector_ = sector;
  }

  template<typename T>
  EMTFHitExtra convert(
      T tag,
      int selected,
      const TriggerPrimitive& muon_primitive
  );

  const EMTFSectorProcessorLUT& lut() const;

  // CSC functions
  void convert_csc(EMTFHitExtra& conv_hit);

  // RPC functions
  void convert_rpc(EMTFHitExtra& conv_hit);

private:
  const EMTFSectorProcessorLUT* lut_;

  int endcap_, sector_;
};

#endif

