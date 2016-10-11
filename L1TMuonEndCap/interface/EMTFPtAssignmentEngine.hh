#ifndef L1TMuonEndCap_EMTFPtAssignmentEngine_hh
#define L1TMuonEndCap_EMTFPtAssignmentEngine_hh

#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>
#include <array>

#include "L1TriggerSep2016/L1TMuonEndCap/interface/bdt/Forest.h"

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFCommon.hh"
#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPtLUTReader.hh"


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

  void configure_details();

  address_t calculate_address(const EMTFTrackExtra& track) const;

  float calculate_pt(const address_t& address);

  float calculate_pt_lut(const address_t& address);
  float calculate_pt_xml(const address_t& address);

private:
  std::vector<int> allowedModes_;
  std::array<Forest, 16> forests_;
  EMTFPtLUTReader ptlut_reader_;

  bool ok_;

  int verbose_;

  bool readPtLUTFile_, fixMode15HighPt_, fix9bDPhi_;
};

#endif
