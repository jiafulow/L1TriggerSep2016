#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFSectorProcessorLUT.hh"

#include <cassert>
#include <iostream>
#include <fstream>

#include "FWCore/ParameterSet/interface/FileInPath.h"
#include "FWCore/Utilities/interface/Exception.h"


EMTFSectorProcessorLUT::EMTFSectorProcessorLUT() :
    ok_(false)
{

}

EMTFSectorProcessorLUT::~EMTFSectorProcessorLUT() {

}

void EMTFSectorProcessorLUT::read(const std::string& ph_th_lut) {
  if (ok_)  return;

  //std::string ph_th_lut_dir = "L1Trigger/L1TMuon/data/emtf_luts/" + ph_th_lut + "/";
  std::string ph_th_lut_dir = "L1TriggerSep2016/L1TMuonEndCap/data/emtf_luts/" + ph_th_lut + "/";

  read_file(ph_th_lut_dir+"ph_init_neighbor.txt",     ph_init_neighbor_);
  read_file(ph_th_lut_dir+"ph_disp_neighbor.txt",     ph_disp_neighbor_);
  read_file(ph_th_lut_dir+"th_init_neighbor.txt",     th_init_neighbor_);
  read_file(ph_th_lut_dir+"th_disp_neighbor.txt",     th_disp_neighbor_);
  read_file(ph_th_lut_dir+"th_lut_neighbor.txt",      th_lut_neighbor_);
  read_file(ph_th_lut_dir+"th_corr_lut_neighbor.txt", th_corr_lut_neighbor_);

  if (ph_init_neighbor_.size() != 2*6*61) {  // [endcap_2][sector_6][chamber_61]
    throw cms::Exception("EMTFSectorProcessorLUT")
        << "Expected ph_init_neighbor_ to get " << 2*6*61 << " values, "
        << "got " << ph_init_neighbor_.size() << " values.";
  }

  if (ph_disp_neighbor_.size() != 2*6*61) {  // [endcap_2][sector_6][chamber_61]
    throw cms::Exception("EMTFSectorProcessorLUT")
        << "Expected ph_disp_neighbor_ to get " << 2*6*61 << " values, "
        << "got " << ph_disp_neighbor_.size() << " values.";
  }

  if (th_init_neighbor_.size() != 2*6*61) {  // [endcap_2][sector_6][chamber_61]
    throw cms::Exception("EMTFSectorProcessorLUT")
        << "Expected th_init_neighbor_ to get " << 2*6*61 << " values, "
        << "got " << th_init_neighbor_.size() << " values.";
  }

  if (th_disp_neighbor_.size() != 2*6*61) {  // [endcap_2][sector_6][chamber_61]
    throw cms::Exception("EMTFSectorProcessorLUT")
        << "Expected th_disp_neighbor_ to get " << 2*6*61 << " values, "
        << "got " << th_disp_neighbor_.size() << " values.";
  }

  if (th_lut_neighbor_.size() != 2*6*61*128) {  // [endcap_2][sector_6][chamber_61][wire_128]
    throw cms::Exception("EMTFSectorProcessorLUT")
        << "Expected th_lut_neighbor_ to get " << 2*6*61*128 << " values, "
        << "got " << th_lut_neighbor_.size() << " values.";
  }

  if (th_corr_lut_neighbor_.size() != 2*6*7*128) {  // [endcap_2][sector_6][chamber_61][strip_wire_128]
    throw cms::Exception("EMTFSectorProcessorLUT")
        << "Expected th_corr_lut_neighbor_ to get " << 2*6*7*128 << " values, "
        << "got " << th_corr_lut_neighbor_.size() << " values.";
  }

  // clct pattern convertion array from CMSSW
  //{0.0, 0.0, -0.60,  0.60, -0.64,  0.64, -0.23,  0.23, -0.21,  0.21, 0.0}
  // 0    0    -5      +5    -5      +5    -2      +2    -2      +2    0
  ph_patt_corr_ = {
    0, 0, 5, 5, 5, 5, 2, 2, 2, 2, 0
  };
  if (ph_patt_corr_.size() != 11) {
    throw cms::Exception("EMTFSectorProcessorLUT")
        << "Expected ph_patt_corr_ to get " << 11 << " values, "
        << "got " << ph_patt_corr_.size() << " values.";
  }

  ph_patt_corr_sign_ = {
    0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0
  };
  if (ph_patt_corr_sign_.size() != 11) {
    throw cms::Exception("EMTFSectorProcessorLUT")
        << "Expected ph_patt_corr_sign_ to get " << 11 << " values, "
        << "got " << ph_patt_corr_sign_.size() << " values.";
  }

  ph_zone_offset_ = {
    39,57,76,39,58,76,41,60,79,
    95,114,132,95,114,133,98,116,135,
    38,76,113,39,58,76,95,114,132,
    38,76,113,39,58,76,95,114,132,
    38,76,113,38,57,76,95,113,132,
    21,21,23,1,21,1,21,1,20
  };
  if (ph_zone_offset_.size() != 6*9) {
    throw cms::Exception("EMTFSectorProcessorLUT")
        << "Expected ph_zone_offset_ to get " << 6*9 << " values, "
        << "got " << ph_zone_offset_.size() << " values.";
  }

  // start phi of each chamber in reduced precision, for zone hits,
  // with negative offset to allow for possible chamber movement
  ph_init_hard_ = {
    39,  57,  76, 39,  58,  76, 41,  60,  79, 39,  57,  76,  0,  0,  0,  0,
    95, 114, 132, 95, 114, 133, 98, 116, 135, 95, 114, 132, 21, 21, 23, 21,
    38,  76, 113, 39,  58,  76, 95, 114, 132,  1,  21,   0,  0,  0,  0,  0,
    38,  76, 113, 39,  58,  76, 95, 114, 132,  1,  21,   0,  0,  0,  0,  0,
    38,  76, 113, 38,  57,  76, 95, 113, 132,  1,  20,   0,  0,  0,  0,  0
  };
  if (ph_init_hard_.size() != 5*16) {
    throw cms::Exception("EMTFSectorProcessorLUT")
        << "Expected ph_init_hard_ to get " << 5*16 << " values, "
        << "got " << ph_init_hard_.size() << " values.";
  }

  ok_ = true;
  return;
}

uint32_t EMTFSectorProcessorLUT::get_ph_init(int fw_endcap, int fw_sector, int pc_lut_id) const {
  size_t index = (fw_endcap * 6 + fw_sector) * 61 + pc_lut_id;
  return ph_init_neighbor_.at(index);
}

uint32_t EMTFSectorProcessorLUT::get_ph_disp(int fw_endcap, int fw_sector, int pc_lut_id) const {
  size_t index = (fw_endcap * 6 + fw_sector) * 61 + pc_lut_id;
  return ph_disp_neighbor_.at(index);
}

uint32_t EMTFSectorProcessorLUT::get_th_init(int fw_endcap, int fw_sector, int pc_lut_id) const {
  size_t index = (fw_endcap * 6 + fw_sector) * 61 + pc_lut_id;
  return th_init_neighbor_.at(index);
}

uint32_t EMTFSectorProcessorLUT::get_th_disp(int fw_endcap, int fw_sector, int pc_lut_id) const {
  size_t index = (fw_endcap * 6 + fw_sector) * 61 + pc_lut_id;
  return th_disp_neighbor_.at(index);
}

uint32_t EMTFSectorProcessorLUT::get_th_lut(int fw_endcap, int fw_sector, int pc_lut_id, int pc_wire_id) const {
  size_t index = ((fw_endcap * 6 + fw_sector) * 61 + pc_lut_id) * 128 + pc_wire_id;
  return th_lut_neighbor_.at(index);
}

uint32_t EMTFSectorProcessorLUT::get_th_corr_lut(int fw_endcap, int fw_sector, int pc_lut_id, int pc_wire_strip_id) const {
  int pc_lut_id2 = pc_lut_id;

  // Make ME1/1a the same as ME1/1b
  if ((9 <= pc_lut_id2 && pc_lut_id2 < 12) || (25 <= pc_lut_id2 && pc_lut_id2 < 28))
    pc_lut_id2 -= 9;
  // Make ME1/1a neighbor the same as ME1/1b
  if (pc_lut_id2 == 15)
    pc_lut_id2 -= 3;

  if (pc_lut_id2 <= 3) {
    pc_lut_id2 -= 0;
  } else if (pc_lut_id2 == 12) {
    pc_lut_id2 -= 9;
  } else if (16 <= pc_lut_id2 && pc_lut_id2 < 19) {
    pc_lut_id2 -= 12;
  } else {
    throw cms::Exception("EMTFSectorProcessorLUT")
      << "get_th_corr_lut(): out of range pc_lut_id: " << pc_lut_id;
  }

  size_t index = ((fw_endcap * 6 + fw_sector) * 7 + pc_lut_id2) * 128 + pc_wire_strip_id;
  return th_corr_lut_neighbor_.at(index);
}

uint32_t EMTFSectorProcessorLUT::get_ph_patt_corr(int pattern) const {
  return ph_patt_corr_.at(pattern);
}

uint32_t EMTFSectorProcessorLUT::get_ph_patt_corr_sign(int pattern) const {
  return ph_patt_corr_sign_.at(pattern);
}

uint32_t EMTFSectorProcessorLUT::get_ph_zone_offset(int pc_station, int pc_chamber) const {
  size_t index = pc_station * 9 + pc_chamber;
  return ph_zone_offset_.at(index);
}

uint32_t EMTFSectorProcessorLUT::get_ph_init_hard(int fw_station, int fw_cscid) const {
  size_t index = fw_station * 16 + fw_cscid;
  return ph_init_hard_.at(index);
}

void EMTFSectorProcessorLUT::read_file(const std::string& filename, std::vector<uint32_t>& vec) {
  vec.clear();

  std::ifstream infile;
  infile.open(edm::FileInPath(filename).fullPath().c_str());

  int buf;
  while (infile >> buf) {
    buf = (buf == -999) ? 0 : buf;
    vec.push_back(buf);
  }
  infile.close();
}
