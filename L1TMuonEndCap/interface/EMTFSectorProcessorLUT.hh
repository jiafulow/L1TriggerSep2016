#ifndef L1TMuonEndCap_EMTFSectorProcessorLUT_hh
#define L1TMuonEndCap_EMTFSectorProcessorLUT_hh

#include <cstdint>
#include <string>
#include <vector>


class EMTFSectorProcessorLUT {
public:
  explicit EMTFSectorProcessorLUT();
  ~EMTFSectorProcessorLUT();

  void read(const std::string& ph_th_lut);

  uint32_t get_ph_init(int fw_endcap, int fw_sector, int fw_cscid, int x);

  uint32_t get_ph_disp(int fw_endcap, int fw_sector, int x);

  uint32_t get_th_init(int fw_endcap, int fw_sector, int x);

  uint32_t get_th_disp(int fw_endcap, int fw_sector, int x);

  uint32_t get_th_mem(int fw_endcap, int fw_sector, int fw_cscid, int x);

  uint32_t get_th_corr_mem(int fw_endcap, int fw_sector, int fw_cscid, int is_me11a, int x);

private:
  void read_file(const std::string& filename, std::vector<uint32_t>& vec);

  std::vector<uint32_t> ph_init_neighbor_;
  std::vector<uint32_t> ph_disp_neighbor_;
  std::vector<uint32_t> th_init_neighbor_;
  std::vector<uint32_t> th_disp_neighbor_;
  std::vector<uint32_t> th_lut_st1_neighbor_;
  std::vector<uint32_t> th_lut_st234_neighbor_;
  std::vector<uint32_t> th_corr_neighbor_;

  bool ok_;
};

#endif
