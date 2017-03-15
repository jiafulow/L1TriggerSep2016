#ifndef L1TMuonEndCap_SubsystemCollector_hh
#define L1TMuonEndCap_SubsystemCollector_hh

#include "L1Trigger/L1TMuonEndCap/interface/Common.hh"


// Forward declarations
namespace edm {
  class Event;
  class EDGetToken;
}


// Class declaration
class SubsystemCollector {
public:
  template<typename T>
  void extractPrimitives(
    T tag,
    const edm::Event& iEvent,
    const edm::EDGetToken& token,
    TriggerPrimitiveCollection& out
  );

};

#endif
