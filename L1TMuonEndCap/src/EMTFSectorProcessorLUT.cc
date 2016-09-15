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

  std::string ph_th_lut_dir = "L1Trigger/L1TMuon/data/emtf_luts/" + ph_th_lut + "/";

  read_file(ph_th_lut_dir+"ph_init_neighbor.txt",      ph_init_neighbor_);
  read_file(ph_th_lut_dir+"ph_disp_neighbor.txt",      ph_disp_neighbor_);
  read_file(ph_th_lut_dir+"th_init_neighbor.txt",      th_init_neighbor_);
  read_file(ph_th_lut_dir+"th_disp_neighbor.txt",      th_disp_neighbor_);
  read_file(ph_th_lut_dir+"th_lut_st1_neighbor.txt",   th_lut_st1_neighbor_);
  read_file(ph_th_lut_dir+"th_lut_st234_neighbor.txt", th_lut_st234_neighbor_);
  read_file(ph_th_lut_dir+"th_corr_neighbor.txt",      th_corr_neighbor_);

  if (ph_init_neighbor_.size() != 12*5*16) {  // [sector_12][station_5][chamber_15,ME11a]
    throw cms::Exception("EMTFSectorProcessorLUT")
        << "Expected ph_init_neighbor_ to get " << 12*5*16 << " values, "
        << "got " << ph_init_neighbor_.size() << " values.";
  }

  if (ph_disp_neighbor_.size() != 12*61) {  // [sector_12][ME1sub1_15,ME1sub2_12,ME2_11,ME3_11,ME4_11,extra_1?]
    throw cms::Exception("EMTFSectorProcessorLUT")
        << "Expected ph_disp_neighbor_ to get " << 12*61 << " values, "
        << "got " << ph_disp_neighbor_.size() << " values.";
  }

  if (th_init_neighbor_.size() != 12*61) {  // [sector_12][ME1sub1_15,ME1sub2_12,ME2_11,ME3_11,ME4_11,extra_1?]
    throw cms::Exception("EMTFSectorProcessorLUT")
        << "Expected th_init_neighbor_ to get " << 12*61 << " values, "
        << "got " << th_init_neighbor_.size() << " values.";
  }

  if (th_disp_neighbor_.size() != 12*61) {  // [sector_12][ME1sub1_15,ME1sub2_12,ME2_11,ME3_11,ME4_11,extra_1?]
    throw cms::Exception("EMTFSectorProcessorLUT")
        << "Expected th_disp_neighbor_ to get " << 12*61 << " values, "
        << "got " << th_disp_neighbor_.size() << " values.";
  }

  if (th_lut_st1_neighbor_.size() != 2*12*16*64) {  // [subsector_2][sector_12][chamber_15,ME11a][wire]
    throw cms::Exception("EMTFSectorProcessorLUT")
        << "Expected th_lut_st1_neighbor_ to get " << 2*12*16*64 << " values, "
        << "got " << th_lut_st1_neighbor_.size() << " values.";
  }

  if (th_lut_st234_neighbor_.size() != 3*12*11*112) {  // [station_3][sector_12][chamber_11][wire]
    throw cms::Exception("EMTFSectorProcessorLUT")
        << "Expected th_lut_st234_neighbor_ to get " << 3*12*11*112 << " values, "
        << "got " << th_lut_st234_neighbor_.size() << " values.";
  }

  if (th_corr_neighbor_.size() != 2*12*4*96) {  // [subsector_2][sector_12][chamber_ME11_4][strip_wire]
    throw cms::Exception("EMTFSectorProcessorLUT")
        << "Expected th_corr_neighbor_ to get " << 2*12*4*96 << " values, "
        << "got " << th_corr_neighbor_.size() << " values.";
  }

  ph_patt_corr_ = {
    0, 0, -5, +5, -5, +5, -2, +2, -2, +2, 0
  };
  if (ph_patt_corr_.size() != 11) {
    throw cms::Exception("EMTFSectorProcessorLUT")
        << "Expected ph_patt_corr_ to get " << 11 << " values, "
        << "got " << ph_patt_corr_.size() << " values.";
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

  ok_ = true;
  return;
}

uint32_t EMTFSectorProcessorLUT::get_ph_init(int fw_endcap, int fw_sector, int fw_station, int fw_cscid, bool is_me11a) const {
  if (fw_station <= 1) {
    fw_station = (fw_cscid == 12) ? 0 : fw_station;  // if neighbor, else

    if (is_me11a) {
      fw_cscid = (fw_cscid == 12) ? 15 : fw_cscid + 9;  // if neighbor, else
    }
  }

  size_t index = fw_endcap * 6 + fw_sector;
  index = index * 5 + fw_station;
  index = index * 16 + fw_cscid;
  return ph_init_neighbor_.at(index);
}

uint32_t EMTFSectorProcessorLUT::get_ph_disp(int fw_endcap, int fw_sector, int fw_station, int fw_cscid, bool is_me11a) const {
  //FIXME: ME1/1a? ME1 substation?
  int x = fw_cscid;
  if (fw_station == 1) {
    x += 15;
  } else if (fw_station == 2) {
    x += 15+12;
  } else if (fw_station == 3) {
    x += 15+12+11;
  } else if (fw_station == 4) {
    x += 15+12+11+11;
  }

  size_t index = fw_endcap * 6 + fw_sector;
  index = index * 61 + x;
  return ph_disp_neighbor_.at(index);
}

uint32_t EMTFSectorProcessorLUT::get_th_init(int fw_endcap, int fw_sector, int fw_station, int fw_cscid, bool is_me11a) const {
  //FIXME: ME1/1a? ME1 substation?
  int x = fw_cscid;
  if (fw_station == 1) {
    x += 15;
  } else if (fw_station == 2) {
    x += 15+12;
  } else if (fw_station == 3) {
    x += 15+12+11;
  } else if (fw_station == 4) {
    x += 15+12+11+11;
  }

  size_t index = fw_endcap * 6 + fw_sector;
  index = index * 61 + x;
  return th_init_neighbor_.at(index);
}

uint32_t EMTFSectorProcessorLUT::get_th_disp(int fw_endcap, int fw_sector, int fw_station, int fw_cscid, bool is_me11a) const {
  //FIXME: ME1/1a? ME1 substation?
  int x = fw_cscid;
  if (fw_station == 1) {
    x += 15;
  } else if (fw_station == 2) {
    x += 15+12;
  } else if (fw_station == 3) {
    x += 15+12+11;
  } else if (fw_station == 4) {
    x += 15+12+11+11;
  }

  size_t index = fw_endcap * 6 + fw_sector;
  index = index * 61 + x;
  return th_disp_neighbor_.at(index);
}

uint32_t EMTFSectorProcessorLUT::get_th_lut(int fw_endcap, int fw_sector, int fw_station, int fw_cscid, int wire, bool is_me11a) const {
  if (fw_station <= 1) {
    fw_station = (fw_cscid == 12) ? 0 : fw_station;  // if neighbor, else

    if (is_me11a) {
      fw_cscid = (fw_cscid == 12) ? 15 : fw_cscid + 9;  // if neighbor, else
    }

    size_t index = fw_station;
    index = index * 12 + fw_endcap * 6 + fw_sector;
    index = index * 16 + fw_cscid;
    index = index * 64 + wire;
    return th_lut_st1_neighbor_.at(index);

  } else {
    size_t index = fw_station-2;
    index = index * 12 + fw_endcap * 6 + fw_sector;
    index = index * 11 + fw_cscid;
    index = index * 112 + wire;
    return th_lut_st234_neighbor_.at(index);
  }
}

uint32_t EMTFSectorProcessorLUT::get_th_corr_lut(int fw_endcap, int fw_sector, int fw_station, int fw_cscid, int strip_wire, bool is_me11a) const {
  if (fw_station <= 1 && (fw_cscid <= 3 || fw_cscid == 12)) {  // ME1/1
    fw_station = (fw_cscid == 12) ? 0 : fw_station;  // if neighbor, else

    int x = (fw_cscid == 12) ? 4 : fw_cscid;  // if neighbor, else

    size_t index = fw_station;
    index = index * 12 + fw_endcap * 6 + fw_sector;
    index = index * 4 + x;
    index = index * 96 + strip_wire;
    return th_corr_neighbor_.at(index);

  } else {
    return 0;
  }
}

int32_t EMTFSectorProcessorLUT::get_ph_patt_corr(int pattern) const {
  return ph_patt_corr_.at(pattern);
}

uint32_t EMTFSectorProcessorLUT::get_ph_zone_offset(int pcs_station, int pcs_chamber) const {
  size_t index = pcs_station * 9 + pcs_chamber;
  return ph_zone_offset_.at(index);
}

void EMTFSectorProcessorLUT::read_file(const std::string& filename, std::vector<uint32_t>& vec) {
  std::ifstream infile;
  infile.open(edm::FileInPath(filename).fullPath().c_str());

  int buf;
  while (infile >> buf) {
    buf = (buf == -999) ? 0 : buf;
    vec.push_back(buf);
  }

  infile.close();
}
