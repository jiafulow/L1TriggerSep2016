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

  if (ph_init_neighbor_.size() != 12*5*16) {
    throw cms::Exception("EMTFSectorProcessorLUT")
        << "Expected ph_init_neighbor_ to get " << 12*5*16 << " values, "
        << "got " << ph_init_neighbor_.size() << " values.";
  }

  if (ph_disp_neighbor_.size() != 12*61) {
    throw cms::Exception("EMTFSectorProcessorLUT")
        << "Expected ph_disp_neighbor_ to get " << 12*61 << " values, "
        << "got " << ph_disp_neighbor_.size() << " values.";
  }

  if (th_init_neighbor_.size() != 12*61) {
    throw cms::Exception("EMTFSectorProcessorLUT")
        << "Expected th_init_neighbor_ to get " << 12*61 << " values, "
        << "got " << th_init_neighbor_.size() << " values.";
  }

  if (th_disp_neighbor_.size() != 12*61) {
    throw cms::Exception("EMTFSectorProcessorLUT")
        << "Expected th_disp_neighbor_ to get " << 12*61 << " values, "
        << "got " << th_disp_neighbor_.size() << " values.";
  }

  if (th_lut_st1_neighbor_.size() != 2*12*16*64) {
    throw cms::Exception("EMTFSectorProcessorLUT")
        << "Expected th_lut_st1_neighbor_ to get " << 2*12*16*64 << " values, "
        << "got " << th_lut_st1_neighbor_.size() << " values.";
  }

  if (th_lut_st234_neighbor_.size() != 3*12*11*112) {
    throw cms::Exception("EMTFSectorProcessorLUT")
        << "Expected th_lut_st234_neighbor_ to get " << 3*12*11*112 << " values, "
        << "got " << th_lut_st234_neighbor_.size() << " values.";
  }

  if (th_corr_neighbor_.size() != 2*12*4*96) {
    throw cms::Exception("EMTFSectorProcessorLUT")
        << "Expected th_corr_neighbor_ to get " << 2*12*4*96 << " values, "
        << "got " << th_corr_neighbor_.size() << " values.";
  }

  ok_ = true;
  return;
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
