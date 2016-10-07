#include <map>
#include <memory>
#include <iostream>

#include "TH1F.h"
#include "TFile.h"

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPtLUTReader.hh"

#include "helper.hh"


class CompareEMTFPtLUT : public edm::EDAnalyzer {
public:
  explicit CompareEMTFPtLUT(const edm::ParameterSet&);
  virtual ~CompareEMTFPtLUT();

private:
  //virtual void beginJob();
  //virtual void endJob();

  //virtual void beginRun(const edm::Run&, const edm::EventSetup&);
  //virtual void endRun(const edm::Run&, const edm::EventSetup&);

  virtual void analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup);

  void compareLUTs();

private:
  EMTFPtLUTReader ptlut_reader1_;
  EMTFPtLUTReader ptlut_reader2_;

  bool done_;
};

// _____________________________________________________________________________
#define PTLUT_SIZE (1<<30)

CompareEMTFPtLUT::CompareEMTFPtLUT(const edm::ParameterSet& iConfig) :
    ptlut_reader1_(),
    ptlut_reader2_(),
    done_(false)
{
  std::stringstream ss;
  ss << std::getenv("CMSSW_BASE") << "/" << "src/L1TriggerSep2016/L1TMuonEndCap/data/emtf_luts/v_16_02_21_ptlut/LUT_AndrewFix_25July16.dat";  // hardcoded, it does not exist in CMSSW
  std::string lut_full_path1 = ss.str();

  ss.str("");
  //ss << std::getenv("CMSSW_BASE") << "/" << "src/L1TriggerSep2016/L1TMuonEndCap/data/emtf_luts/v_16_02_21_ptlut_madorsky/LUT_AndrewFix_25July16.dat";  // hardcoded, it does not exist in CMSSW
  ss << std::getenv("CMSSW_BASE") << "/" << "src/L1TriggerSep2016/L1TMuonEndCap/data/emtf_luts/v_16_02_21_ptlut_jftest/LUT_AndrewFix_25July16.dat";  // hardcoded, it does not exist in CMSSW
  std::string lut_full_path2 = ss.str();

  ptlut_reader1_.read(lut_full_path1);
  ptlut_reader2_.read(lut_full_path2);
}

CompareEMTFPtLUT::~CompareEMTFPtLUT() {}

void CompareEMTFPtLUT::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  if (done_)  return;

  compareLUTs();

  done_ = true;
  return;
}

void CompareEMTFPtLUT::compareLUTs() {
  TFile* f = TFile::Open("diff.root", "RECREATE");

  std::map<int, TH1F*> histograms;
  for (int mode_inv=0; mode_inv<16; ++mode_inv) {
    histograms[mode_inv] = new TH1F(Form("diff_mode_%i", mode_inv), "", 200+1, -100-0.5, 100+0.5);
  }

  EMTFPtLUTReader::address_t address = 0;
  //EMTFPtLUTReader::content_t pt_value1 = 0;
  //EMTFPtLUTReader::content_t pt_value2 = 0;

  for (; address<PTLUT_SIZE; ++address) {
    show_progress_bar(address, PTLUT_SIZE);

    int mode_inv = (address >> (30-4)) & ((1<<4)-1);

    int pt_value1 = ptlut_reader1_.lookup(address);
    int pt_value2 = ptlut_reader2_.lookup(address);

    int diff = pt_value1 - pt_value2;
    diff = std::min(std::max(-100, diff), 100);

    //if (address % (1<<20) == 0)
    //  std::cout << mode_inv << " " << address << " " << pt_value1 << " " << pt_value2 << " " << diff << std::endl;

    histograms[mode_inv]->Fill(diff);
  }

  for (int mode_inv=0; mode_inv<16; ++mode_inv) {
    histograms[mode_inv]->Write();
  }
  f->Close();
}

// DEFINE THIS AS A PLUG-IN
#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(CompareEMTFPtLUT);
