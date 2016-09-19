#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPatternRecognition.hh"

#include <iterator>
#include <string>
#include <regex>

#define NUM_ZONES 4
#define NUM_ZONE_HITS 160
#define NUM_PATTERNS 9
#define PATTERN_KEY_ZHIT 7


void EMTFPatternRecognition::configure(
    int minBX, int maxBX, int bxWindow,
    const std::vector<std::string>& pattDefinitions
) {
  minBX_           = minBX;
  maxBX_           = maxBX;
  bxWindow_        = bxWindow;

  patterns_.clear();
  straightnesses_.clear();

  // Parse pattern definitions
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

    // There can only be one zone hit in the key station in the pattern
    // and it has to be this magic number
    assert(st2_min == PATTERN_KEY_ZHIT && st2_max == PATTERN_KEY_ZHIT);

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

  //// pattern half-width for stations 3,4
  //const int pat_w_st3 = 3;
  //// pattern half-width for station 1
  //const int pat_w_st1 = pat_w_st3 + 1;
  //// number of input bits for stations 3,4
  //const int full_pat_w_st3 = (1 << (pat_w_st3+1)) - 1;  // = 15
  //// number of input bits for station 1
  //const int full_pat_w_st1 = (1 << (pat_w_st1+1)) - 1;  // = 31
  //// width of zero padding for station copies
  //const int padding_w_st3 = full_pat_w_st3 / 2;  // = 7
  //const int padding_w_st1 = full_pat_w_st1 / 2;  // = 15

  // Don't need to explicitly pad with zeroes in EMTFPhiMemoryImage because
  // it has 192 bits allocated. 160 bits are used; the most-significant
  // 32 bits are unused and can be treated as implicitly padded zeroes.
}

void EMTFPatternRecognition::detect(
    const std::deque<EMTFHitExtraCollection>& extended_conv_hits,
    std::map<pattern_id_t, int>& patt_lifetime_map
) {

  // Make zone images
  std::vector<EMTFPhiMemoryImage> zone_images;
  make_zone_images(extended_conv_hits, zone_images);

  if (true) {  // debug
    for (const auto& conv_hits : extended_conv_hits) {
      for (const auto& conv_hit : conv_hits) {
        std::cout << "st: " << conv_hit.pc_station << " ch: " << conv_hit.pc_chamber
            << " ph: " << conv_hit.phi_fp << " th: " << conv_hit.theta_fp
            << " ph_hit: " << (1ul<<conv_hit.ph_hit) << " phzvl: " << conv_hit.phzvl
            << " zone_hit: " << conv_hit.zone_hit << " zone_code: " << conv_hit.zone_code
            << std::endl;
      }
    }

    for (int izone = NUM_ZONES; izone >= 1; --izone) {
      std::cout << "zone: " << izone << std::endl;
      zone_images.at(izone-1).print(std::cout);
    }
  }

  // Perform pattern recognition in each zone
  std::vector<EMTFRoadExtraCollection> zone_roads;

  for (int izone = 0; izone < NUM_ZONES; ++izone) {
    EMTFRoadExtraCollection roads;

    detect_single_zone(izone, zone_images.at(izone), patt_lifetime_map, roads);

    zone_roads.push_back(roads);
  }

  // Debug
  for (const auto& roads : zone_roads) {
    for (const auto& road : roads) {
      std::cout << "road " << road.zone << " " << road.key_zhit << " " << road.pattern << " " << road.straightness << " " << road.layer_code << " " << road.quality_code << std::endl;
    }
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
      const EMTFHitExtra& conv_hit = *conv_hits_it;

      for (int izone = 0; izone < NUM_ZONES; ++izone) {
        if (conv_hit.zone_code & (1<<izone)) {  // hit belongs to this zone
          unsigned int layer = conv_hit.station - 1;
          unsigned int bit   = conv_hit.zone_hit;

          EMTFPhiMemoryImage& image = zone_images.at(izone);  // pass by reference
          image.set_bit(layer, bit);
        }
      }
    }  // end loop over conv_hits
  }  // end loop over extended_conv_hits
}

void EMTFPatternRecognition::detect_single_zone(
    int zone,
    EMTFPhiMemoryImage cloned_image,
    std::map<pattern_id_t, int>& patt_lifetime_map,
    EMTFRoadExtraCollection& roads
) {
  roads.clear();

  // The zone hit image is rotated/shifted before comparing with patterns
  // First, rotate left/shift up to the zone hit in the key station
  cloned_image.rotl(PATTERN_KEY_ZHIT);

  for (int izhit = 0; izhit < NUM_ZONE_HITS; ++izhit) {
    // The zone hit image is rotated/shift before comparing with patterns
    // For every zone hit, rotate right/shift down by one
    if (izhit > 0)
      cloned_image.rotr(1);

    int max_quality_code = -1;
    EMTFRoadExtra tmp_road;

    // Compare with patterns
    for (int ipatt = 0; ipatt < NUM_PATTERNS; ++ipatt) {
      const EMTFPhiMemoryImage& patt = patterns_.at(ipatt);
      const pattern_id_t patt_id = std::make_tuple(zone, izhit, ipatt);

      bool is_lifetime_up = false;

      int layer_code = patt.op_and(cloned_image);  // kind of like AND operator

      if (layer_code > 0) {
        // Starts counting at any hit in the pattern, even single
        auto ins = patt_lifetime_map.insert({patt_id, 1});

        if (!ins.second) {  // already exists, increment counter
          ins.first->second += 1;

          // Is lifetime up?
          if (ins.first->second == bxWindow_) {  // = 3
            is_lifetime_up = true;
          }
        }

      } else {
        patt_lifetime_map.erase(patt_id);  // erase if exists
      }

      // If lifetime is up, and not single-layer hit patterns (stations 3&4 considered
      // as a single layer), find quality of this pattern
      if (
        is_lifetime_up &&
        ((layer_code != 1) && (layer_code != 2) && (layer_code != 4) && (layer_code != 0))
      ) {
        int straightness = straightnesses_.at(ipatt);

        // This quality code scheme is giving almost-equal priority to
        // more stations and better straightness
        // Station 1 has higher weight, station 2 lower, stations 3&4 lowest
        int quality_code = (
            (((straightness>>2) & 1) << 5) |
            (((straightness>>1) & 1) << 3) |
            (((straightness>>0) & 1) << 1) |
            (((layer_code>>2)   & 1) << 4) |
            (((layer_code>>1)   & 1) << 2) |
            (((layer_code>>0)   & 1) << 0)
        );

        EMTFRoadExtra road;
        road.zone     = std::get<0>(patt_id);
        road.key_zhit = std::get<1>(patt_id);
        road.pattern  = std::get<2>(patt_id);

        road.straightness = straightness;
        road.layer_code   = layer_code;
        road.quality_code = quality_code;

        // Find max quality code in a given key_zhit
        if (max_quality_code < road.quality_code) {
          max_quality_code = road.quality_code;
          tmp_road = road;
        }
      }

    }  // end loop over zone hits

    // Output road
    if (max_quality_code != -1) {
      roads.push_back(tmp_road);
    }

  }  // end loop over patterns

  // Ghost cancellation logic by considering neighbor patterns
  std::vector<int> quality_codes(NUM_ZONE_HITS, 0);

  for (unsigned iroad = 0; iroad < roads.size(); ++iroad) {
    const EMTFRoadExtra& road = roads.at(iroad);
    quality_codes.at(road.key_zhit) = road.quality_code;
  }

  std::vector<bool> mask_roads(roads.size(), true);

  for (unsigned iroad = 0; iroad < roads.size(); ++iroad) {
    const EMTFRoadExtra& road = roads.at(iroad);
    int izhit = road.key_zhit;

    // Center quality is the current one
    int qc = quality_codes.at(izhit);

    // Left and right qualities are the neighbors
    // Protect against the right end and left end special cases
    int ql = (izhit == (int) quality_codes.size()-1) ? 0 : quality_codes.at(izhit+1);
    int qr = (izhit == 0) ? 0 : quality_codes.at(izhit-1);

    // Cancellation conditions
    if (qc <= ql || qc < qr) {  // this pattern is lower quality than neighbors
      mask_roads.at(iroad) = false;  // cancel
    }
  }

  EMTFRoadExtraCollection survived_roads;

  for (unsigned iroad = 0; iroad < roads.size(); ++iroad) {
    // Output road
    if (mask_roads.at(iroad)) {
      survived_roads.push_back(roads.at(iroad));
    }
  }

  roads.swap(survived_roads);
}
