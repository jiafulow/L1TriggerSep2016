#ifndef L1TMuonEndCap_Phase2SectorProcessor_h_experimental
#define L1TMuonEndCap_Phase2SectorProcessor_h_experimental


// _____________________________________________________________________________
// This implements a TEMPORARY version of the Phase 2 EMTF sector processor.
// It is supposed to be replaced in the future. It is intentionally written
// in a monolithic fashion to allow easy replacement.
//

#include <deque>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <array>

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "L1Trigger/L1TMuonEndCap/interface/Common.h"
//#include "L1Trigger/L1TMuonEndCap/interface/GeometryTranslator.h"
#include "L1Trigger/L1TMuonEndCap/interface/ConditionHelper.h"
#include "L1Trigger/L1TMuonEndCap/interface/SectorProcessorLUT.h"


namespace experimental {

class Phase2SectorProcessor {
public:
  void configure(
      // Object pointers
      const GeometryTranslator* geom,
      const ConditionHelper* cond,
      const SectorProcessorLUT* lut,
      // Sector processor config
      int verbose, int endcap, int sector, int bx,
      int bxShiftCSC, int bxShiftRPC, int bxShiftGEM,
      std::string era
  );

  void process(
      // Input
      const edm::Event& iEvent, const edm::EventSetup& iSetup,
      const TriggerPrimitiveCollection& muon_primitives,
      // Output
      EMTFHitCollection& out_hits,
      EMTFTrackCollection& out_tracks
  ) const;

private:
  const GeometryTranslator* geom_;

  const ConditionHelper* cond_;

  const SectorProcessorLUT* lut_;

  int verbose_, endcap_, sector_, bx_,
      bxShiftCSC_, bxShiftRPC_, bxShiftGEM_;

  std::string era_;
};

}  // namesapce experimental

#endif  // L1TMuonEndCap_Phase2SectorProcessor_h_experimental
