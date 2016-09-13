#ifndef L1TMuonEndCap_EMTFPrimitiveConversion_hh
#define L1TMuonEndCap_EMTFPrimitiveConversion_hh

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFCommon.hh"


class EMTFPrimitiveConversion {
public:
  void configure(int endcap, int sector) {
    endcap_ = endcap;
    sector_ = sector;
  }

  template<typename T>
  EMTFHitExtra convert(
      T tag,
      int selected,
      const TriggerPrimitive& muon_primitive
  );

  // CSC functions
  void convert_csc_me11(EMTFHitExtra& conv_hit);

  void convert_csc(EMTFHitExtra& conv_hit);

  // RPC functions
  void convert_rpc(EMTFHitExtra& conv_hit);

private:
  int endcap_, sector_;
};

#endif

