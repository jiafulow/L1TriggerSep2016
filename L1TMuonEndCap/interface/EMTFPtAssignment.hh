#ifndef L1TMuonEndCap_EMTFPtAssignment_hh
#define L1TMuonEndCap_EMTFPtAssignment_hh

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFCommon.hh"


class EMTFPtAssignment {
public:
  void configure(
    int endcap, int sector, int bx
  );

private:
  int endcap_, sector_, bx_;
};

#endif
