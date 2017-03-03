#include "DummyL1TMuonEndCapTrackProducer.h"


DummyL1TMuonEndCapTrackProducerSep2016::DummyL1TMuonEndCapTrackProducerSep2016(const edm::ParameterSet& iConfig) :
    config_(iConfig),
    tokenCSC_(consumes<CSCTag::digi_collection>(iConfig.getParameter<edm::InputTag>("CSCInput"))),
    tokenRPC_(consumes<RPCTag::digi_collection>(iConfig.getParameter<edm::InputTag>("RPCInput"))),
    verbose_(iConfig.getUntrackedParameter<int>("verbosity")),
    useCSC_(iConfig.getParameter<bool>("CSCEnable")),
    useRPC_(iConfig.getParameter<bool>("RPCEnable"))
{
  // Make output products
  produces<EMTFHitExtraCollection>           ("");      // Same as EMTFHit, but with extra emulator-only variables
  produces<EMTFTrackExtraCollection>         ("");      // Same as EMTFTrack, but with extra emulator-only variables
  produces<l1t::RegionalMuonCandBxCollection>("EMTF");  // EMTF tracks output to uGMT

  produces<EMTFHitCollection>                ("");      // All CSC LCTs and RPC clusters received by EMTF

  produces<EMTFTrackCollection>              ("");      // All output EMTF tracks, in same format as unpacked data
}

DummyL1TMuonEndCapTrackProducerSep2016::~DummyL1TMuonEndCapTrackProducerSep2016() {

}

void DummyL1TMuonEndCapTrackProducerSep2016::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  // Create pointers to output products
  auto out_xhits   = std::make_unique<EMTFHitExtraCollection>();
  auto out_xtracks = std::make_unique<EMTFTrackExtraCollection>();
  auto out_cands   = std::make_unique<l1t::RegionalMuonCandBxCollection>();

  auto out_hits    = std::make_unique<EMTFHitCollection>();
  auto out_tracks  = std::make_unique<EMTFTrackCollection>();


  // Get input colections
  edm::Handle<CSCTag::digi_collection> cscDigis;
  if (useCSC_)
    iEvent.getByToken(tokenCSC_, cscDigis);

  edm::Handle<RPCTag::digi_collection> rpcDigis;
  if (useRPC_)
    iEvent.getByToken(tokenRPC_, rpcDigis);

  // Do stuff
  std::vector<int> dummy_vector;

  auto chamber = cscDigis->begin();
  auto chend   = cscDigis->end();
  for( ; chamber != chend; ++chamber ) {
    auto digi = (*chamber).second.first;
    auto dend = (*chamber).second.second;
    for( ; digi != dend; ++digi ) {
      // Do useless things with the digis
      int dummy = digi->getStrip() * digi->getKeyWG();
      dummy_vector.push_back(dummy);
    }
  }

  for (auto dummy : dummy_vector) {
    // Do more useless things
    int dummy2 = dummy * dummy;

    if (verbose_ > 0) {
      std::cout << "I am a dummy: " << dummy2 << std::endl;
    }
  }


  // Fill the output products
  iEvent.put(std::move(out_xhits)  , "");
  iEvent.put(std::move(out_xtracks), "");
  iEvent.put(std::move(out_cands)  , "EMTF");

  iEvent.put(std::move(out_hits)   , "");
  iEvent.put(std::move(out_tracks) , "");
}

void DummyL1TMuonEndCapTrackProducerSep2016::beginJob() {

}

void DummyL1TMuonEndCapTrackProducerSep2016::endJob() {

}

// Fill 'descriptions' with the allowed parameters
void DummyL1TMuonEndCapTrackProducerSep2016::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  // The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

// Define this as a plug-in
DEFINE_FWK_MODULE(DummyL1TMuonEndCapTrackProducerSep2016);
