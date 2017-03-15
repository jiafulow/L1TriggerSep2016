#ifndef L1TMuonEndCap_EMTFPtAssignment_hh
#define L1TMuonEndCap_EMTFPtAssignment_hh

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFCommon.hh"


class EMTFPtAssignmentEngine;
class EMTFPtAssignmentEngineAux;

class EMTFPtAssignment {
public:
  void configure(
      const EMTFPtAssignmentEngine* pt_assign_engine,
      int verbose, int endcap, int sector, int bx,
      bool readPtLUTFile, bool fixMode15HighPt,
      bool bug9BitDPhi, bool bugMode7CLCT, bool bugNegPt,
      bool bugGMTPhi
  );

  void process(
      EMTFTrackExtraCollection& best_tracks
  );

  const EMTFPtAssignmentEngineAux& aux() const;

private:
  EMTFPtAssignmentEngine* pt_assign_engine_;

  int verbose_, endcap_, sector_, bx_;

  bool bugGMTPhi_;
};

#endif
