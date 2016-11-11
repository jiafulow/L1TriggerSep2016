#ifndef L1TMuonEndCap_L1TMuonEndCapTrackProducerSep2016_h
#define L1TMuonEndCap_L1TMuonEndCapTrackProducerSep2016_h

// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFTrackFinder.hh"
#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFTrackAdaptor.hh"
#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFMicroGMTConverter.hh"


// Class declaration
class L1TMuonEndCapTrackProducerSep2016 : public edm::EDProducer {
public:
  explicit L1TMuonEndCapTrackProducerSep2016(const edm::ParameterSet&);
  virtual ~L1TMuonEndCapTrackProducerSep2016();

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  virtual void beginJob() override;
  virtual void produce(edm::Event&, const edm::EventSetup&) override;
  virtual void endJob() override;

  //virtual void beginRun(edm::Run const&, edm::EventSetup const&);
  //virtual void endRun(edm::Run const&, edm::EventSetup const&);
  //virtual void beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&);
  //virtual void endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&);

private:
  std::unique_ptr<EMTFTrackFinder>       track_finder_;
  std::unique_ptr<EMTFTrackAdaptor>      track_adaptor_;
  std::unique_ptr<EMTFMicroGMTConverter> uGMT_converter_;

  const edm::ParameterSet& config_;
};

#endif
