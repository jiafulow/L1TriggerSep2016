#ifndef L1TMuonEndCap_EMTFCommon_hh
#define L1TMuonEndCap_EMTFCommon_hh

#include "DataFormatsSep2016/L1TMuon/interface/EMTFHit.h"
#include "DataFormatsSep2016/L1TMuon/interface/EMTFRoad.h"
#include "DataFormatsSep2016/L1TMuon/interface/EMTFTrack.h"

#include "DataFormatsSep2016/L1TMuon/interface/EMTFHitExtra.h"
#include "DataFormatsSep2016/L1TMuon/interface/EMTFRoadExtra.h"
#include "DataFormatsSep2016/L1TMuon/interface/EMTFTrackExtra.h"

#include "L1TriggerSep2016/L1TMuonEndCap/interface/MuonTriggerPrimitive.h"
#include "L1TriggerSep2016/L1TMuonEndCap/interface/MuonTriggerPrimitiveFwd.h"

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFSubsystemTag.hh"


typedef L1TMuonEndCap::EMTFHit             EMTFHit;
typedef L1TMuonEndCap::EMTFHitCollection   EMTFHitCollection;
typedef L1TMuonEndCap::EMTFRoad            EMTFRoad;
typedef L1TMuonEndCap::EMTFRoadCollection  EMTFRoadCollection;
typedef L1TMuonEndCap::EMTFTrack           EMTFTrack;
typedef L1TMuonEndCap::EMTFTrackCollection EMTFTrackCollection;

typedef L1TMuonEndCap::EMTFHitExtra             EMTFHitExtra;
typedef L1TMuonEndCap::EMTFHitExtraCollection   EMTFHitExtraCollection;
typedef L1TMuonEndCap::EMTFRoadExtra            EMTFRoadExtra;
typedef L1TMuonEndCap::EMTFRoadExtraCollection  EMTFRoadExtraCollection;
typedef L1TMuonEndCap::EMTFTrackExtra           EMTFTrackExtra;
typedef L1TMuonEndCap::EMTFTrackExtraCollection EMTFTrackExtraCollection;

typedef L1TMuonEndCap::EMTFPtLUTData EMTFPtLUTData;

typedef L1TMuonEndCap::TriggerPrimitive           TriggerPrimitive;
typedef L1TMuonEndCap::TriggerPrimitiveCollection TriggerPrimitiveCollection;

typedef L1TMuonEndCap::CSCTag CSCTag;
typedef L1TMuonEndCap::RPCTag RPCTag;

// Constants

// from DataFormats/MuonDetId/interface/CSCDetId.h
#define MIN_ENDCAP 1
#define MAX_ENDCAP 2

// from DataFormats/MuonDetId/interface/CSCTriggerNumbering.h
#define MIN_TRIGSECTOR 1
#define MAX_TRIGSECTOR 6
#define NUM_SECTORS 12

// Zones
#define NUM_ZONES 4
#define NUM_ZONE_HITS 160

// BX window
#define BX_WINDOW 3

// Stations
#define NUM_STATIONS 4
#define NUM_STATION_PAIRS 6

// Fixed-size arrays
#include <array>
template<typename T>
using zone_array = std::array<T, NUM_ZONES>;

template<typename T>
using bx_array = std::array<T, BX_WINDOW>;

#endif

