#include "L1Trigger/L1TMuonEndCap/interface/experimental/EMTFSubsystemCollector.h"

#include "FWCore/Framework/interface/Event.h"
#include "DataFormats/Common/interface/Handle.h"
#include "Geometry/CSCGeometry/interface/CSCGeometry.h"
#include "Geometry/RPCGeometry/interface/RPCGeometry.h"
#include "Geometry/GEMGeometry/interface/GEMGeometry.h"
#include "Geometry/GEMGeometry/interface/ME0Geometry.h"

//#include "helper.h"  // adjacent_cluster

#include "L1Trigger/L1TMuonEndCap/interface/experimental/EMTFCSCComparatorDigiFitter.h"


namespace experimental {

// Specialized for CSC
template<>
void EMTFSubsystemCollector::extractPrimitives(
    CSCTag tag,
    const GeometryTranslator* tp_geom,
    const edm::Event& iEvent,
    const edm::EDGetToken& token1, // for CSC digis
    const edm::EDGetToken& token2, // for CSC comparator digis
    TriggerPrimitiveCollection& out
) const {
  edm::Handle<CSCTag::digi_collection> cscDigis;
  iEvent.getByToken(token1, cscDigis);

  edm::Handle<CSCTag::comparator_digi_collection> cscComparatorDigis;
  iEvent.getByToken(token2, cscComparatorDigis);

  // My comparator digi fitter
  std::unique_ptr<EMTFCSCComparatorDigiFitter> emtf_fitter = std::make_unique<EMTFCSCComparatorDigiFitter>();

  // Loop over chambers
  auto chamber = cscDigis->begin();
  auto chend   = cscDigis->end();
  for( ; chamber != chend; ++chamber ) {
    auto digi = (*chamber).second.first;
    auto dend = (*chamber).second.second;
    for( ; digi != dend; ++digi ) {
      const CSCDetId& detid = (*chamber).first;
      const CSCCorrelatedLCTDigi& lct = (*digi);

      // _______________________________________________________________________
      // My comparator digi fitter
      std::vector<std::vector<CSCComparatorDigi> > compDigisAllLayers(CSCConstants::NUM_LAYERS);
      std::vector<int> stagger(CSCConstants::NUM_LAYERS, 0);

      //std::cout << "LCT detid " << detid << ", keyStrip " << lct.getStrip() << ", pattern " << lct.getPattern() << ", keyWG " << lct.getKeyWG() << std::endl;
      for (int ilayer=0; ilayer<CSCConstants::NUM_LAYERS; ++ilayer) {
        // Build CSCDetId for particular layer
        // Layer numbers increase going away from the IP
        const CSCDetId layerId(detid.endcap(), detid.station(), detid.ring(), detid.chamber(), ilayer+1);

        // Retrieve comparator digis
        const auto& compRange = cscComparatorDigis->get(layerId);
        for (auto compDigiItr = compRange.first; compDigiItr != compRange.second; compDigiItr++) {
          const CSCComparatorDigi& compDigi = (*compDigiItr);
          if (std::abs(compDigi.getHalfStrip() - lct.getStrip()) <= 5) {  // only if the comparator digis fit the CLCT patterns
            compDigisAllLayers.at(ilayer).push_back(compDigi);
          }

          // Debug
          //std::cout << ".. " << ilayer+1 << " " << compDigi.getHalfStrip() << " " << compDigi.getFractionalStrip() << " " << compDigi.getTimeBin() << std::endl;
        }

        // Stagger corrections
        // In all types of chambers except ME1/1, strip 1 is indented by 1/2-strip in layers 1 (top), 3, and 5;
        // with respect to strip 1 in layers 2, 4, and 6 (bottom).
        const bool is_me11 = (detid.station() == 1 && (detid.ring() == 1 || detid.ring() == 4));
        if (!is_me11) {
          stagger.at(ilayer) = ((ilayer+1)%2 == 0) ? 0 : 1;  // 1,3,5 -> stagger; 2,4,6 -> no stagger
        }

        // Check stagger corrections
        //{
        //  const CSCChamber* chamber = tp_geom->getCSCGeometry().chamber(detid);
        //  int stagger0 = stagger.at(ilayer);
        //  int stagger1 = (chamber->layer(ilayer+1)->geometry()->stagger() + 1) / 2;
        //  assert(stagger0 == stagger1);
        //}
      }

      int nhitlayers = 0;
      for (const auto& x : compDigisAllLayers) {
        if (x.size() > 0)
          ++nhitlayers;
      }
      if (nhitlayers == 0)  // how?!
        continue;

      assert(nhitlayers >= 3);

      const EMTFCSCComparatorDigiFitter::FitResult& res = emtf_fitter->fit(compDigisAllLayers, stagger, lct.getStrip());
      //std::cout << "fit result: " << res.position << " " << res.slope << " " << res.chi2 << " halfStrip: " << lct.getStrip() << std::endl;

      // Find half-strip after fit
      float position = res.position + lct.getStrip();
      int strip = std::round(position);

      // Encode fractional strip in 3 bits (4 including sign), which corresponds to 1/16-strip unit
      float frac_position = position - static_cast<float>(strip);
      int frac_strip = static_cast<int>(std::round(std::abs(frac_position) * 8));
      frac_strip = std::min(std::max(frac_strip, 0), 7);
      frac_strip = (frac_position >= 0) ? frac_strip : -frac_strip;

      // Encode bend in 6 bits, which correspinds to 1/32-strip unit
      int bend = static_cast<int>(std::round(res.slope * 16));
      bend = std::min(std::max(bend, -32), 31);

      // Encode quality in 5 bits, which corresponds to 0.25 step from 0 to 8
      int quality = static_cast<int>(std::round(res.chi2 * 4));
      quality = std::min(std::max(quality, 0), 31);

      //std::cout << "check position: " << position << " " << strip << " " << frac_position << " " << frac_strip << " " << (float(strip) + float(frac_strip)/8) << std::endl;
      //std::cout << "check bend    : " << bend << " " << float(bend)/16 << std::endl;
      //std::cout << "check quality : " << quality << " " << float(quality)/4 << std::endl;

      // _______________________________________________________________________
      // Output
      out.emplace_back(detid, lct);

      // Overwrite strip, bend, quality, and syncErr
      out.back().accessCSCData().strip   = static_cast<uint16_t>(strip);
      out.back().accessCSCData().bend    = static_cast<uint16_t>(bend);
      out.back().accessCSCData().quality = static_cast<uint16_t>(quality);
      out.back().accessCSCData().syncErr = static_cast<uint16_t>(frac_strip);
    }
  }
  return;
}

// _____________________________________________________________________________
// Specialized for RPC
template<>
void EMTFSubsystemCollector::extractPrimitives(
    RPCTag tag,
    const GeometryTranslator* tp_geom,
    const edm::Event& iEvent,
    const edm::EDGetToken& token1, // for RPC digis
    const edm::EDGetToken& token2, // for RPC rechits
    TriggerPrimitiveCollection& out
) const {
  constexpr int maxClusterSize = 3;

  //edm::Handle<RPCTag::digi_collection> rpcDigis;
  //iEvent.getByToken(token1, rpcDigis);

  edm::Handle<RPCTag::rechit_collection> rpcRecHits;
  iEvent.getByToken(token2, rpcRecHits);

  auto rechit = rpcRecHits->begin();
  auto rhend  = rpcRecHits->end();
  for (; rechit != rhend; ++rechit) {
    const RPCDetId& detid = rechit->rpcId();
    const RPCRoll* roll = dynamic_cast<const RPCRoll*>(tp_geom->getRPCGeometry().roll(detid));
    if (!roll)  continue;

    if (detid.region() != 0) {  // 0 is barrel
      //if (detid.station() <= 2 && detid.ring() == 3)  continue;  // do not include RE1/3, RE2/3
      if (detid.station() >= 3 && detid.ring() == 1)  continue;  // do not include RE3/1, RE4/1 (iRPC)

      if (rechit->clusterSize() <= maxClusterSize) {
        out.emplace_back(detid, *rechit);
      }
    }
  }
  return;
}

// _____________________________________________________________________________
// Specialized for iRPC
template<>
void EMTFSubsystemCollector::extractPrimitives(
    IRPCTag tag,
    const GeometryTranslator* tp_geom,
    const edm::Event& iEvent,
    const edm::EDGetToken& token1, // for RPC digis
    const edm::EDGetToken& token2, // for RPC rechits
    TriggerPrimitiveCollection& out
) const {
  constexpr int maxClusterSize = 6;

  //edm::Handle<IRPCTag::digi_collection> irpcDigis;
  //iEvent.getByToken(token1, irpcDigis);

  edm::Handle<IRPCTag::rechit_collection> irpcRecHits;
  iEvent.getByToken(token2, irpcRecHits);

  auto rechit = irpcRecHits->begin();
  auto rhend  = irpcRecHits->end();
  for (; rechit != rhend; ++rechit) {
    const RPCDetId& detid = rechit->rpcId();
    const RPCRoll* roll = dynamic_cast<const RPCRoll*>(tp_geom->getRPCGeometry().roll(detid));
    if (!roll)  continue;

    if (detid.region() != 0) {  // 0 is barrel
      if (detid.station() >= 3 && detid.ring() == 1) {  // only RE3/1, RE4/1 (iRPC)
        RPCRecHit tmpRecHit = (*rechit);  // make a copy

        // According to the TDR, the iRPC chamber has 96 strips.
        // But in CMSSW, the iRPC chamber has 192 strips.
        bool fixNStrips192 = true;
        if (fixNStrips192 && roll->nstrips() == 192) {
          int firstStrip  = tmpRecHit.firstClusterStrip();
          int clusterSize = tmpRecHit.clusterSize();
          float time      = tmpRecHit.time();
          float timeError = tmpRecHit.timeError();

          int lastStrip  = firstStrip + clusterSize - 1;
          firstStrip -= 1;
          firstStrip /= 2;
          firstStrip += 1;
          lastStrip -= 1;
          lastStrip /= 2;
          lastStrip += 1;
          clusterSize = lastStrip - firstStrip + 1;

          tmpRecHit = RPCRecHit(tmpRecHit.rpcId(), tmpRecHit.BunchX(), firstStrip, clusterSize, tmpRecHit.localPosition(), tmpRecHit.localPositionError());
          tmpRecHit.setTimeAndError(time, timeError);
        }

        if (tmpRecHit.clusterSize() <= maxClusterSize) {
          out.emplace_back(detid, tmpRecHit);
        }
      }
    }
  }
  return;
}

}  // namespace experimental
