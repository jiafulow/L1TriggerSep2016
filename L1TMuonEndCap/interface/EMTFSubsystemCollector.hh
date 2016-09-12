#ifndef L1TMuonEndCap_EMTFSubsystemCollector_hh
#define L1TMuonEndCap_EMTFSubsystemCollector_hh

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFCommon.hh"


// Forward declarations
namespace edm {
  class Event;
  class EDGetToken;
}


// Class declaration
class EMTFSubsystemCollector {
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
