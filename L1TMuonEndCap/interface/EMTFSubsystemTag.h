#ifndef L1TMuonEndCap_EMTFSubsystemTag_h
#define L1TMuonEndCap_EMTFSubsystemTag_h

#include "DataFormats/L1DTTrackFinder/interface/L1MuDTChambPhDigi.h"
#include "DataFormats/L1DTTrackFinder/interface/L1MuDTChambPhContainer.h"
#include "DataFormats/L1DTTrackFinder/interface/L1MuDTChambThDigi.h"
#include "DataFormats/L1DTTrackFinder/interface/L1MuDTChambThContainer.h"
#include "DataFormats/CSCDigi/interface/CSCCorrelatedLCTDigi.h"
#include "DataFormats/CSCDigi/interface/CSCCorrelatedLCTDigiCollection.h"
#include "DataFormats/CSCDigi/interface/CSCComparatorDigi.h"
#include "DataFormats/CSCDigi/interface/CSCComparatorDigiCollection.h"
#include "DataFormats/RPCDigi/interface/RPCDigi.h"
#include "DataFormats/RPCDigi/interface/RPCDigiCollection.h"
#include "DataFormats/GEMDigi/interface/GEMPadDigi.h"
#include "DataFormats/GEMDigi/interface/GEMPadDigiCollection.h"
#include "DataFormats/GEMRecHit/interface/ME0Segment.h"
#include "DataFormats/GEMRecHit/interface/ME0SegmentCollection.h"


namespace emtf {

  struct DTTag {
    typedef L1MuDTChambPhDigi      digi_type;
    typedef L1MuDTChambPhContainer digi_collection;
    typedef L1MuDTChambThDigi      theta_digi_type;
    typedef L1MuDTChambThContainer theta_digi_collection;
  };

  struct CSCTag {
    typedef CSCCorrelatedLCTDigi           digi_type;
    typedef CSCCorrelatedLCTDigiCollection digi_collection;
    typedef CSCComparatorDigi              comparator_digi_type;
    typedef CSCComparatorDigiCollection    comparator_digi_collection;
  };

  struct RPCTag {
    typedef RPCDigi           digi_type;
    typedef RPCDigiCollection digi_collection;
  };

  struct GEMTag {
    typedef GEMPadDigi           digi_type;
    typedef GEMPadDigiCollection digi_collection;
  };

  struct IRPCTag {
    typedef RPCDigi           digi_type;
    typedef RPCDigiCollection digi_collection;
  };

  struct ME0Tag {
    typedef ME0Segment           digi_type;
    typedef ME0SegmentCollection digi_collection;
  };

}  //  namespace emtf

#endif
