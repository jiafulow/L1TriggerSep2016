#ifndef L1TMuonEndCap_EMTFSubsystemCollector_h
#define L1TMuonEndCap_EMTFSubsystemCollector_h

#include "L1Trigger/L1TMuonEndCap/interface/Common.h"


// Forward declarations
namespace edm {
  class Event;
  class EDGetToken;
}


// Class declaration
class EMTFSubsystemCollector {
public:
  // For 1 input collection
  template<typename T>
  void extractPrimitives(
    T tag,
    const GeometryTranslator* tp_geom,
    const edm::Event& iEvent,
    const edm::EDGetToken& token,
    TriggerPrimitiveCollection& out
  ) const;

  // For 2 input collections
  template<typename T>
  void extractPrimitives(
    T tag,
    const GeometryTranslator* tp_geom,
    const edm::Event& iEvent,
    const edm::EDGetToken& token1,
    const edm::EDGetToken& token2,
    TriggerPrimitiveCollection& out
  ) const;

  // RPC functions
  void cluster_rpc(const TriggerPrimitiveCollection& muon_primitives, TriggerPrimitiveCollection& clus_muon_primitives) const;

  // GEM functions
  // 1. Cluster GEM pads.
  // 2. Declusterize GEM clusters.
  //    - Reject clusters with width > 8 pads. Then, for each of the 2 layers, declusterize a maximum of 8 pad clusters.
  // 3. Make GEM copads.
  void cluster_gem(const TriggerPrimitiveCollection& muon_primitives, TriggerPrimitiveCollection& clus_muon_primitives) const;

  void declusterize_gem(TriggerPrimitiveCollection& clus_muon_primitives, TriggerPrimitiveCollection& declus_muon_primitives) const;

  void make_copad_gem(TriggerPrimitiveCollection& declus_muon_primitives, TriggerPrimitiveCollection& copad_muon_primitives) const;
};

#endif
