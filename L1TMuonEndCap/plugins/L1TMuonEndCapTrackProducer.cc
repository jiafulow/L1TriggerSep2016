#include "L1TMuonEndCapTrackProducer.h"


L1TMuonEndCapTrackProducer::L1TMuonEndCapTrackProducer(const edm::ParameterSet& iConfig) :
    track_finder_(new EMTFTrackFinder(iConfig, consumesCollector())),
    config_(iConfig)
{
  // Make output products
  produces<l1t::EMTFHitExtraCollection>      ("EMTF");
  produces<l1t::EMTFTrackExtraCollection>    ("EMTF");
  produces<l1t::RegionalMuonCandBxCollection>("EMTF");
}

L1TMuonEndCapTrackProducer::~L1TMuonEndCapTrackProducer() {

}

void L1TMuonEndCapTrackProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  // Create pointers to output products
  std::unique_ptr<l1t::EMTFHitExtraCollection>       out_hits  (new l1t::EMTFHitExtraCollection);
  std::unique_ptr<l1t::EMTFTrackExtraCollection>     out_tracks(new l1t::EMTFTrackExtraCollection);
  std::unique_ptr<l1t::RegionalMuonCandBxCollection> out_cands (new l1t::RegionalMuonCandBxCollection);

  // Run
  track_finder_->process(iEvent, iSetup, *out_hits, *out_tracks, *out_cands);

  // Fill the output products
  iEvent.put(std::move(out_hits)  , "EMTF");
  iEvent.put(std::move(out_tracks), "EMTF");
  iEvent.put(std::move(out_cands) , "EMTF");
}

void L1TMuonEndCapTrackProducer::beginJob() {

}

void L1TMuonEndCapTrackProducer::endJob() {

}

//fill 'descriptions' with the allowed parameters
void L1TMuonEndCapTrackProducer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
typedef L1TMuonEndCapTrackProducer L1TMuonEndCapTrackProducerSep2016;
DEFINE_FWK_MODULE(L1TMuonEndCapTrackProducerSep2016);
