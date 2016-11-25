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
  typedef EMTFPatternRecognition::pattern_ref_t pattern_ref_t;

  void configure(
      const EMTFSectorProcessorLUT* lut,
      const EMTFPtAssignmentEngine* pt_assign_engine,
      int verbose, int endcap, int sector,
      int minBX, int maxBX, int bxWindow, int bxShiftCSC, int bxShiftRPC,
      const std::vector<int>& zoneBoundaries, int zoneOverlap, int zoneOverlapRPC,
      bool includeNeighbor, bool duplicateTheta, bool fixZonePhi, bool useNewZones,
      const std::vector<std::string>& pattDefinitions, const std::vector<std::string>& symPattDefinitions, int thetaWindow, int thetaWindowRPC, bool useSymPatterns,
      int maxRoadsPerZone, int maxTracks, bool useSecondEarliest,
      bool readPtLUTFile, bool fixMode15HighPt, bool bug9BitDPhi, bool bugMode7CLCT, bool bugNegPt
  );

  void process(
      // Input
      EventNumber_t ievent,
      const std::unique_ptr<L1TMuonEndCap::GeometryTranslator>& tp_geom,
      const TriggerPrimitiveCollection& muon_primitives,
      // Output
      EMTFHitExtraCollection& out_hits,
      EMTFTrackExtraCollection& out_tracks
  ) const;

  void process_single_bx(
      // Input
      int bx,
      const std::unique_ptr<L1TMuonEndCap::GeometryTranslator>& tp_geom,
      const TriggerPrimitiveCollection& muon_primitives,
      // Output
      EMTFHitExtraCollection& out_hits,
      EMTFTrackExtraCollection& out_tracks,
      // Intermediate objects
      std::deque<EMTFHitExtraCollection>& extended_conv_hits,
      std::deque<EMTFTrackExtraCollection>& extended_best_track_cands,
      std::map<pattern_ref_t, int>& patt_lifetime_map
  ) const;

private:
  const EMTFSectorProcessorLUT* lut_;

  const EMTFPtAssignmentEngine* pt_assign_engine_;

  int verbose_, endcap_, sector_;

  int minBX_, maxBX_, bxWindow_, bxShiftCSC_, bxShiftRPC_;

  // For primitive conversion
  std::vector<int> zoneBoundaries_;
  int zoneOverlap_, zoneOverlapRPC_;
  bool includeNeighbor_, duplicateTheta_, fixZonePhi_, useNewZones_;

  // For pattern recognition
  std::vector<std::string> pattDefinitions_, symPattDefinitions_;
  int thetaWindow_, thetaWindowRPC_;
  bool useSymPatterns_;

  // For ghost cancellation
  int maxRoadsPerZone_, maxTracks_;
  bool useSecondEarliest_;

  // For pt assignment
  bool readPtLUTFile_, fixMode15HighPt_;
  bool bug9BitDPhi_, bugMode7CLCT_, bugNegPt_;
};

#endif
