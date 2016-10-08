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

  const edm::ParameterSet config_;

  int verbose_;

  std::string infile1_;
  std::string infile2_;

  bool done_;
};

// _____________________________________________________________________________
#define PTLUT_SIZE (1<<30)

CompareEMTFPtLUT::CompareEMTFPtLUT(const edm::ParameterSet& iConfig) :
    ptlut_reader1_(),
    ptlut_reader2_(),
    config_(iConfig),
    verbose_(iConfig.getUntrackedParameter<int>("verbosity")),
    infile1_(iConfig.getParameter<std::string>("infile1")),
    infile2_(iConfig.getParameter<std::string>("infile2")),
    done_(false)
{
  ptlut_reader1_.read(infile1_);
  ptlut_reader2_.read(infile2_);
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
  int diff_limit = 20;
  for (int mode_inv=0; mode_inv<16; ++mode_inv) {
    histograms[mode_inv] = new TH1F(Form("diff_mode_inv_%i", mode_inv), "", 2*diff_limit+1, -1.0*diff_limit-0.5, 1.0*diff_limit+0.5);
  }

  EMTFPtLUTReader::address_t address = 0;
  //EMTFPtLUTReader::content_t pt_value1 = 0;
  //EMTFPtLUTReader::content_t pt_value2 = 0;

  for (; address<PTLUT_SIZE; ++address) {
    show_progress_bar(address, PTLUT_SIZE);

    int mode_inv = (address >> (30-4)) & ((1<<4)-1);

    int pt_value1 = ptlut_reader1_.lookup(address);
    int pt_value2 = ptlut_reader2_.lookup(address);

    int diff = pt_value2 - pt_value1;
    diff = std::min(std::max(-diff_limit, diff), diff_limit);

    //if (address % (1<<20) == 0)
    //  std::cout << mode_inv << " " << address << " " << pt_value1 << " " << pt_value2 << " " << pt_value2 - pt_value1 << std::endl;

    if (std::abs(pt_value2 - pt_value1) > diff_limit)
      std::cout << mode_inv << " " << address << " " << pt_value1 << " " << pt_value2 << " " << pt_value2 - pt_value1 << std::endl;

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
