#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFSingleBXSectorProcessor.hh"

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPrimitiveConversion.hh"


EMTFSingleBXSectorProcessor::EMTFSingleBXSectorProcessor(const edm::ParameterSet& iConfig) :
    config_(iConfig),
    verbose_(iConfig.getUntrackedParameter<int>("verbosity"))
{
  const edm::ParameterSet spPCParams16 = config_.getParameter<edm::ParameterSet>("spPCParams16");
  includeNeighbor_ = spPCParams16.getParameter<bool>("IncludeNeighbor");
}

EMTFSingleBXSectorProcessor::~EMTFSingleBXSectorProcessor() {

}

void EMTFSingleBXSectorProcessor::reset(int sector, int bx) {
  sector_ = sector;
  bx_ = bx;
}

void EMTFSingleBXSectorProcessor::process(
    const EMTFSingleBXSectorProcessorQueue& prev_processors,
    const TriggerPrimitiveCollection& muon_primitives,
    EMTFHitExtraCollection& out_hits,
    EMTFTrackExtraCollection& out_tracks
) {

  EMTFHitExtraCollection conv_hits;

  EMTFPrimitiveConversion prim_conv;
  prim_conv.convert(sector_, bx_, includeNeighbor_, muon_primitives, conv_hits);

  out_hits.insert(out_hits.end(), conv_hits.begin(), conv_hits.end());

  return;
}
