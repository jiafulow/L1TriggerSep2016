#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFSubsystemCollector.hh" // Why .hh and not .h? - AWB 27.09.16
// Why aren't the includes below in EMTFSubsystemCollector.hh? - AWB 27.09.16
#include "FWCore/Framework/interface/Event.h"
#include "DataFormats/Common/interface/Handle.h"


// Specialized for CSC
template<>
void EMTFSubsystemCollector::extractPrimitives(
    CSCTag tag, // Defined in interface//EMTFSubsystemTag.hh, maps to CSCCorrelatedLCTDigi
    const edm::Event& iEvent,
    const edm::EDGetToken& token,
    TriggerPrimitiveCollection& out
) {
  edm::Handle<CSCTag::digi_collection> cscDigis;
  if (!token.isUninitialized())
      iEvent.getByToken(token, cscDigis);

  auto chamber = cscDigis->begin();
  auto chend   = cscDigis->end();
  for( ; chamber != chend; ++chamber ) {
    auto digi = (*chamber).second.first;
    auto dend = (*chamber).second.second;
    for( ; digi != dend; ++digi ) {
      out.emplace_back((*chamber).first,*digi); // emplace_back does the same thing as push_back: appends to the end of the vector - AWB 28.09.16
    }
  }
  return;
}

// Specialized for RPC
template<>
void EMTFSubsystemCollector::extractPrimitives(
    RPCTag tag, // Defined in interface//EMTFSubsystemTag.hh, maps to RPCDigi
    const edm::Event& iEvent,
    const edm::EDGetToken& token,
    TriggerPrimitiveCollection& out
) {
  edm::Handle<RPCTag::digi_collection> rpcDigis;
  if (!token.isUninitialized())
      iEvent.getByToken(token, rpcDigis);

  auto chamber = rpcDigis->begin();
  auto chend   = rpcDigis->end();
  for( ; chamber != chend; ++chamber ) {
    auto digi = (*chamber).second.first;
    auto dend = (*chamber).second.second;
    for( ; digi != dend; ++digi ) {
      if ((*chamber).first.region() != 0) {  // 0 is barrel
        out.emplace_back((*chamber).first,digi->strip(),(*chamber).first.layer(),digi->bx());
      }
    }
  }
  return;
}
