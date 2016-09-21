#ifndef L1TMuonEndCap_EMTFPtAssignmentEngine_hh
#define L1TMuonEndCap_EMTFPtAssignmentEngine_hh

#include <cstdint>
#include <string>
#include <vector>


class EMTFPtAssignmentEngine {
public:
  explicit EMTFPtAssignmentEngine();
  ~EMTFPtAssignmentEngine();

  typedef uint64_t address_t;

  address_t calculate_address() const;

  address_t calculate_address_fw() const;

  float calculate_pt(address_t address) const;

private:
  //std::vector<int> allowedModes_;
  //Forest forest_[16];

  bool ok_;
};

#endif
