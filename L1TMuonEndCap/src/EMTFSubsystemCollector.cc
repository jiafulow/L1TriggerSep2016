#include "L1Trigger/L1TMuonEndCap/interface/EMTFSubsystemCollector.h"

#include "FWCore/Framework/interface/Event.h"
#include "DataFormats/Common/interface/Handle.h"
#include "Geometry/CSCGeometry/interface/CSCGeometry.h"
#include "Geometry/RPCGeometry/interface/RPCGeometry.h"
#include "Geometry/GEMGeometry/interface/GEMGeometry.h"
#include "Geometry/GEMGeometry/interface/ME0Geometry.h"

#include "L1Trigger/CSCCommonTrigger/interface/CSCConstants.h"
#include "L1Trigger/CSCTriggerPrimitives/src/CSCComparatorDigiFitter.h"

#include "L1Trigger/L1TMuonEndCap/interface/EMTFCSCComparatorDigiFitter.h"

#include "helper.h"  // adjacent_cluster


// Specialized for CSC
template<>
void EMTFSubsystemCollector::extractPrimitives(
    CSCTag tag, // Defined in interface/EMTFSubsystemTag.h, maps to CSCCorrelatedLCTDigi
    const GeometryTranslator* tp_geom,
    const edm::Event& iEvent,
    const edm::EDGetToken& token,
    TriggerPrimitiveCollection& out
) const {
  edm::Handle<CSCTag::digi_collection> cscDigis;
  iEvent.getByToken(token, cscDigis);

  auto chamber = cscDigis->begin();
  auto chend   = cscDigis->end();
  for( ; chamber != chend; ++chamber ) {
    auto digi = (*chamber).second.first;
    auto dend = (*chamber).second.second;
    for( ; digi != dend; ++digi ) {
      // emplace_back does the same thing as push_back: appends to the end of the vector
      out.emplace_back((*chamber).first,*digi);
    }
  }
  return;
}

// Specialized for RPC
template<>
void EMTFSubsystemCollector::extractPrimitives(
    RPCTag tag, // Defined in interface/EMTFSubsystemTag.h, maps to RPCDigi
    const GeometryTranslator* tp_geom,
    const edm::Event& iEvent,
    const edm::EDGetToken& token,
    TriggerPrimitiveCollection& out
) const {
  edm::Handle<RPCTag::digi_collection> rpcDigis;
  iEvent.getByToken(token, rpcDigis);

  TriggerPrimitiveCollection muon_primitives;

  auto chamber = rpcDigis->begin();
  auto chend   = rpcDigis->end();
  for( ; chamber != chend; ++chamber ) {
    auto digi = (*chamber).second.first;
    auto dend = (*chamber).second.second;
    for( ; digi != dend; ++digi ) {
      if ((*chamber).first.region() != 0) {  // 0 is barrel
        if ((*chamber).first.station() <= 2 && (*chamber).first.ring() == 3)  continue;  // do not include RE1/3, RE2/3
        if ((*chamber).first.station() >= 3 && (*chamber).first.ring() == 1)  continue;  // do not include RE3/1, RE4/1

        muon_primitives.emplace_back((*chamber).first,*digi);
      }
    }
  }

  // Cluster the RPC digis
  TriggerPrimitiveCollection clus_muon_primitives;
  cluster_rpc(muon_primitives, clus_muon_primitives);

  // Output
  std::copy(clus_muon_primitives.begin(), clus_muon_primitives.end(), std::back_inserter(out));
  return;
}

// Specialized for GEM
template<>
void EMTFSubsystemCollector::extractPrimitives(
    GEMTag tag, // Defined in interface/EMTFSubsystemTag.h, maps to GEMPadDigi
    const GeometryTranslator* tp_geom,
    const edm::Event& iEvent,
    const edm::EDGetToken& token,
    TriggerPrimitiveCollection& out
) const {
  edm::Handle<GEMTag::digi_collection> gemDigis;
  iEvent.getByToken(token, gemDigis);

  TriggerPrimitiveCollection muon_primitives;

  auto chamber = gemDigis->begin();
  auto chend   = gemDigis->end();
  for( ; chamber != chend; ++chamber ) {
    auto digi = (*chamber).second.first;
    auto dend = (*chamber).second.second;
    for( ; digi != dend; ++digi ) {
      muon_primitives.emplace_back((*chamber).first,*digi);

      //std::cout << "GEM detid " << (*chamber).first << ", pad " << digi->pad() << std::endl;
    }
  }

  // 1. Cluster GEM pads.
  TriggerPrimitiveCollection clus_muon_primitives;
  cluster_gem(muon_primitives, clus_muon_primitives);

  // 2. Declusterize GEM pads.
  //    - Reject clusters with width > 8 pads. Then, for each of the 2 layers, declusterize a maximum of 8 pad clusters.
  TriggerPrimitiveCollection declus_muon_primitives;
  declusterize_gem(clus_muon_primitives, declus_muon_primitives);

  // 3. Make GEM copads.
  TriggerPrimitiveCollection copad_muon_primitives;
  make_copad_gem(declus_muon_primitives, copad_muon_primitives);

  // Output
  std::copy(copad_muon_primitives.begin(), copad_muon_primitives.end(), std::back_inserter(out));
  return;
}

// Specialized for iRPC
template<>
void EMTFSubsystemCollector::extractPrimitives(
    IRPCTag tag, // Defined in interface/EMTFSubsystemTag.h, maps to RPCDigi
    const GeometryTranslator* tp_geom,
    const edm::Event& iEvent,
    const edm::EDGetToken& token,
    TriggerPrimitiveCollection& out
) const {
  edm::Handle<IRPCTag::digi_collection> irpcDigis;
  iEvent.getByToken(token, irpcDigis);

  TriggerPrimitiveCollection muon_primitives;

  auto chamber = irpcDigis->begin();
  auto chend   = irpcDigis->end();
  for( ; chamber != chend; ++chamber ) {
    auto digi = (*chamber).second.first;
    auto dend = (*chamber).second.second;
    for( ; digi != dend; ++digi ) {
      if ((*chamber).first.region() != 0) {  // 0 is barrel
        if ((*chamber).first.station() >= 3 && (*chamber).first.ring() == 1) {  // only RE3/1, RE4/1
          muon_primitives.emplace_back((*chamber).first,*digi);
        }
      }
    }
  }

  // Cluster the iRPC digis
  TriggerPrimitiveCollection clus_muon_primitives;
  cluster_rpc(muon_primitives, clus_muon_primitives);

  // Output
  std::copy(clus_muon_primitives.begin(), clus_muon_primitives.end(), std::back_inserter(out));
  return;
}

// Specialized for ME0
template<>
void EMTFSubsystemCollector::extractPrimitives(
    ME0Tag tag, // Defined in interface/EMTFSubsystemTag.h, maps to ME0Segment
    const GeometryTranslator* tp_geom,
    const edm::Event& iEvent,
    const edm::EDGetToken& token,
    TriggerPrimitiveCollection& out
) const {
  edm::Handle<ME0Tag::digi_collection> me0Digis;
  iEvent.getByToken(token, me0Digis);

  auto segment = me0Digis->begin();
  auto segend  = me0Digis->end();
  for( ; segment != segend; ++segment ) {
    // Debug
    //std::cout << "segment id: " << segment->me0DetId() << " lp: " << segment->localPosition() << " ld: " << segment->localDirection() << " time: " << segment->time() << " bend: " << segment->deltaPhi() << " chi2: " << segment->chi2() / float(segment->nRecHits()*2 - 4) << std::endl;
    //for (auto rechit = segment->specificRecHits().begin(); rechit != segment->specificRecHits().end(); ++rechit) {
    //  std::cout << "rechit id: " << rechit->me0Id() << " lp: " << rechit->localPosition() << " err: " << rechit->localPositionError() << " tof: " << rechit->tof() << " pad: " << tp_geom->getME0Geometry().etaPartition(rechit->me0Id())->pad(rechit->localPosition()) << std::endl;
    //}

    //out.emplace_back((*segment).me0DetId(), *segment, tp_geom->getME0Geometry());  // does not work because me0DetId is missing the eta partition number

    assert((*segment).specificRecHits().size() > 0);  // must contain at least 1 rechit
    ME0DetId detid = (*segment).me0DetId();
    if (detid.roll() == 0 || detid.layer() == 0) {
      auto rechit_it = (*segment).specificRecHits().begin();
      std::advance(rechit_it, (*segment).specificRecHits().size()/2);  // pick the median
      detid = ME0DetId(rechit_it->me0Id().region(), 3, rechit_it->me0Id().chamber(), rechit_it->me0Id().roll());  // use layer 3 as the key layer
    }
    out.emplace_back(detid, *segment, tp_geom->getME0Geometry());
  }
  return;
}


// _____________________________________________________________________________
// RPC functions
void EMTFSubsystemCollector::cluster_rpc(const TriggerPrimitiveCollection& muon_primitives, TriggerPrimitiveCollection& clus_muon_primitives) const {
  // Define operator to select RPC digis
  struct {
    typedef TriggerPrimitive value_type;
    bool operator()(const value_type& x) const {
      return (x.subsystem() == TriggerPrimitive::kRPC);
    }
  } rpc_digi_select;

  // Define operator to sort the RPC digis prior to clustering.
  // Use rawId, bx and strip as the sorting id. RPC rawId fully specifies
  // sector, subsector, endcap, station, ring, layer, roll. Strip is used as
  // the least significant sorting id.
  struct {
    typedef TriggerPrimitive value_type;
    bool operator()(const value_type& lhs, const value_type& rhs) const {
      bool cmp = (
          std::make_pair(std::make_pair(lhs.rawId(), lhs.getRPCData().bx), lhs.getRPCData().strip) <
          std::make_pair(std::make_pair(rhs.rawId(), rhs.getRPCData().bx), rhs.getRPCData().strip)
      );
      return cmp;
    }
  } rpc_digi_less;

  struct {
    typedef TriggerPrimitive value_type;
    bool operator()(const value_type& lhs, const value_type& rhs) const {
      bool cmp = (
          std::make_pair(std::make_pair(lhs.rawId(), lhs.getRPCData().bx), lhs.getRPCData().strip) ==
          std::make_pair(std::make_pair(rhs.rawId(), rhs.getRPCData().bx), rhs.getRPCData().strip)
      );
      return cmp;
    }
  } rpc_digi_equal;

  // Define operators for the nearest-neighbor clustering algorithm.
  // If two digis are next to each other (check strip_hi on the 'left', and
  // strip_low on the 'right'), cluster them (increment strip_hi on the 'left')
  struct {
    typedef TriggerPrimitive value_type;
    bool operator()(const value_type& lhs, const value_type& rhs) const {
      bool cmp = (
          (lhs.rawId() == rhs.rawId()) &&
          (lhs.getRPCData().bx == rhs.getRPCData().bx) &&
          (lhs.getRPCData().strip_hi+1 == rhs.getRPCData().strip_low)
      );
      return cmp;
    }
  } rpc_digi_adjacent;

  struct {
    typedef TriggerPrimitive value_type;
    void operator()(value_type& lhs, value_type& rhs) {  // pass by reference
      lhs.accessRPCData().strip_hi += 1;
    }
  } rpc_digi_cluster;

  // ___________________________________________________________________________
  // Do clustering using C++ <algorithm> functions

  // 1. Select RPC digis
  clus_muon_primitives.clear();
  std::copy_if(muon_primitives.begin(), muon_primitives.end(), std::back_inserter(clus_muon_primitives), rpc_digi_select);

  // 2. Sort
  std::stable_sort(clus_muon_primitives.begin(), clus_muon_primitives.end(), rpc_digi_less);

  // 3. Remove duplicates
  clus_muon_primitives.erase(
      std::unique(clus_muon_primitives.begin(), clus_muon_primitives.end(), rpc_digi_equal),
      clus_muon_primitives.end()
  );

  // 4. Cluster adjacent digis
  clus_muon_primitives.erase(
      adjacent_cluster(clus_muon_primitives.begin(), clus_muon_primitives.end(), rpc_digi_adjacent, rpc_digi_cluster),
      clus_muon_primitives.end()
  );
}


// _____________________________________________________________________________
// GEM functions
void EMTFSubsystemCollector::cluster_gem(const TriggerPrimitiveCollection& muon_primitives, TriggerPrimitiveCollection& clus_muon_primitives) const {
  // Define operator to select GEM digis
  struct {
    typedef TriggerPrimitive value_type;
    bool operator()(const value_type& x) const {
      return (x.subsystem() == TriggerPrimitive::kGEM);
    }
  } gem_digi_select;

  // Define operator to sort the GEM digis prior to clustering.
  // Use rawId, bx and pad as the sorting id. GEM rawId fully specifies
  // endcap, station, ring, layer, roll, chamber. Pad is used as
  // the least significant sorting id.
  struct {
    typedef TriggerPrimitive value_type;
    bool operator()(const value_type& lhs, const value_type& rhs) const {
      bool cmp = (
          std::make_pair(std::make_pair(lhs.rawId(), lhs.getGEMData().bx), lhs.getGEMData().pad) <
          std::make_pair(std::make_pair(rhs.rawId(), rhs.getGEMData().bx), rhs.getGEMData().pad)
      );
      return cmp;
    }
  } gem_digi_less;

  struct {
    typedef TriggerPrimitive value_type;
    bool operator()(const value_type& lhs, const value_type& rhs) const {
      bool cmp = (
          std::make_pair(std::make_pair(lhs.rawId(), lhs.getGEMData().bx), lhs.getGEMData().pad) ==
          std::make_pair(std::make_pair(rhs.rawId(), rhs.getGEMData().bx), rhs.getGEMData().pad)
      );
      return cmp;
    }
  } gem_digi_equal;

  // Define operators for the nearest-neighbor clustering algorithm.
  // If two digis are next to each other (check pad_hi on the 'left', and
  // pad_low on the 'right'), cluster them (increment pad_hi on the 'left')
  struct {
    typedef TriggerPrimitive value_type;
    bool operator()(const value_type& lhs, const value_type& rhs) const {
      bool cmp = (
          (lhs.rawId() == rhs.rawId()) &&
          (lhs.getGEMData().bx == rhs.getGEMData().bx) &&
          (lhs.getGEMData().pad_hi+1 == rhs.getGEMData().pad_low)
      );
      return cmp;
    }
  } gem_digi_adjacent;

  struct {
    typedef TriggerPrimitive value_type;
    void operator()(value_type& lhs, value_type& rhs) {  // pass by reference
      lhs.accessGEMData().pad_hi += 1;
    }
  } gem_digi_cluster;

  // ___________________________________________________________________________
  // Do clustering using C++ <algorithm> functions

  // 1. Select GEM digis
  clus_muon_primitives.clear();
  std::copy_if(muon_primitives.begin(), muon_primitives.end(), std::back_inserter(clus_muon_primitives), gem_digi_select);

  // 2. Sort
  std::stable_sort(clus_muon_primitives.begin(), clus_muon_primitives.end(), gem_digi_less);

  // 3. Remove duplicates
  clus_muon_primitives.erase(
      std::unique(clus_muon_primitives.begin(), clus_muon_primitives.end(), gem_digi_equal),
      clus_muon_primitives.end()
  );

  // 4. Cluster adjacent digis
  clus_muon_primitives.erase(
      adjacent_cluster(clus_muon_primitives.begin(), clus_muon_primitives.end(), gem_digi_adjacent, gem_digi_cluster),
      clus_muon_primitives.end()
  );
}

void EMTFSubsystemCollector::declusterize_gem(TriggerPrimitiveCollection& clus_muon_primitives, TriggerPrimitiveCollection& declus_muon_primitives) const {
  const unsigned int maxClusterSize = 8;
  const unsigned int maxClusters = 8;

  // Define operator to reject GEM clusters
  struct {
    typedef TriggerPrimitive value_type;
    bool operator()(const value_type& x) const {
      unsigned int sz = x.getGEMData().pad_hi - x.getGEMData().pad_low + 1;
      return sz > maxClusterSize;  // at most 8 pads
    }
  } gem_cluster_size_cut;

  // Define operator to select GEM clusters in a given layer
  struct {
    typedef TriggerPrimitive value_type;
    bool operator()(const value_type& x) const {
      const GEMDetId& detid = x.detId<GEMDetId>();
      return (x.subsystem() == TriggerPrimitive::kGEM && detid.layer() == 1);
    }
  } gem_cluster_layer1_select;

  struct {
    typedef TriggerPrimitive value_type;
    bool operator()(const value_type& x) const {
      const GEMDetId& detid = x.detId<GEMDetId>();
      return (x.subsystem() == TriggerPrimitive::kGEM && detid.layer() == 2);
    }
  } gem_cluster_layer2_select;


  // ___________________________________________________________________________
  // 1. Reject clusters with width > 8 pads
  clus_muon_primitives.erase(
      std::remove_if(clus_muon_primitives.begin(), clus_muon_primitives.end(), gem_cluster_size_cut),
      clus_muon_primitives.end()
  );

  // 2. For each of the 2 layers, pick a maximum of 8 pad clusters.
  TriggerPrimitiveCollection tmp_clus_muon_primitives;
  copy_n_if(clus_muon_primitives.begin(), clus_muon_primitives.end(), maxClusters, std::back_inserter(tmp_clus_muon_primitives), gem_cluster_layer1_select);
  copy_n_if(clus_muon_primitives.begin(), clus_muon_primitives.end(), maxClusters, std::back_inserter(tmp_clus_muon_primitives), gem_cluster_layer2_select);

  // 3. Declusterize
  declus_muon_primitives.clear();

  TriggerPrimitiveCollection::const_iterator tp_it  = tmp_clus_muon_primitives.begin();
  TriggerPrimitiveCollection::const_iterator tp_end = tmp_clus_muon_primitives.end();

  for (; tp_it != tp_end; ++tp_it) {
    for (uint16_t pad = tp_it->getGEMData().pad_low; pad != tp_it->getGEMData().pad_hi+1; ++pad) {
      TriggerPrimitive new_tp = *tp_it;  // make a copy
      new_tp.accessGEMData().pad     = pad;
      new_tp.accessGEMData().pad_low = pad;
      new_tp.accessGEMData().pad_hi  = pad;
      declus_muon_primitives.push_back(new_tp);
    }
  }
}

void EMTFSubsystemCollector::make_copad_gem(TriggerPrimitiveCollection& declus_muon_primitives, TriggerPrimitiveCollection& copad_muon_primitives) const {
  // Use the inner layer (layer 1) hit coordinates as output, and the outer
  // layer (layer 2) as coincidence
  // Copied from: L1Trigger/CSCTriggerPrimitives/src/GEMCoPadProcessor.cc

  const unsigned int maxDeltaBX = 1;
  const unsigned int maxDeltaRoll = 1;
  const unsigned int maxDeltaPadGE11 = 3;  // it was 2
  const unsigned int maxDeltaPadGE21 = 2;

  // Make sure that the difference is calculated using signed integer, and
  // output the absolute difference (as unsigned integer)
  auto calculate_delta = [](int a, int b) -> unsigned int {
    return std::abs(a-b);
  };

  // Create maps of GEM pads (key = detid), split by layer
  std::map<uint32_t, TriggerPrimitiveCollection> in_pads_layer1, in_pads_layer2;

  TriggerPrimitiveCollection::const_iterator tp_it  = declus_muon_primitives.begin();
  TriggerPrimitiveCollection::const_iterator tp_end = declus_muon_primitives.end();

  for (; tp_it != tp_end; ++tp_it) {
    GEMDetId detid = tp_it->detId<GEMDetId>();
    assert(detid.layer() == 1 || detid.layer() == 2);
    assert(1 <= detid.roll() && detid.roll() <= 8);
    uint32_t layer = detid.layer();

    // Remove layer number and roll number from detid
    detid = GEMDetId(detid.region(), detid.ring(), detid.station(), 0, detid.chamber(), 0);

    if (layer == 1) {
      in_pads_layer1[detid.rawId()].push_back(*tp_it);
    } else {
      in_pads_layer2[detid.rawId()].push_back(*tp_it);
    }
  }

  // Build coincidences
  copad_muon_primitives.clear();

  std::map<uint32_t, TriggerPrimitiveCollection>::iterator map_tp_it  = in_pads_layer1.begin();
  std::map<uint32_t, TriggerPrimitiveCollection>::iterator map_tp_end = in_pads_layer1.end();

  for (; map_tp_it != map_tp_end; ++map_tp_it) {
    const GEMDetId& detid = map_tp_it->first;
    const TriggerPrimitiveCollection& pads = map_tp_it->second;

    // find all corresponding ids with layer 2
    auto found = in_pads_layer2.find(detid);

    // empty range = no possible coincidence pads
    if (found == in_pads_layer2.end())  continue;

    // now let's correlate the pads in two layers
    const TriggerPrimitiveCollection& co_pads = found->second;
    for (TriggerPrimitiveCollection::const_iterator p = pads.begin(); p != pads.end(); ++p) {
      bool has_copad = false;
      int bend = 999999;

      for (TriggerPrimitiveCollection::const_iterator co_p = co_pads.begin(); co_p != co_pads.end(); ++co_p) {
        unsigned int deltaPad  = calculate_delta(p->getGEMData().pad, co_p->getGEMData().pad);
        unsigned int deltaBX   = calculate_delta(p->getGEMData().bx, co_p->getGEMData().bx);
        unsigned int deltaRoll = calculate_delta(p->detId<GEMDetId>().roll(), co_p->detId<GEMDetId>().roll());

        // check the match in pad
        if ((detid.station() == 1 && deltaPad > maxDeltaPadGE11) || (detid.station() == 2 && deltaPad > maxDeltaPadGE21))
          continue;

        // check the match in BX
        if (deltaBX > maxDeltaBX)
          continue;

        // check the match in roll
        if (deltaRoll > maxDeltaRoll)
          continue;

        has_copad = true;
        if (std::abs(bend) > deltaPad) {
          if (co_p->getGEMData().pad >= p->getGEMData().pad)
            bend = deltaPad;
          else
            bend = -deltaPad;
        }
      }  // end loop over co_pads

      // Need to flip the bend sign depending on the parity
      bool isEven = (detid.chamber() % 2 == 0);
      if (!isEven) {
        bend = -bend;
      }

      // make a new coincidence pad digi
      if (has_copad) {
        copad_muon_primitives.push_back(*p);
        copad_muon_primitives.back().accessGEMData().bend = bend;  // overwrites the bend
      }
    }  // end loop over pads
  }  // end loop over in_pads_layer1
}


// _____________________________________________________________________________
// Experimental features! Very unstable!!
namespace experimental {

// Specialized for CSC
template<>
void EMTFSubsystemCollector::extractPrimitives(
    CSCTag tag, // Defined in interface/EMTFSubsystemTag.h, maps to CSCCorrelatedLCTDigi
    const GeometryTranslator* tp_geom,
    const edm::Event& iEvent,
    const edm::EDGetToken& token_lct,        // for CSC
    const edm::EDGetToken& token_comparator, // for CSC
    TriggerPrimitiveCollection& out
) const {
  edm::Handle<CSCTag::digi_collection> cscDigis;
  iEvent.getByToken(token_lct, cscDigis);

  edm::Handle<CSCTag::comparator_digi_collection> cscComparatorDigis;
  iEvent.getByToken(token_comparator, cscComparatorDigis);

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

}
