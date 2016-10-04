#ifndef L1TMuonEndCap_EMTFPatternRecognition_hh
#define L1TMuonEndCap_EMTFPatternRecognition_hh

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFCommon.hh"
#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPhiMemoryImage.hh"


class EMTFPatternRecognition {
public:
  // Pattern detector ID: [zone, keystrip, pattern]
  typedef std::array<int, 3>  pattern_ref_t;

  void configure(
      int verbose, int endcap, int sector, int bx,
      const std::vector<std::string>& pattDefinitions, const std::vector<std::string>& symPattDefinitions,
      int maxRoadsPerZone, bool useSecondEarliest, bool useSymPatterns
  );

  void configure_details();

  void process(
      const std::deque<EMTFHitExtraCollection>& extended_conv_hits,
      std::map<pattern_ref_t, int>& patt_lifetime_map,
      std::vector<EMTFRoadExtraCollection>& zone_roads
  ) const;

  void make_zone_image(
      int zone,
      const std::deque<EMTFHitExtraCollection>& extended_conv_hits,
      EMTFPhiMemoryImage& image
  ) const;

  void process_single_zone(
      int zone,
      EMTFPhiMemoryImage cloned_image,
      std::map<pattern_ref_t, int>& patt_lifetime_map,
      EMTFRoadExtraCollection& roads
  ) const;

  void sort_single_zone(EMTFRoadExtraCollection& roads) const;


private:
  int verbose_, endcap_, sector_, bx_;

  std::vector<std::string> pattDefinitions_, symPattDefinitions_;
  int maxRoadsPerZone_;
  bool useSecondEarliest_, useSymPatterns_;

  std::vector<EMTFPhiMemoryImage> patterns_;
};

typedef EMTFPatternRecognition::pattern_ref_t EMTFPatternRef;

#endif
