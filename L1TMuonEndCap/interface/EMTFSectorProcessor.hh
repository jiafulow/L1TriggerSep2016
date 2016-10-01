#ifndef L1TMuonEndCap_EMTFSectorProcessor_hh
#define L1TMuonEndCap_EMTFSectorProcessor_hh

#include <deque>
#include <map>
#include <string>
#include <vector>

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFCommon.hh"

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFSectorProcessorLUT.hh"
#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPtAssignmentEngine.hh"

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPrimitiveSelection.hh"
#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPrimitiveConversion.hh"
#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPatternRecognition.hh"
#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPrimitiveMatching.hh"
#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFAngleCalculation.hh"
#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFBestTrackSelection.hh"
#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPtAssignment.hh"


class EMTFSectorProcessor {
public:
  explicit EMTFSectorProcessor();
  ~EMTFSectorProcessor();

  typedef unsigned long long EventNumber_t;

  void configure(
      const EMTFSectorProcessorLUT* lut,
      const EMTFPtAssignmentEngine* pt_assign_engine,
      int verbose, int minBX, int maxBX, int bxWindow,
      int endcap, int sector,
      bool includeNeighbor, bool duplicateTheta, bool fixZonePhi,
      const std::vector<int>& zoneBoundaries1, const std::vector<int>& zoneBoundaries2, int zoneOverlap,
      const std::vector<std::string>& pattDefinitions,
      int maxRoadsPerZone, int thetaWindow, int maxTracks
  );

  void process(
      EventNumber_t ievent,
      const TriggerPrimitiveCollection& muon_primitives,
      EMTFHitExtraCollection& out_hits,
      EMTFTrackExtraCollection& out_tracks
  ) const;

  void process_single_bx(
      int bx,
      const TriggerPrimitiveCollection& muon_primitives,
      EMTFHitExtraCollection& out_hits,
      EMTFTrackExtraCollection& out_tracks,
      std::deque<EMTFHitExtraCollection>& extended_conv_hits,
      std::map<EMTFPatternRef, int>& patt_lifetime_map
  ) const;

  int sector() const { return sector_; }

private:
  const EMTFSectorProcessorLUT* lut_;

  const EMTFPtAssignmentEngine* pt_assign_engine_;

  int verbose_, minBX_, maxBX_, bxWindow_;

  int endcap_, sector_;

  bool includeNeighbor_, duplicateTheta_, fixZonePhi_;

  std::vector<int> zoneBoundaries1_, zoneBoundaries2_;
  int zoneOverlap_;
  std::vector<std::string> pattDefinitions_;
  int maxRoadsPerZone_, thetaWindow_, maxTracks_;
};

#endif
