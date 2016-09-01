#ifndef L1TMuonEndCap_L1TMuonEndCapTrackProducer_h
#define L1TMuonEndCap_L1TMuonEndCapTrackProducer_h

// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

//#include "FWCore/Utilities/interface/Exception.h"
//#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFTrackFinder.hh"


namespace L1TMuonEndCap {

  class L1TMuonEndCapTrackProducer : public edm::EDProducer {
  public:
    explicit L1TMuonEndCapTrackProducer(const edm::ParameterSet&);
    ~L1TMuonEndCapTrackProducer();

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
    std::unique_ptr<EMTFTrackFinder> track_finder_;
  };

}  // namespace L1TMuonEndCap

#endif
