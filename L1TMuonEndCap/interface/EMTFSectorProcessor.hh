#ifndef L1TMuonEndCap_EMTFSectorProcessor_hh
#define L1TMuonEndCap_EMTFSectorProcessor_hh

#include <deque>
#include <map>
#include <string>
#include <vector>

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFCommon.hh"

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFSectorProcessorLUT.hh"
#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPrimitiveSelection.hh"
#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPrimitiveConversion.hh"
#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPatternRecognition.hh"


class EMTFSectorProcessor {
public:
  explicit EMTFSectorProcessor();
  ~EMTFSectorProcessor();

  void configure(
      const EMTFSectorProcessorLUT* lut,
      int endcap, int sector,
      int minBX, int maxBX, int bxWindow,
      const std::vector<int>& zoneBoundaries1, const std::vector<int>& zoneBoundaries2, int zoneOverlap,
      const std::vector<std::string>& pattDefinitions, int maxRoadsPerZone,
      bool includeNeighbor, bool duplicateWires
  );

  void process(
      const TriggerPrimitiveCollection& muon_primitives,
      EMTFHitExtraCollection& out_hits,
      EMTFTrackExtraCollection& out_tracks
  ) const;

  void process_single_bx(
      int bx,
      const TriggerPrimitiveCollection& muon_primitives,
      EMTFHitExtraCollection& out_hits,
      EMTFTrackExtraCollection& out_tracks,
      std::deque<EMTFHitExtraCollection>& extended_conv_hits,
      std::map<EMTFPatternId, int>& patt_lifetime_map
  ) const;

  int sector() const { return sector_; }

private:
  const EMTFSectorProcessorLUT* lut_;

  int endcap_, sector_;

  int minBX_, maxBX_, bxWindow_;
  std::vector<int> zoneBoundaries1_, zoneBoundaries2_;
  int zoneOverlap_;
  std::vector<std::string> pattDefinitions_;
  int maxRoadsPerZone_;

  bool includeNeighbor_, duplicateWires_;
};

#endif
