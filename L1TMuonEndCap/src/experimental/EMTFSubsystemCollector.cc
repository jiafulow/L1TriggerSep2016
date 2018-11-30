#include "L1Trigger/L1TMuonEndCap/interface/experimental/EMTFSubsystemCollector.h"

#include "FWCore/Framework/interface/Event.h"
#include "DataFormats/Common/interface/Handle.h"
#include "Geometry/CSCGeometry/interface/CSCGeometry.h"
#include "Geometry/RPCGeometry/interface/RPCGeometry.h"
#include "Geometry/GEMGeometry/interface/GEMGeometry.h"
#include "Geometry/GEMGeometry/interface/ME0Geometry.h"

//#include "helper.h"  // adjacent_cluster

#include "L1Trigger/CSCCommonTrigger/interface/CSCConstants.h"
#include "L1Trigger/CSCTriggerPrimitives/src/CSCComparatorDigiFitter.h"
#include "L1Trigger/L1TMuonEndCap/interface/EMTFCSCComparatorDigiFitter.h"


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

  // TAMU comparator digi fitter
  bool use_tamu_fitter = false;
  std::unique_ptr<CSCComparatorDigiFitter> tamu_fitter;
  if (use_tamu_fitter) {
    tamu_fitter = std::make_unique<CSCComparatorDigiFitter>();
    tamu_fitter->setGeometry(&(tp_geom->getCSCGeometry()));
    tamu_fitter->setStripBits(0);
    tamu_fitter->useKeyRadius(true);
  }

  // My rough comparator digi fitter
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

      // TAMU comparator digi fitter
      if (use_tamu_fitter && (1 <= detid.station() && detid.station() <= 4) && (detid.ring() == 1 || detid.ring() == 4)) {
        std::vector<float> fit_phi_layers;
        std::vector<float> fit_z_layers;
        float fitRadius = 0.;
        tamu_fitter->fit(detid, lct, *cscComparatorDigis, fit_phi_layers, fit_z_layers, fitRadius);

        // Debug
        //std::cout << "size: " << fit_phi_layers.size() << ", " << fit_z_layers.size() << std::endl;
        //for (unsigned i=0; i < fit_phi_layers.size(); ++i) {
        //  std::cout << ".. " << i << " " << fit_phi_layers.at(i) << ", " << fit_z_layers.at(i) << std::endl;
        //}
      }

      // My rough comparator digi fitter
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
        bool is_me11 = (detid.station() == 1 && (detid.ring() == 1 || detid.ring() == 4));
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
      assert(nhitlayers >= 3);

      const std::pair<float, float>& res = emtf_fitter->fit(compDigisAllLayers, stagger, lct.getStrip());
      //std::cout << "fit result: " << res.first << " " << res.second << std::endl;

      out.emplace_back(detid, lct);

      // Overwrite bend and quality
      // 'bend' is deltaPhi. It gets converted into large positive number if negative. Multiply by 4 for now.
      // 'quality' is chi2/ndof. Multiply by 100 for now.
      out.back().accessCSCData().bend    = static_cast<uint16_t>(std::round(res.first * 4));
      out.back().accessCSCData().quality = static_cast<uint16_t>(std::round(res.second * 100));
    }
  }
  return;
}

}  // namespace experimental
