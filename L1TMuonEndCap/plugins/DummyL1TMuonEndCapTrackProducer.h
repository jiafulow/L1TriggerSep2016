#ifndef L1TMuonEndCap_DummyL1TMuonEndCapTrackProducerSep2016_h
#define L1TMuonEndCap_DummyL1TMuonEndCapTrackProducerSep2016_h

// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/L1TMuon/interface/RegionalMuonCand.h"
#include "DataFormats/L1TMuon/interface/RegionalMuonCandFwd.h"

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFCommon.hh"


// Class declaration
class DummyL1TMuonEndCapTrackProducerSep2016 : public edm::EDProducer {
public:
  explicit DummyL1TMuonEndCapTrackProducerSep2016(const edm::ParameterSet&);
  virtual ~DummyL1TMuonEndCapTrackProducerSep2016();

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
  const edm::ParameterSet& config_;

  const edm::EDGetToken tokenCSC_, tokenRPC_;

  int verbose_;

  bool useCSC_, useRPC_;
};

#endif
