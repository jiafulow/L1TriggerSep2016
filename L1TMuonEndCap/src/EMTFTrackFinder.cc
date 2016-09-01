#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFTrackFinder.hh"


EMTFTrackFinder::EMTFTrackFinder(const edm::ParameterSet& iConfig, edm::ConsumesCollector&& iConsumes) :
    tokenCSC_(iConsumes.consumes<CSCCorrelatedLCTDigiCollection>(iConfig.getParameter<edm::InputTag>("CSCInput"))),
    tokenRPC_(iConsumes.consumes<RPCDigiCollection>(iConfig.getParameter<edm::InputTag>("RPCInput")))
{

}

EMTFTrackFinder::~EMTFTrackFinder() {

}

void EMTFTrackFinder::process(
    const edm::Event& iEvent, const edm::EventSetup& iSetup,
    l1t::EMTFHitExtraCollection& out_hits,
    l1t::EMTFTrackExtraCollection& out_tracks,
    l1t::RegionalMuonCandBxCollection& out_cands
) {

}
