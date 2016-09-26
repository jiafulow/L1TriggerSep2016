#ifndef L1TMuonEndCap_EMTFPtAssignment_hh
#define L1TMuonEndCap_EMTFPtAssignment_hh

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFCommon.hh"


class EMTFPtAssignmentEngine;

class EMTFPtAssignment {
public:
  void configure(
    const EMTFPtAssignmentEngine* pt_assign_engine,
    int endcap, int sector, int bx
  );

  void process(EMTFTrackExtraCollection& best_tracks);

private:
  EMTFPtAssignmentEngine* pt_assign_engine_;

  int endcap_, sector_, bx_;
};

#endif
