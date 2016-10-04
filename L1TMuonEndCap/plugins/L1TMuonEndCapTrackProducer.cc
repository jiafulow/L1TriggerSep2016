#include "L1TMuonEndCapTrackProducer.h"


L1TMuonEndCapTrackProducer::L1TMuonEndCapTrackProducer(const edm::ParameterSet& iConfig) :
    track_finder_(new EMTFTrackFinder(iConfig, consumesCollector())), // Builds tracks from input hits
    track_adaptor_(new EMTFTrackAdaptor()),      // Converts output tracks to unpacked data format 
    uGMT_converter_(new EMTFMicroGMTConverter()), // Converts output tracks into RegionalMuonCands for uGMT
    config_(iConfig)
{
  // Make output products
  produces<EMTFHitExtraCollection>           (""); // Same as EMTFHit, but with extra emulator-only variables
  produces<EMTFTrackExtraCollection>         (""); // Same as EMTFTrack, but with extra emulator-only variables
  produces<l1t::RegionalMuonCandBxCollection>("EMTF"); // EMTF tracks output to uGMT

  produces<EMTFHitCollection>                (""); // All CSC LCTs and RPC clusters received by EMTF
  produces<EMTFTrackCollection>              (""); // All output EMTF tracks, in same format as unpacked data
}

L1TMuonEndCapTrackProducer::~L1TMuonEndCapTrackProducer() {

}

void L1TMuonEndCapTrackProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  // Create pointers to output products
  auto out_xhits   = std::make_unique<EMTFHitExtraCollection>();
  auto out_xtracks = std::make_unique<EMTFTrackExtraCollection>();
  auto out_cands   = std::make_unique<l1t::RegionalMuonCandBxCollection>();

  auto out_hits    = std::make_unique<EMTFHitCollection>();
  auto out_tracks  = std::make_unique<EMTFTrackCollection>();

  // Main EMTF emulator process, produces tracks from hits in each sector in each event
  // Defined in src/EMTFTrackFinder.cc
  track_finder_->process(iEvent, iSetup, *out_xhits, *out_xtracks);

  // Convert into unpacker EMTFHit, EMTFTrack formats
  track_adaptor_->convert_all(*out_xhits, *out_xtracks, *out_hits, *out_tracks);

  // Convert into uGMT format
  uGMT_converter_->convert_all(*out_xtracks, *out_cands);

  // Fill the output products
  iEvent.put(std::move(out_xhits)  , "");
  iEvent.put(std::move(out_xtracks), "");
  iEvent.put(std::move(out_cands)  , "EMTF");

  iEvent.put(std::move(out_hits)   , "");
  iEvent.put(std::move(out_tracks) , "");
}

void L1TMuonEndCapTrackProducer::beginJob() {

}

void L1TMuonEndCapTrackProducer::endJob() {

}

// Fill 'descriptions' with the allowed parameters
void L1TMuonEndCapTrackProducer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

// Define this as a plug-in
typedef L1TMuonEndCapTrackProducer L1TMuonEndCapTrackProducerSep2016;
DEFINE_FWK_MODULE(L1TMuonEndCapTrackProducerSep2016);
