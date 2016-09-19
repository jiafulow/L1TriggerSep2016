#ifndef L1TMuonEndCap_EMTFPatternRecognition_hh
#define L1TMuonEndCap_EMTFPatternRecognition_hh

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFCommon.hh"
#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPhiMemoryImage.hh"


class EMTFPatternRecognition {
public:
  // Pattern detector ID: [zone][keystrip][pattern]
  typedef std::tuple<int, int, int>  pattern_id_t;

  void configure(
      int endcap, int sector, int bx,
      int minBX, int maxBX, int bxWindow,
      const std::vector<std::string>& pattDefinitions
  );

  void detect(
      const std::deque<EMTFHitExtraCollection>& extended_conv_hits,
      std::map<pattern_id_t, int>& patt_lifetime_map
  );

  void make_zone_images(
      const std::deque<EMTFHitExtraCollection>& extended_conv_hits,
      std::vector<EMTFPhiMemoryImage>& zone_images
  );

  void detect_single_zone(
      int zone,
      EMTFPhiMemoryImage cloned_image,
      std::map<pattern_id_t, int>& patt_lifetime_map,
      EMTFRoadExtraCollection& roads
  );

private:
  int endcap_, sector_, bx_;

  int minBX_, maxBX_, bxWindow_;

  std::vector<EMTFPhiMemoryImage> patterns_;
  std::vector<int> straightnesses_;
};

typedef EMTFPatternRecognition::pattern_id_t EMTFPatternId;

#endif
