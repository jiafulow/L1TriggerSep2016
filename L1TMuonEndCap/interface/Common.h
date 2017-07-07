#ifndef L1TMuonEndCap_Common_h
#define L1TMuonEndCap_Common_h

#include "DataFormats/L1TMuon/interface/EMTFHit.h"
#include "DataFormats/L1TMuon/interface/EMTFRoad.h"
#include "DataFormats/L1TMuon/interface/EMTFTrack.h"

#include "L1Trigger/L1TMuonEndCap/interface/GeometryTranslator.h"
#include "L1Trigger/L1TMuonEndCap/interface/MuonTriggerPrimitive.h"
#include "L1Trigger/L1TMuonEndCap/interface/MuonTriggerPrimitiveFwd.h"

#include "L1Trigger/L1TMuonEndCap/interface/TTGeometryTranslator.h"
#include "L1Trigger/L1TMuonEndCap/interface/TTMuonTriggerPrimitive.h"

#include "L1Trigger/L1TMuonEndCap/interface/EMTFSubsystemTag.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"


// Resolve namespaces

typedef l1t::EMTFHit             EMTFHit;
typedef l1t::EMTFHitCollection   EMTFHitCollection;
typedef l1t::EMTFRoad            EMTFRoad;
typedef l1t::EMTFRoadCollection  EMTFRoadCollection;
typedef l1t::EMTFTrack           EMTFTrack;
typedef l1t::EMTFTrackCollection EMTFTrackCollection;
typedef l1t::EMTFPtLUT           EMTFPtLUT;

typedef L1TMuonEndCap::GeometryTranslator         GeometryTranslator;
typedef L1TMuonEndCap::TriggerPrimitive           TriggerPrimitive;
typedef L1TMuonEndCap::TriggerPrimitiveCollection TriggerPrimitiveCollection;

typedef L1TMuonEndCap::TTGeometryTranslator         TTGeometryTranslator;
typedef L1TMuonEndCap::TTTriggerPrimitive           TTTriggerPrimitive;
typedef L1TMuonEndCap::TTTriggerPrimitiveCollection TTTriggerPrimitiveCollection;

typedef TriggerPrimitive::CSCData   CSCData;
typedef TriggerPrimitive::RPCData   RPCData;
typedef TriggerPrimitive::GEMData   GEMData;
typedef TTTriggerPrimitive::TTData  TTData;

typedef emtf::CSCTag CSCTag;
typedef emtf::RPCTag RPCTag;
typedef emtf::GEMTag GEMTag;
typedef emtf::TTTag  TTTag;

namespace emtf {

  // Constants

  // from DataFormats/MuonDetId/interface/CSCDetId.h
  constexpr int MIN_ENDCAP = 1;
  constexpr int MAX_ENDCAP = 2;

  // from DataFormats/MuonDetId/interface/CSCTriggerNumbering.h
  constexpr int MIN_TRIGSECTOR = 1;
  constexpr int MAX_TRIGSECTOR = 6;
  constexpr int NUM_SECTORS = 12;

  // Zones
  constexpr int NUM_ZONES = 4;
  constexpr int NUM_ZONE_HITS = 160;

  // Stations
  constexpr int NUM_STATIONS = 4;
  constexpr int NUM_STATION_PAIRS = 6;

  // Fixed-size arrays
  #include <array>
  template<typename T>
  using sector_array = std::array<T, NUM_SECTORS>;
  template<typename T>
  using zone_array = std::array<T, NUM_ZONES>;

} // namespace emtf

#endif

