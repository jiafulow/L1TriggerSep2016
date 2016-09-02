#ifndef L1TMuonEndCap_EMTFSubsystemCollector_hh
#define L1TMuonEndCap_EMTFSubsystemCollector_hh

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFCommon.hh"
#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFSubsystemTag.hh"


// Forward declarations
namespace edm {
  class Event;
  class EDGetToken;
}


// Class declaration
struct EMTFSubsystemCollector {

  template<class T>
  void extractPrimitives(
    const edm::Event& iEvent,
    const edm::EDGetToken& token,
    TriggerPrimitiveCollection& out
  );

};

#endif
