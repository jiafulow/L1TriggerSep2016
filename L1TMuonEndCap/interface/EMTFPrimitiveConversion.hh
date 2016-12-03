#ifndef L1TMuonEndCap_EMTFPrimitiveConversion_hh
#define L1TMuonEndCap_EMTFPrimitiveConversion_hh

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFCommon.hh"


class EMTFSectorProcessorLUT;

class EMTFPrimitiveConversion {
public:
  void configure(
      const GeometryTranslator* tp_geom,
      const EMTFSectorProcessorLUT* lut,
      int verbose, int endcap, int sector, int bx,
      int bxShiftCSC, int bxShiftRPC,
      const std::vector<int>& zoneBoundaries, int zoneOverlap, int zoneOverlapRPC,
      bool duplicateTheta, bool fixZonePhi, bool useNewZones
  );

  template<typename T>
  void process(
      T tag,
      const std::map<int, TriggerPrimitiveCollection>& selected_prim_map,
      EMTFHitExtraCollection& conv_hits
  ) const;

  const EMTFSectorProcessorLUT& lut() const;

  // CSC functions
  void convert_csc(
      int pc_sector, int pc_station, int pc_chamber, int pc_segment,
      const TriggerPrimitive& muon_primitive,
      EMTFHitExtra& conv_hit
  ) const;
  void convert_csc_details(EMTFHitExtra& conv_hit) const;

  // RPC functions
  void convert_rpc(
      int pc_sector, int pc_station, int pc_chamber, int pc_segment,
      const TriggerPrimitive& muon_primitive,
      EMTFHitExtra& conv_hit
  ) const;
  void convert_rpc_details(EMTFHitExtra& conv_hit) const;

private:
  const GeometryTranslator* tp_geom_;

  const EMTFSectorProcessorLUT* lut_;

  int verbose_, endcap_, sector_, bx_;

  int bxShiftCSC_, bxShiftRPC_;

  std::vector<int> zoneBoundaries_;
  int zoneOverlap_, zoneOverlapRPC_;
  bool duplicateTheta_, fixZonePhi_, useNewZones_;
};

#endif

