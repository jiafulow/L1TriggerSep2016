#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFSectorProcessor.hh"

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFSingleBXSectorProcessor.hh"


EMTFSectorProcessor::EMTFSectorProcessor(const edm::ParameterSet& iConfig) :
    config_(iConfig),
    verbose_(iConfig.getUntrackedParameter<int>("verbosity"))
{
    const edm::ParameterSet spPRParams16 = config_.getParameter<edm::ParameterSet>("spPRParams16");
    minBX_    = spPRParams16.getParameter<int>("MinBX");
    maxBX_    = spPRParams16.getParameter<int>("MaxBX");
    bxWindow_ = spPRParams16.getParameter<int>("BXWindow");
}

EMTFSectorProcessor::~EMTFSectorProcessor() {

}

void EMTFSectorProcessor::reset(int sector) {
  sector_ = sector;
}

void EMTFSectorProcessor::process(
    const TriggerPrimitiveCollection& muon_primitives,
    EMTFHitExtraCollection& out_hits,
    EMTFTrackExtraCollection& out_tracks
) {

  EMTFSingleBXSectorProcessor single_bx_sector_processor(config_);
  std::queue<EMTFSingleBXSectorProcessor> prev_single_bx_sector_processors;

  int waitBX = bxWindow_ - 1;

  for (int ibx = minBX_; ibx < maxBX_ + waitBX; ibx++) {
    single_bx_sector_processor.reset(sector(), ibx);
    single_bx_sector_processor.process(
        prev_single_bx_sector_processors,
        muon_primitives, out_hits, out_tracks
    );

    prev_single_bx_sector_processors.push(single_bx_sector_processor);
    if (ibx >= waitBX) {
      prev_single_bx_sector_processors.pop();
    }
  }

  return;
}
