#ifndef L1TMuonEndCap_EMTFPrimitiveConversion_hh
#define L1TMuonEndCap_EMTFPrimitiveConversion_hh

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFCommon.hh"


class EMTFSectorProcessorLUT;

class EMTFPrimitiveConversion {
public:
  void configure(
      const EMTFSectorProcessorLUT* lut,
      int verbose, int endcap, int sector, int bx,
      bool duplicateTheta, bool fixZonePhi,
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
  void convert_csc(int selected, const TriggerPrimitive& muon_primitive, EMTFHitExtra& conv_hit) const;
  void convert_csc_details(EMTFHitExtra& conv_hit) const;

  // RPC functions
  void convert_rpc(int selected, const TriggerPrimitive& muon_primitive, EMTFHitExtra& conv_hit) const;
  void convert_rpc_details(EMTFHitExtra& conv_hit) const;

private:
  const EMTFSectorProcessorLUT* lut_;

  int verbose_, endcap_, sector_, bx_;

  bool duplicateTheta_, fixZonePhi_;

  std::vector<int> zoneBoundaries1_, zoneBoundaries2_;
  int zoneOverlap_;
};

#endif

