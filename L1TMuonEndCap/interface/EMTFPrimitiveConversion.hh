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
      bool is_neighbor,
      const TriggerPrimitive& muon_primitive
  );

private:
  int endcap_, sector_;
};

#endif

