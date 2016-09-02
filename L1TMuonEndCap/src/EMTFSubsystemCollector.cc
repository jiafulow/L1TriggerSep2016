#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFSubsystemCollector.hh"

#include "FWCore/Framework/interface/Event.h"
#include "DataFormats/Common/interface/Handle.h"


// Specialized for CSC
template<> void EMTFSubsystemCollector::extractPrimitives<L1TMuonEndCap::CSCTag>(
    const edm::Event& iEvent,
    const edm::EDGetToken& token,
    TriggerPrimitiveCollection& out
) {
  edm::Handle<L1TMuonEndCap::CSCTag::digi_collection> cscDigis;
  if (!token.isUninitialized())
      iEvent.getByToken(token, cscDigis);

  auto chamber = cscDigis->begin();
  auto chend   = cscDigis->end();
  for( ; chamber != chend; ++chamber ) {
    auto digi = (*chamber).second.first;
    auto dend = (*chamber).second.second;
    for( ; digi != dend; ++digi ) {
      out.emplace_back((*chamber).first,*digi);
    }
  }
  return;
}

// Specialized for RPC
template<> void EMTFSubsystemCollector::extractPrimitives<L1TMuonEndCap::RPCTag>(
    const edm::Event& iEvent,
    const edm::EDGetToken& token,
    TriggerPrimitiveCollection& out
) {
  edm::Handle<L1TMuonEndCap::RPCTag::digi_collection> rpcDigis;
  if (!token.isUninitialized())
      iEvent.getByToken(token, rpcDigis);

  auto chamber = rpcDigis->begin();
  auto chend   = rpcDigis->end();
  for( ; chamber != chend; ++chamber ) {
    auto digi = (*chamber).second.first;
    auto dend = (*chamber).second.second;
    for( ; digi != dend; ++digi ) {
      out.emplace_back((*chamber).first,digi->strip(),(*chamber).first.layer(),digi->bx());
    }
  }
  return;
}
