#include "L1Trigger/L1TMuonEndCap/interface/experimental/Phase2SectorProcessor.h"


// _____________________________________________________________________________
// This implements a TEMPORARY version of the Phase 2 EMTF sector processor.
// It is supposed to be replaced in the future. It is intentionally written
// in a monolithic fashion to allow easy replacement.
//


namespace experimental {

void Phase2SectorProcessor::configure(
    // Object pointers
    const GeometryTranslator* geom,
    const ConditionHelper* cond,
    const SectorProcessorLUT* lut,
    // Sector processor config
    int verbose, int endcap, int sector, int bx,
    int bxShiftCSC, int bxShiftRPC, int bxShiftGEM,
    std::string era
) {
  assert(emtf::MIN_ENDCAP <= endcap && endcap <= emtf::MAX_ENDCAP);
  assert(emtf::MIN_TRIGSECTOR <= sector && sector <= emtf::MAX_TRIGSECTOR);

  assert(geom != nullptr);
  assert(cond != nullptr);
  assert(lut  != nullptr);

  geom_       = geom;
  cond_       = cond;
  lut_        = lut;

  verbose_    = verbose;
  endcap_     = endcap;
  sector_     = sector;
  bx_         = bx;

  bxShiftCSC_ = bxShiftCSC;
  bxShiftRPC_ = bxShiftRPC;
  bxShiftGEM_ = bxShiftGEM;

  era_        = era;
}

void Phase2SectorProcessor::process(
    // Input
    const edm::Event& iEvent, const edm::EventSetup& iSetup,
    const TriggerPrimitiveCollection& muon_primitives,
    // Output
    EMTFHitCollection& out_hits,
    EMTFTrackCollection& out_tracks
) const {

}

}  // namespace experimental
