#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFSingleBXSectorProcessor.hh"

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPrimitiveSelection.hh"
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

  EMTFPrimitiveSelection prim_sel;
  EMTFPrimitiveConversion prim_conv;

  TriggerPrimitiveCollection::const_iterator tp_it  = muon_primitives.begin();
  TriggerPrimitiveCollection::const_iterator tp_end = muon_primitives.end();

  for (; tp_it != tp_end; ++tp_it) {
    int selected_csc = prim_sel.select(CSCTag(), sector_, bx_, includeNeighbor_, *tp_it);
    int selected_rpc = prim_sel.select(RPCTag(), sector_, bx_, includeNeighbor_, *tp_it);

    if (selected_csc) {
      bool is_neighbor = (selected_csc == 2);
      const EMTFHitExtra& conv_hit = prim_conv.convert(CSCTag(), sector_, is_neighbor, *tp_it);
      conv_hits.push_back(conv_hit);

    } else if (selected_rpc) {
      bool is_neighbor = (selected_rpc == 2);
      const EMTFHitExtra& conv_hit = prim_conv.convert(RPCTag(), sector_, is_neighbor, *tp_it);
      conv_hits.push_back(conv_hit);
    }
  }




  out_hits.insert(out_hits.end(), conv_hits.begin(), conv_hits.end());

  return;
}
