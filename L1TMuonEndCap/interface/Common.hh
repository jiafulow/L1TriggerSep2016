#ifndef L1TMuonEndCap_Common_hh
#define L1TMuonEndCap_Common_hh

#include "DataFormats/L1TMuon/interface/EMTFHit.h"
#include "DataFormats/L1TMuon/interface/EMTFRoad.h"
#include "DataFormats/L1TMuon/interface/EMTFTrack.h"

#include "L1Trigger/L1TMuonEndCap/interface/GeometryTranslator.h"
#include "L1Trigger/L1TMuonEndCap/interface/MuonTriggerPrimitive.h"
#include "L1Trigger/L1TMuonEndCap/interface/MuonTriggerPrimitiveFwd.h"

#include "L1Trigger/L1TMuonEndCap/interface/SubsystemTag.hh"


typedef L1TMuonEndCap::EMTFHit             EMTFHit;
typedef L1TMuonEndCap::EMTFHitCollection   EMTFHitCollection;
typedef L1TMuonEndCap::EMTFRoad            EMTFRoad;
typedef L1TMuonEndCap::EMTFRoadCollection  EMTFRoadCollection;
typedef L1TMuonEndCap::EMTFTrack           EMTFTrack;
typedef L1TMuonEndCap::EMTFTrackCollection EMTFTrackCollection;

typedef L1TMuonEndCap::EMTFHit             EMTFHit;
typedef L1TMuonEndCap::EMTFHitCollection   EMTFHitCollection;
typedef L1TMuonEndCap::EMTFRoad            EMTFRoad;
typedef L1TMuonEndCap::EMTFRoadCollection  EMTFRoadCollection;
typedef L1TMuonEndCap::EMTFTrack           EMTFTrack;
typedef L1TMuonEndCap::EMTFTrackCollection EMTFTrackCollection;

typedef L1TMuonEndCap::EMTFPtLUT EMTFPtLUT;

typedef L1TMuonEndCap::GeometryTranslator         GeometryTranslator;
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

// Stations
#define NUM_STATIONS 4
#define NUM_STATION_PAIRS 6

// Fixed-size arrays
#include <array>
template<typename T>
using zone_array = std::array<T, NUM_ZONES>;

#endif

