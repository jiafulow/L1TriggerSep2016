#ifndef L1TMuonEndCap_EMTFPtLUTReader_hh
#define L1TMuonEndCap_EMTFPtLUTReader_hh

#include <cstdint>
#include <string>
#include <vector>


class EMTFPtLUTReader {
public:
  explicit EMTFPtLUTReader();
  ~EMTFPtLUTReader();

  typedef uint16_t               content_t;
  typedef uint64_t               address_t;
  typedef std::vector<content_t> table_t;

  void read(const std::string& lut_full_path);

  content_t lookup(const address_t& address) const;

private:
  table_t ptlut_;
  bool ok_;
};

#endif
