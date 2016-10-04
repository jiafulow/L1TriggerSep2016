#ifndef L1TMuonEndCap_EMTFPtAssignmentEngine_hh
#define L1TMuonEndCap_EMTFPtAssignmentEngine_hh

#include <cstdint>
#include <string>
#include <vector>
#include <array>

#include "L1TriggerSep2016/L1TMuonEndCap/interface/bdt/Forest.h"

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFCommon.hh"


class EMTFPtAssignmentEngine {
public:
  explicit EMTFPtAssignmentEngine();
  ~EMTFPtAssignmentEngine();

  typedef uint64_t address_t;

  void read(const std::string& xml_dir);

  void configure(
      int verbose,
      bool readPtLUTFile, bool fixMode15HighPt, bool fix9bDPhi
  );

  address_t calculate_address(const EMTFTrackExtra& track) const;

  address_t calculate_address_fw(const EMTFTrackExtra& track) const;

  float calculate_pt(const address_t& address);

private:
  std::vector<int> allowedModes_;
  std::array<Forest, 16> forests_;

  bool ok_;

  int verbose_;

  bool readPtLUTFile_, fixMode15HighPt_, fix9bDPhi_;
};

#endif
