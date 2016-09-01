#include "L1TMuonEndCapTrackProducer.h"


using namespace L1TMuonEndCap;

L1TMuonEndCapTrackProducer::L1TMuonEndCapTrackProducer(const edm::ParameterSet& iConfig) {

}

L1TMuonEndCapTrackProducer::~L1TMuonEndCapTrackProducer() {

}

void L1TMuonEndCapTrackProducer::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {

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
