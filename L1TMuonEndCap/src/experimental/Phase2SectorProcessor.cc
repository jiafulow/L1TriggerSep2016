#include "L1Trigger/L1TMuonEndCap/interface/experimental/Phase2SectorProcessor.h"


// _____________________________________________________________________________
// This implements a TEMPORARY version of the Phase 2 EMTF sector processor.
// It is supposed to be replaced in the future. It is intentionally written
// in a monolithic fashion to allow easy replacement.
//


namespace experimental {

void Phase2SectorProcessor::configure(
    // Object pointers
    const GeometryTranslator* geom,
    const ConditionHelper* cond,
    const SectorProcessorLUT* lut,
    // Sector processor config
    int verbose, int endcap, int sector, int bx,
    int bxShiftCSC, int bxShiftRPC, int bxShiftGEM,
    std::string era
) {
  assert(emtf::MIN_ENDCAP <= endcap && endcap <= emtf::MAX_ENDCAP);
  assert(emtf::MIN_TRIGSECTOR <= sector && sector <= emtf::MAX_TRIGSECTOR);

  assert(geom != nullptr);
  assert(cond != nullptr);
  assert(lut  != nullptr);

  geom_       = geom;
  cond_       = cond;
  lut_        = lut;

  verbose_    = verbose;
  endcap_     = endcap;
  sector_     = sector;
  bx_         = bx;

  bxShiftCSC_ = bxShiftCSC;
  bxShiftRPC_ = bxShiftRPC;
  bxShiftGEM_ = bxShiftGEM;

  era_        = era;
}

void Phase2SectorProcessor::process(
    // Input
    const edm::Event& iEvent, const edm::EventSetup& iSetup,
    const TriggerPrimitiveCollection& muon_primitives,
    // Output
    EMTFHitCollection& out_hits,
    EMTFTrackCollection& out_tracks
) const {

  // ___________________________________________________________________________
  // Primitive selection & primitive conversion
  // (shared with current EMTF)

  bool includeNeighbor  = true;
  bool duplicateTheta   = true;
  bool bugME11Dupes     = false;

  std::vector<int> zoneBoundaries = {0, 41, 49, 87, 127};
  int zoneOverlap       = 2;
  bool fixZonePhi       = true;
  bool useNewZones      = false;
  bool fixME11Edges     = true;

  PrimitiveSelection prim_sel;
  prim_sel.configure(
      verbose_, endcap_, sector_, bx_,
      bxShiftCSC_, bxShiftRPC_, bxShiftGEM_,
      includeNeighbor, duplicateTheta,
      bugME11Dupes
  );

  PrimitiveConversion prim_conv;
  prim_conv.configure(
      geom_, lut_,
      verbose_, endcap_, sector_, bx_,
      bxShiftCSC_, bxShiftRPC_, bxShiftGEM_,
      zoneBoundaries, zoneOverlap,
      duplicateTheta, fixZonePhi, useNewZones, fixME11Edges,
      bugME11Dupes
  );

  // ___________________________________________________________________________
  // Input

  EMTFHitCollection conv_hits;     // "converted" hits converted by primitive converter
  EMTFTrackCollection best_tracks; // "best" tracks selected from all the zones

  std::map<int, TriggerPrimitiveCollection> selected_dt_map;
  std::map<int, TriggerPrimitiveCollection> selected_csc_map;
  std::map<int, TriggerPrimitiveCollection> selected_rpc_map;
  std::map<int, TriggerPrimitiveCollection> selected_gem_map;
  std::map<int, TriggerPrimitiveCollection> selected_me0_map;
  std::map<int, TriggerPrimitiveCollection> selected_prim_map;
  std::map<int, TriggerPrimitiveCollection> inclusive_selected_prim_map;

  // Select muon primitives that belong to this sector and this BX.
  // Put them into maps with an index that roughly corresponds to
  // each input link.
  prim_sel.process(DTTag(), muon_primitives, selected_dt_map);
  prim_sel.process(CSCTag(), muon_primitives, selected_csc_map);
  prim_sel.process(RPCTag(), muon_primitives, selected_rpc_map);
  prim_sel.process(GEMTag(), muon_primitives, selected_gem_map);
  prim_sel.process(ME0Tag(), muon_primitives, selected_me0_map);
  prim_sel.merge(selected_dt_map, selected_csc_map, selected_rpc_map, selected_gem_map, selected_me0_map, selected_prim_map);

  // Convert trigger primitives into "converted" hits
  // A converted hit consists of integer representations of phi, theta, and zones
  prim_conv.process(selected_prim_map, conv_hits);

  // ___________________________________________________________________________
  // Build

  build(conv_hits, best_tracks);

  // ___________________________________________________________________________
  // Output

  out_hits.insert(out_hits.end(), conv_hits.begin(), conv_hits.end());
  out_tracks.insert(out_tracks.end(), best_tracks.begin(), best_tracks.end());

  return;
}

// _____________________________________________________________________________
// Specific data formats
// (adapted from rootpy_trackbuilding9.py)

constexpr int NLAYERS = 16;  // 5 (CSC) + 4 (RPC) + 3 (GEM) + 4 (DT)
constexpr int NPREDS = 2;    // pT, PU discr

constexpr int ROAD_LAYER_NVARS = 10;  // each layer in the road carries 10 variables
constexpr int ROAD_LAYER_NVARS_P1 = ROAD_LAYER_NVARS + 1;  // plus layer mask
constexpr int ROAD_INFO_NVARS = 3;

constexpr int PATTERN_BANK_NPT = 18;   // straightness
constexpr int PATTERN_BANK_NETA = 7;   // zone
constexpr int PATTERN_BANK_NLAYERS = NLAYERS;
constexpr int PATTERN_BANK_NVARS = 3;  // min, med, max

class Hit {
public:
  explicit Hit(int16_t vh_type, int16_t vh_station, int16_t vh_ring,
               int16_t vh_sector, int16_t vh_fr, int16_t vh_bx,
               int32_t vh_emtf_layer, int32_t vh_emtf_phi, int32_t vh_emtf_theta,
               int32_t vh_emtf_bend, int32_t vh_emtf_qual, int32_t vh_emtf_time,
               int32_t vh_old_emtf_phi, int32_t vh_old_emtf_bend, int32_t vh_extra_emtf_theta,
               int32_t vh_sim_tp, int32_t vh_ref)
  {
    type             = vh_type;
    station          = vh_station;
    ring             = vh_ring;
    sector           = vh_sector;
    fr               = vh_fr;
    bx               = vh_bx;
    emtf_layer       = vh_emtf_layer;
    emtf_phi         = vh_emtf_phi;
    emtf_theta       = vh_emtf_theta;
    emtf_bend        = vh_emtf_bend;
    emtf_qual        = vh_emtf_qual;
    emtf_time        = vh_emtf_time;
    old_emtf_phi     = vh_old_emtf_phi;
    old_emtf_bend    = vh_old_emtf_bend;
    extra_emtf_theta = vh_extra_emtf_theta;
    sim_tp           = vh_sim_tp;
    ref              = vh_ref;
  }

  // Properties
  int16_t type;
  int16_t station;
  int16_t ring;
  int16_t sector;
  int16_t fr;
  int16_t bx;
  int32_t emtf_layer;
  int32_t emtf_phi;
  int32_t emtf_theta;
  int32_t emtf_bend;
  int32_t emtf_qual;
  int32_t emtf_time;
  int32_t old_emtf_phi;
  int32_t old_emtf_bend;
  int32_t extra_emtf_theta;
  int32_t sim_tp;
  int32_t ref;
};

class Road {
public:
  explicit Road(int16_t vr_endcap, int16_t vr_sector, int16_t vr_ipt, int16_t vr_ieta, int16_t vr_iphi,
                const std::vector<Hit>& vr_hits, int16_t vr_mode, int16_t vr_quality,
                int16_t vr_sort_code, int32_t vr_theta_median)
  {
    endcap       = vr_endcap;
    sector       = vr_sector;
    ipt          = vr_ipt;
    ieta         = vr_ieta;
    iphi         = vr_iphi;
    hits         = vr_hits;
    mode         = vr_mode;
    quality      = vr_quality;
    sort_code    = vr_sort_code;
    theta_median = vr_theta_median;
  }

  // Properties
  int16_t endcap;
  int16_t sector;
  int16_t ipt;
  int16_t ieta;
  int16_t iphi;
  std::vector<Hit> hits;
  int16_t mode;
  int16_t quality;
  int16_t sort_code;
  int32_t theta_median;
};

class Track {
public:
  explicit Track(int16_t vt_endcap, int16_t vt_sector,
                 const std::vector<Hit>& vt_hits, int16_t vt_mode, int16_t vt_quality, int16_t vt_zone,
                 float vt_xml_pt, float vt_pt, int16_t vt_q, int16_t vt_ndof, float vt_chi2,
                 int32_t vt_emtf_phi, int32_t vt_emtf_theta, float vt_glob_phi, float vt_glob_eta)
  {
    endcap     = vt_endcap;
    sector     = vt_sector;
    hits       = vt_hits;
    mode       = vt_mode;
    quality    = vt_quality;
    zone       = vt_zone;
    xml_pt     = vt_xml_pt;
    pt         = vt_pt;
    q          = vt_q;
    ndof       = vt_ndof;
    chi2       = vt_chi2;
    emtf_phi   = vt_emtf_phi;
    emtf_theta = vt_emtf_theta;
    glob_phi   = vt_glob_phi;
    glob_eta   = vt_glob_eta;
  }

  // Properties
  int16_t endcap;
  int16_t sector;
  std::vector<Hit> hits;
  int16_t mode;
  int16_t quality;
  int16_t zone;
  float   xml_pt;
  float   pt;
  int16_t q;
  int16_t ndof;
  float   chi2;
  int32_t emtf_phi;
  int32_t emtf_theta;
  float   glob_phi;
  float   glob_eta;
};

typedef std::array<float, (ROAD_LAYER_NVARS_P1 * NLAYERS) + ROAD_INFO_NVARS> Variable;
typedef std::array<float, NPREDS> Prediction;


// _____________________________________________________________________________
// Specific modules
// (adapted from rootpy_trackbuilding9.py)

#include "patternbank.icc"

class PatternBank {
public:
  // Constructor
  explicit PatternBank() {
    // Initialize 4-D array
    for (size_t i=0; i<x_array.size(); i++) {
      for (size_t j=0; j<x_array[i].size(); j++) {
        for (size_t k=0; k<x_array[i][j].size(); k++) {
          for (size_t l=0; l<x_array[i][j][k].size(); l++) {
            x_array[i][j][k][l] = _patternbank[i][j][k][l];
          }
        }
      }
    }
  }  // end constructor

  // 4-D array of size [NLAYERS][NETA][NVARS][NPT]
  // Note: rearranged for cache-friendliness. In the original python script,
  // it's arranged as [NPT][NETA][NLAYERS][NVARS]
  typedef std::array<std::array<std::array<std::array<int32_t, PATTERN_BANK_NPT>,
      PATTERN_BANK_NVARS>, PATTERN_BANK_NETA>, PATTERN_BANK_NLAYERS> patternbank_t;

  patternbank_t x_array;
};

static const PatternBank bank;

class PatternRecognition {
public:
  void run(const EMTFHitCollection& conv_hits, std::vector<Road>& roads) const {
    return;
  }
private:
};

class RoadCleaning {
public:
  void run(const std::vector<Road>& roads, std::vector<Road>& clean_roads) const {
    return;
  }
private:
};

class RoadSlimming {
public:
  void run(const std::vector<Road>& clean_roads, std::vector<Road>& slim_roads) const {
    return;
  }
private:
};

class PtAssignment {
public:
  void run(const std::vector<Road>& slim_roads, std::vector<Variable>& variables, std::vector<Prediction>& predictions) const {
    return;
  }
private:
};

class TrackProducer {
public:
  void run(const std::vector<Road>& slim_roads, const std::vector<Variable>& variables, const std::vector<Prediction>& predictions, std::vector<Track>& tracks) const {
    return;
  }
private:
};

class GhostBusting {
public:
  void run(const std::vector<Track>& tracks, EMTFTrackCollection& best_tracks) const {
    return;
  }
private:
};

static const PatternRecognition recog;
static const RoadCleaning clean;
static const RoadSlimming slim;
static const PtAssignment assig;
static const TrackProducer trkprod;
static const GhostBusting ghost;

// _____________________________________________________________________________
void Phase2SectorProcessor::build(
    // Input
    const EMTFHitCollection& conv_hits,
    // Output
    EMTFTrackCollection& best_tracks
) const {

  std::vector<Road> roads, clean_roads, slim_roads;
  std::vector<Variable> variables;
  std::vector<Prediction> predictions;
  std::vector<Track> tracks;

  recog.run(conv_hits, roads);
  clean.run(roads, clean_roads);
  slim.run(clean_roads, slim_roads);
  assig.run(slim_roads, variables, predictions);
  trkprod.run(slim_roads, variables, predictions, tracks);
  ghost.run(tracks, best_tracks);

  return;
}

}  // namespace experimental
