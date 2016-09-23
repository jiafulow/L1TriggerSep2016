#ifndef L1TMuonEndCap_EMTFPtAssignmentEngine_hh
#define L1TMuonEndCap_EMTFPtAssignmentEngine_hh

#include <cstdint>
#include <string>
#include <vector>

#include "L1TriggerSep2016/L1TMuonEndCap/interface/bdt/Forest.h"

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFCommon.hh"


class EMTFPtAssignmentEngine {
public:
  explicit EMTFPtAssignmentEngine();
  ~EMTFPtAssignmentEngine();

  typedef uint64_t address_t;

  void read(const std::string& tree_ver);

  address_t calculate_address(const EMTFTrackExtra& track) const;

  address_t calculate_address_fw(const EMTFTrackExtra& track) const;

  float calculate_pt(const address_t& address, const EMTFTrackExtra& track);

private:
  std::vector<int> allowedModes_;
  Forest forest_[16];

  bool ok_;
};

#endif
