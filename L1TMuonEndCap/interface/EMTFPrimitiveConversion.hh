#ifndef L1TMuonEndCap_EMTFPrimitiveConversion_hh
#define L1TMuonEndCap_EMTFPrimitiveConversion_hh

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFCommon.hh"


class EMTFPrimitiveConversion {
public:
  EMTFPrimitiveConversion();
  ~EMTFPrimitiveConversion();

  template<typename T>
  EMTFHitExtra convert(
      T tag,
      int sector, bool is_neighbor,
      const TriggerPrimitive& muon_primitive
  );

private:
  int sector_;
  bool is_neighbor_;
};

#endif

