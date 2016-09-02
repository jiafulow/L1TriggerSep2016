#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFSingleBXSectorProcessor.hh"

EMTFSingleBXSectorProcessor::EMTFSingleBXSectorProcessor(const edm::ParameterSet& iConfig) :
    config_(iConfig),
    verbose_(iConfig.getUntrackedParameter<int>("verbosity"))
{

}

EMTFSingleBXSectorProcessor::~EMTFSingleBXSectorProcessor() {

}

void EMTFSingleBXSectorProcessor::reset(int sector, int bx) {
  sector_ = sector;
  bx_ = bx;
}

void EMTFSingleBXSectorProcessor::process(
    const L1TMuon::TriggerPrimitiveCollection& muon_primitives,
    l1t::EMTFHitExtraCollection& out_hits,
    l1t::EMTFTrackExtraCollection& out_tracks
) {

  //FIXME: implement this

  return;
}
