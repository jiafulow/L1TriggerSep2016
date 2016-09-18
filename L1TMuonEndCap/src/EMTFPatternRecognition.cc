#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPatternRecognition.hh"

#include <iterator>
#include <string>
#include <regex>

#define NUM_ZONES 4
#define NUM_ZONE_HITS 160
#define NUM_PATTERNS 9


void EMTFPatternRecognition::configure(
    int minBX, int maxBX, int bxWindow,
    const std::vector<std::string>& pattDefinitions
) {
  minBX_           = minBX;
  maxBX_           = maxBX;
  bxWindow_        = bxWindow;

  patterns_.clear();
  straightnesses_.clear();

  std::regex digits("(\\d+)");

  for (const auto& s: pattDefinitions) {
    auto digits_begin = std::sregex_iterator(s.begin(), s.end(), digits);
    auto digits_end = std::sregex_iterator();
    //for (std::sregex_iterator imatch = digits_begin; imatch != digits_end; ++imatch) {
    //  std::cout << imatch->str() << std::endl;
    //}
    assert(std::distance(digits_begin, digits_end) == 9);  // Want to find 9 numbers

    std::sregex_iterator imatch = digits_begin;
    auto to_integer = [](auto imatch) { return std::stoi(imatch->str()); };

    // Get the 9 integers
    // straightness, hits in ME1, hits in ME2, hits in ME3, hits in ME4
    int straightness = to_integer(imatch++);
    int st1_min      = to_integer(imatch++);
    int st1_max      = to_integer(imatch++);
    int st2_min      = to_integer(imatch++);
    int st2_max      = to_integer(imatch++);
    int st3_min      = to_integer(imatch++);
    int st3_max      = to_integer(imatch++);
    int st4_min      = to_integer(imatch++);
    int st4_max      = to_integer(imatch++);

    // Create a pattern
    EMTFPhiMemoryImage pattern;
    for (int i = st1_min; i <= st1_max; i++)
      pattern.set_bit(0, i);
    for (int i = st2_min; i <= st2_max; i++)
      pattern.set_bit(1, i);
    for (int i = st3_min; i <= st3_max; i++)
      pattern.set_bit(2, i);
    for (int i = st4_min; i <= st4_max; i++)
      pattern.set_bit(3, i);

    patterns_.push_back(pattern);
    straightnesses_.push_back(straightness);
  }
  assert(patterns_.size() == NUM_PATTERNS);

  // pattern half-width for stations 3,4
  const int pat_w_st3 = 3;
  // pattern half-width for station 1
  const int pat_w_st1 = pat_w_st3 + 1;
  // number of input bits for stations 3,4
  const int full_pat_w_st3 = (1 << (pat_w_st3+1)) - 1;  // = 15
  // number of input bits for station 1
  const int full_pat_w_st1 = (1 << (pat_w_st1+1)) - 1;  // = 31
  // width of zero padding for station copies
  const int padding_w_st3 = full_pat_w_st3 / 2;  // = 7
  const int padding_w_st1 = full_pat_w_st1 / 2;  // = 15

  padding_w_st3_ = padding_w_st3;
  padding_w_st1_ = padding_w_st1;
}

void EMTFPatternRecognition::detect(
    const std::deque<EMTFHitExtraCollection>& extended_conv_hits,
    std::map<pattern_id_t, int>& patt_lifetime_map
) {

  // Make zone images
  std::vector<EMTFPhiMemoryImage> zone_images;
  make_zone_images(extended_conv_hits, zone_images);

  // Perform pattern matching
  std::vector<EMTFRoadExtraCollection> zone_roads;

  for (int izone = 0; izone < NUM_ZONES; ++izone) {
    EMTFRoadExtraCollection roads;

    const EMTFPhiMemoryImage& image = zone_images.at(izone);
    detect_single_zone(izone, image, patt_lifetime_map, roads);

    zone_roads.push_back(roads);
  }

}

void EMTFPatternRecognition::make_zone_images(
    const std::deque<EMTFHitExtraCollection>& extended_conv_hits,
    std::vector<EMTFPhiMemoryImage>& zone_images
) {
  zone_images.clear();

  // Prepare zone images
  for (int izone = 0; izone < NUM_ZONES; ++izone) {
    // Create a zone image
    EMTFPhiMemoryImage image;
    zone_images.push_back(image);
  }

  // Fill zone images
  std::deque<EMTFHitExtraCollection>::const_iterator ext_conv_hits_it  = extended_conv_hits.begin();
  std::deque<EMTFHitExtraCollection>::const_iterator ext_conv_hits_end = extended_conv_hits.end();

  for (; ext_conv_hits_it != ext_conv_hits_end; ++ext_conv_hits_it) {
    EMTFHitExtraCollection::const_iterator conv_hits_it  = ext_conv_hits_it->begin();
    EMTFHitExtraCollection::const_iterator conv_hits_end = ext_conv_hits_it->end();

    for (; conv_hits_it != conv_hits_end; ++conv_hits_it) {

      for (int izone = 0; izone < NUM_ZONES; ++izone) {
        const EMTFHitExtra& conv_hit = *conv_hits_it;

        if (conv_hit.ph_zone_contrib & (1<<izone)) {  // hit belongs to this zone
          unsigned int layer = conv_hit.station - 1;
          unsigned int bit   = conv_hit.ph_zone_hit;

          // Add padding of zeroes so that it's easier to pass to detectors
          if (izone == 0) {
            bit += padding_w_st1_;
          } else {
            bit += padding_w_st3_;
          }

          EMTFPhiMemoryImage& image = zone_images.at(izone);  // pass by reference
          image.set_bit(layer, bit);
        }

      }
    }  // end loop over conv_hits
  }  // end loop over extended_conv_hits
}

void EMTFPatternRecognition::detect_single_zone(
    int zone,
    const EMTFPhiMemoryImage& image,
    std::map<pattern_id_t, int>& patt_lifetime_map,
    EMTFRoadExtraCollection& roads
) {
  roads.clear();

  for (int ipatt = 0; ipatt < NUM_PATTERNS; ++ipatt) {
    EMTFPhiMemoryImage patt = patterns_.at(ipatt);  // clone
    int straightness        = straightnesses_.at(ipatt);
    int layer_code          = 0;

    for (int izhit = 0; izhit < NUM_ZONE_HITS; ++izhit) {
      // In firmware, the zone phi image is rotated right/shift down to compare with patterns
      // In emulator, the pattern is rotated left/shift up to compare with the zone phi image
      // They should be the same!

      layer_code = patt.op_and(image);  // kind of like AND operator

      const pattern_id_t patt_id = std::make_tuple(zone, izhit, ipatt);
      bool is_lifetime_up = false;

      if (layer_code > 0) {
        // Starts counting at any hit in the pattern, even single
        auto ins = patt_lifetime_map.insert({patt_id, 1});

        if (ins.second) {  // already exists
          ins.first->second += 1;

          // Is lifetime up?
          if (ins.first->second == bxWindow_) {  // delayBX = bxWindow_ - 1
            is_lifetime_up = true;
          }
        }

      } else {
        patt_lifetime_map.erase(patt_id);  // erase if exists
      }

      // Rotate
      patt.rotl(1);

      // If lifetime is up, find roads
      if (is_lifetime_up) {
        int quality_code = (
            ((straightness & (1<<2)) << 5) |
            ((straightness & (1<<1)) << 3) |
            ((straightness & (1<<0)) << 1) |
            ((layer_code & (1<<2)) << 4) |
            ((layer_code & (1<<1)) << 2) |
            ((layer_code & (1<<0)) << 0)
        );

        EMTFRoadExtra road;
        road.zone     = std::get<0>(patt_id);
        road.key_zhit = std::get<1>(patt_id);
        road.pattern  = std::get<2>(patt_id);

        road.straightness = straightness;
        road.layer_code   = layer_code;
        road.quality_code = quality_code;
        roads.push_back(road);
      }

    }  // end loop over zone hits
  }  // end loop over patterns

}
