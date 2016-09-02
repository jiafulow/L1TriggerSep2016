#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFSectorProcessor.hh"

#define MIN_BX -3
#define MAX_BX 4


EMTFSectorProcessor::EMTFSectorProcessor(const edm::ParameterSet& iConfig) :
    config_(iConfig),
    verbose_(iConfig.getUntrackedParameter<int>("verbosity"))
{

}

EMTFSectorProcessor::~EMTFSectorProcessor() {

}

void EMTFSectorProcessor::reset(int sector) {
  sector_ = sector;
}

void EMTFSectorProcessor::process(
    const L1TMuon::TriggerPrimitiveCollection& muon_primitives,
    l1t::EMTFHitExtraCollection& out_hits,
    l1t::EMTFTrackExtraCollection& out_tracks
) {

  EMTFSingleBXSectorProcessor sector_processor_bx_minus2(config_);
  EMTFSingleBXSectorProcessor sector_processor_bx_minus1(config_);
  EMTFSingleBXSectorProcessor sector_processor_bx_0(config_);

  for (int ibx = MIN_BX; ibx < MAX_BX; ibx++) {
    sector_processor_bx_0.reset(sector(), ibx);
    sector_processor_bx_0.process(muon_primitives, out_hits, out_tracks);
  }

  return;
}
