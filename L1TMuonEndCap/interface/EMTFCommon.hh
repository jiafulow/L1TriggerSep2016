#ifndef L1TMuonEndCap_EMTFCommon_hh
#define L1TMuonEndCap_EMTFCommon_hh

#include "DataFormatsSep2016/L1TMuon/interface/EMTFHitExtra.h"
#include "DataFormatsSep2016/L1TMuon/interface/EMTFTrackExtra.h"

#include "L1TriggerSep2016/L1TMuonEndCap/interface/MuonTriggerPrimitive.h"
#include "L1TriggerSep2016/L1TMuonEndCap/interface/MuonTriggerPrimitiveFwd.h"

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFSubsystemTag.hh"


typedef L1TMuonEndCap::EMTFHitExtra             EMTFHitExtra;
typedef L1TMuonEndCap::EMTFHitExtraCollection   EMTFHitExtraCollection;
typedef L1TMuonEndCap::EMTFTrackExtra           EMTFTrackExtra;
typedef L1TMuonEndCap::EMTFTrackExtraCollection EMTFTrackExtraCollection;

typedef L1TMuonEndCap::TriggerPrimitive           TriggerPrimitive;
typedef L1TMuonEndCap::TriggerPrimitiveCollection TriggerPrimitiveCollection;

typedef L1TMuonEndCap::CSCTag CSCTag;
typedef L1TMuonEndCap::RPCTag RPCTag;

// Constants
#define NUM_SECTORS 12

#endif

