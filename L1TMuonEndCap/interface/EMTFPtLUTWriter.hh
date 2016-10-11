#ifndef L1TMuonEndCap_EMTFPtLUTWriter_hh
#define L1TMuonEndCap_EMTFPtLUTWriter_hh

#include <cstdint>
#include <string>
#include <vector>


class EMTFPtLUTWriter {
public:
  explicit EMTFPtLUTWriter();
  ~EMTFPtLUTWriter();

  typedef uint16_t               content_t;
  typedef uint64_t               address_t;
  typedef std::vector<content_t> table_t;

  void write(const std::string& lut_full_path) const;

  void push_back(const content_t& pt);

private:
  table_t ptlut_;
  bool ok_;
};

#endif
