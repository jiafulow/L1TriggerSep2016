#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPatternRecognition.hh"

#include "helper.h"  // to_hex, to_binary

#define NUM_PATTERNS 9
#define PATTERN_KEY_ZHIT 7
#define PATTERN_PADDING_EXTRA_W_ST1 15-7

namespace {
  // See http://stackoverflow.com/a/53878
  std::vector<std::string> split_string(const std::string& s, char c = ' ', char d = ' ') {
    std::vector<std::string> result;
    const char* str = s.c_str();
    do {
      const char* begin = str;
      while(*str != c && *str != d && *str)
        str++;
      result.push_back(std::string(begin, str));
    } while (0 != *str++);
    return result;
  }
}  // namespace


void EMTFPatternRecognition::configure(
    int verbose, int endcap, int sector, int bx,
    int minBX, int maxBX, int bxWindow,
    const std::vector<std::string>& pattDefinitions, int maxRoadsPerZone
) {
  verbose_ = verbose;
  endcap_  = endcap;
  sector_  = sector;
  bx_      = bx;

  minBX_           = minBX;
  maxBX_           = maxBX;
  bxWindow_        = bxWindow;

  pattDefinitions_ = pattDefinitions;
  maxRoadsPerZone_ = maxRoadsPerZone;

  configure_details();
}

void EMTFPatternRecognition::configure_details() {
  patterns_.clear();
  straightnesses_.clear();

  // Parse pattern definitions
  for (const auto& s: pattDefinitions_) {
    const std::vector<std::string>& tokens = split_string(s, ',', ':');  // split by comma or colon
    assert(tokens.size() == 9);  // want to find 9 numbers
    int itoken = 0;

    // Get the 9 integers
    // straightness, hits in ME1, hits in ME2, hits in ME3, hits in ME4
    int straightness = std::stoi(tokens.at(itoken++));
    int st1_max      = std::stoi(tokens.at(itoken++));
    int st1_min      = std::stoi(tokens.at(itoken++));
    int st2_max      = std::stoi(tokens.at(itoken++));
    int st2_min      = std::stoi(tokens.at(itoken++));
    int st3_max      = std::stoi(tokens.at(itoken++));
    int st3_min      = std::stoi(tokens.at(itoken++));
    int st4_max      = std::stoi(tokens.at(itoken++));
    int st4_min      = std::stoi(tokens.at(itoken++));

    // There can only be one zone hit in the key station in the pattern
    // and it has to be this magic number
    assert(st2_min == PATTERN_KEY_ZHIT && st2_max == PATTERN_KEY_ZHIT);

    // There is extra "padding" in st1 w.r.t st2,3,4
    // Add the extra padding to st2,3,4
    st2_max += PATTERN_PADDING_EXTRA_W_ST1;
    st2_min += PATTERN_PADDING_EXTRA_W_ST1;
    st3_max += PATTERN_PADDING_EXTRA_W_ST1;
    st3_min += PATTERN_PADDING_EXTRA_W_ST1;
    st4_max += PATTERN_PADDING_EXTRA_W_ST1;
    st4_min += PATTERN_PADDING_EXTRA_W_ST1;

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

    // Remove the extra padding
    pattern.rotr(PATTERN_PADDING_EXTRA_W_ST1);

    if (verbose_ > 1) {  // debug
      std::cout << "Pattern definition: " << straightness << " "
          << st4_min << " " << st4_max << " "
          << st3_min << " " << st3_max << " "
          << st2_min << " " << st2_max << " "
          << st1_min << " " << st1_max << std::endl;
      std::cout << pattern << std::endl;
    }

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
}

void EMTFPatternRecognition::process(
    const std::deque<EMTFHitExtraCollection>& extended_conv_hits,
    std::map<pattern_ref_t, int>& patt_lifetime_map,
    std::vector<EMTFRoadExtraCollection>& zone_roads
) const {

  if (verbose_ > 0) {  // debug
    for (const auto& conv_hits : extended_conv_hits) {
      for (const auto& conv_hit : conv_hits) {
        std::cout << "st: " << conv_hit.pc_station << " ch: " << conv_hit.pc_chamber
            << " ph: " << conv_hit.phi_fp << " th: " << conv_hit.theta_fp
            << " ph_hit: " << (1ul<<conv_hit.ph_hit) << " phzvl: " << conv_hit.phzvl
            << " zone_hit: " << conv_hit.zone_hit << " zone_code: " << conv_hit.zone_code
            << std::endl;
      }
    }
  }

  // Make zone images
  std::array<EMTFPhiMemoryImage, NUM_ZONES> zone_images;

  for (int izone = 0; izone < NUM_ZONES; ++izone) {
    make_zone_image(izone, extended_conv_hits, zone_images.at(izone));
  }

  if (verbose_ > 1) {  // debug
    for (int izone = NUM_ZONES; izone >= 1; --izone) {
      std::cout << "zone: " << izone << std::endl;
      std::cout << zone_images.at(izone-1) << std::endl;
    }
  }

  // Perform pattern recognition in each zone
  zone_roads.clear();
  zone_roads.resize(NUM_ZONES);

  for (int izone = 0; izone < NUM_ZONES; ++izone) {
    process_single_zone(izone, zone_images.at(izone), patt_lifetime_map, zone_roads.at(izone));
  }

  if (verbose_ > 0) {  // debug
    for (const auto& roads : zone_roads) {
      for (const auto& road : roads) {
        std::cout << "pattern: z: " << road.zone << " ph: " << road.key_zhit << " q: " << to_hex(road.quality_code) << " ly: " << to_binary(road.layer_code, 3) << " str: " << to_binary(road.straightness, 3) << std::endl;
      }
    }
  }

  // Sort patterns and select best three patterns in each zone
  for (int izone = 0; izone < NUM_ZONES; ++izone) {
    sort_single_zone(zone_roads.at(izone));
  }

  if (verbose_ > 0) {  // debug
    for (const auto& roads : zone_roads) {
      for (const auto& road : roads) {
        std::cout << "z: " << road.zone << " r: " << road.winner << " ph_num: " << road.ph_num << " ph_q: " << to_hex(road.quality_code) << " ly: " << to_binary(road.layer_code, 3) << " str: " << to_binary(road.straightness, 3) << std::endl;
      }
    }
  }

}

void EMTFPatternRecognition::make_zone_image(
    int zone,
    const std::deque<EMTFHitExtraCollection>& extended_conv_hits,
    EMTFPhiMemoryImage& image
) const {
  // Loop over converted hits and fill the zone image
  std::deque<EMTFHitExtraCollection>::const_iterator ext_conv_hits_it  = extended_conv_hits.begin();
  std::deque<EMTFHitExtraCollection>::const_iterator ext_conv_hits_end = extended_conv_hits.end();

  for (; ext_conv_hits_it != ext_conv_hits_end; ++ext_conv_hits_it) {
    EMTFHitExtraCollection::const_iterator conv_hits_it  = ext_conv_hits_it->begin();
    EMTFHitExtraCollection::const_iterator conv_hits_end = ext_conv_hits_it->end();

    for (; conv_hits_it != conv_hits_end; ++conv_hits_it) {
      const EMTFHitExtra& conv_hit = *conv_hits_it;

      if (conv_hit.zone_code & (1<<zone)) {  // hit belongs to this zone
        unsigned int layer = conv_hit.station - 1;
        unsigned int bit   = conv_hit.zone_hit;
        image.set_bit(layer, bit);
      }
    }  // end loop over conv_hits
  }  // end loop over extended_conv_hits
}

void EMTFPatternRecognition::process_single_zone(
    int zone,
    EMTFPhiMemoryImage cloned_image,
    std::map<pattern_ref_t, int>& patt_lifetime_map,
    EMTFRoadExtraCollection& roads
) const {
  roads.clear();

  const int drift_time = bxWindow_ - 1;

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
      const pattern_ref_t patt_ref = {{zone, izhit, ipatt}};  // due to GCC bug, use {{}} instead of {}

      bool is_lifetime_up = false;

      int layer_code = patt.op_and(cloned_image);  // kind of like AND operator

      if (layer_code > 0) {
        // Starts counting at any hit in the pattern, even single
        auto ins = patt_lifetime_map.insert({patt_ref, 1});

        if (!ins.second) {  // already exists, increment counter
          // Is lifetime up?
          if (ins.first->second == drift_time) {
            is_lifetime_up = true;
          }

          ins.first->second += 1;
        }

      } else {
        patt_lifetime_map.erase(patt_ref);  // erase if exists
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

        // Create a road (fired pattern)
        EMTFRoadExtra road;
        road.endcap   = endcap_;
        road.sector   = sector_;
        road.bx       = bx_ - drift_time;

        road.zone     = patt_ref.at(0);
        road.key_zhit = patt_ref.at(1);
        road.pattern  = patt_ref.at(2);

        road.straightness = straightness;
        road.layer_code   = layer_code;
        road.quality_code = quality_code;

        road.ph_q     = road.quality_code;
        road.ph_num   = road.key_zhit;

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

void EMTFPatternRecognition::sort_single_zone(EMTFRoadExtraCollection& roads) const {
  // First, order by key_zhit
  struct {
    typedef EMTFRoadExtra value_type;
    constexpr bool operator()(const value_type& lhs, const value_type& rhs) {
      return lhs.key_zhit > rhs.key_zhit;
    }
  } greater_zhit_cmp;

  std::sort(roads.begin(), roads.end(), greater_zhit_cmp);

  // Second, sort by quality_code, but preserving the original order
  struct {
    typedef EMTFRoadExtra value_type;
    constexpr bool operator()(const value_type& lhs, const value_type& rhs) {
      return lhs.quality_code > rhs.quality_code;
    }
  } greater_quality_cmp;

  std::stable_sort(roads.begin(), roads.end(), greater_quality_cmp);

  // Finally, select 3 best
  const size_t n = maxRoadsPerZone_;
  if (roads.size() > n) {
    roads.erase(roads.begin() + n, roads.end());
  }
  assert(roads.size() <= n);

  // Assign the winner variable
  const int nroads = roads.size();
  for (int iroad = 0; iroad < nroads; ++iroad) {
    roads.at(iroad).winner = iroad;
  }
}
