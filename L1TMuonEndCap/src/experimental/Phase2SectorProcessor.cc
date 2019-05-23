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

constexpr int PATTERN_X_CENTRAL = 31;  // pattern bin number 31 is the central
constexpr int PATTERN_X_SEARCH_MIN = 33;
//constexpr int PATTERN_X_SEARCH_MAX = 154-10;
constexpr int PATTERN_X_SEARCH_MAX = 154-10+12;  // account for DT


class Hit {
public:
  explicit Hit(int16_t vh_type, int16_t vh_station, int16_t vh_ring,
               int16_t vh_endsec, int16_t vh_fr, int16_t vh_bx,
               int32_t vh_emtf_layer, int32_t vh_emtf_phi, int32_t vh_emtf_theta,
               int32_t vh_emtf_bend, int32_t vh_emtf_qual, int32_t vh_emtf_time,
               int32_t vh_old_emtf_phi, int32_t vh_old_emtf_bend, int32_t vh_extra_emtf_theta,
               int32_t vh_sim_tp, int32_t vh_ref)
  {
    type             = vh_type;
    station          = vh_station;
    ring             = vh_ring;
    endsec           = vh_endsec;
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
  int16_t endsec;
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
  typedef std::vector<Hit> road_hits_t;

  // road_id = (endcap, sector, ipt, ieta, iphi)
  typedef std::array<int32_t, 5> road_id_t;

  explicit Road(int16_t vr_endcap, int16_t vr_sector, int16_t vr_ipt, int16_t vr_ieta, int16_t vr_iphi,
                const road_hits_t& vr_hits, int16_t vr_mode, int16_t vr_quality,
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
  road_hits_t hits;
  int16_t mode;
  int16_t quality;
  int16_t sort_code;
  int32_t theta_median;
};

class Track {
public:
  typedef std::vector<Hit> road_hits_t;

  explicit Track(int16_t vt_endcap, int16_t vt_sector,
                 const road_hits_t& vt_hits, int16_t vt_mode, int16_t vt_quality, int16_t vt_zone,
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
  road_hits_t hits;
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
// Specific functions
// (adapted from rootpy_trackbuilding9.py)

template<typename Container, typename Predicate>
Container my_filter(Predicate pred, const Container& input) {
  Container output;
  std::copy_if(input.begin(), input.end(), std::back_inserter(output), pred);
  return output;
}


// _____________________________________________________________________________
// Specific modules
// (adapted from rootpy_trackbuilding9.py)

#include "utility.icc"

class Utility {
public:
  // Constructor
  explicit Utility() {
    // Initialize 3-D array
    for (size_t i=0; i<find_emtf_layer_lut.size(); i++) {
      for (size_t j=0; j<find_emtf_layer_lut[i].size(); j++) {
        for (size_t k=0; k<find_emtf_layer_lut[i][j].size(); k++) {
          find_emtf_layer_lut[i][j][k] = _find_emtf_layer_lut[i][j][k];
        }
      }
    }

    // Initialize 5-D array
    for (size_t i=0; i<find_emtf_zones_lut.size(); i++) {
      for (size_t j=0; j<find_emtf_zones_lut[i].size(); j++) {
        for (size_t k=0; k<find_emtf_zones_lut[i][j].size(); k++) {
          for (size_t l=0; l<find_emtf_zones_lut[i][j][k].size(); l++) {
            for (size_t m=0; m<find_emtf_zones_lut[i][j][k][l].size(); m++) {
              find_emtf_zones_lut[i][j][k][l][m] = _find_emtf_zones_lut[i][j][k][l][m];
            }
          }
        }
      }
    }
  }  // end constructor

  bool isFront_detail(int subsystem, int station, int ring, int chamber, int subsector) const {
    bool result = false;

    if (subsystem == TriggerPrimitive::kCSC) {
      bool isOverlapping = !(station == 1 && ring == 3);
      // not overlapping means back
      if(isOverlapping)
      {
        bool isEven = (chamber % 2 == 0);
        // odd chambers are bolted to the iron, which faces
        // forward in 1&2, backward in 3&4, so...
        result = (station < 3) ? isEven : !isEven;
      }
    } else if (subsystem == TriggerPrimitive::kRPC) {
      //// 10 degree rings have even subsectors in front
      //// 20 degree rings have odd subsectors in front
      //bool is_10degree = !((station == 3 || station == 4) && (ring == 1));
      //bool isEven = (subsector % 2 == 0);
      //result = (is_10degree) ? isEven : !isEven;

      // Use the equivalent CSC chamber F/R
      bool isEven = (chamber % 2 == 0);
      result = (station < 3) ? isEven : !isEven;
    } else if (subsystem == TriggerPrimitive::kGEM) {
      //
      result = (chamber % 2 == 0);
    } else if (subsystem == TriggerPrimitive::kME0) {
      //
      result = (chamber % 2 == 0);
    } else if (subsystem == TriggerPrimitive::kDT) {
      //
      result = (chamber % 2 == 0);
    }
    return result;
  }

  bool find_fr(const EMTFHit& conv_hit) const {
    return isFront_detail(conv_hit.Subsystem(), conv_hit.Station(), conv_hit.Ring(), conv_hit.Chamber(),
                          (conv_hit.Subsystem() == TriggerPrimitive::kRPC ? conv_hit.Subsector_RPC() : conv_hit.Subsector()));
  }

  int32_t find_endsec(int32_t endcap, int32_t sector) const {
    return (endcap == 1) ? (sector - 1) : (sector - 1 + 6);
  }

  int32_t find_endsec(const EMTFHit& conv_hit) const {
    int32_t endcap     = conv_hit.Endcap();
    int32_t sector     = conv_hit.Sector();
    return find_endsec(endcap, sector);
  }

  int32_t find_pattern_x(int32_t emtf_phi) const {
    return (emtf_phi+16)/32;  // divide by 'quadstrip' unit (4 * 8)
  }

  // Calculate transverse impact parameter, d0
  double calculate_d0(double invPt, double phi, double xv, double yv, double B=3.811) const {
    double _invPt = (std::abs(invPt) < 1./10000) ? (invPt < 0 ? -1./10000 : +1./10000) : invPt;
    double _R = -1.0 / (0.003 * B * _invPt);                          // R = -pT/(0.003 q B)  [cm]
    double _xc = xv - (_R * std::sin(phi));                           // xc = xv - R sin(phi)
    double _yc = yv + (_R * std::cos(phi));                           // yc = yv + R cos(phi)
    double _d0 = _R - ((_R < 0 ? -1. : +1.) * std::hypot(_xc, _yc));  // d0 = R - sign(R) * sqrt(xc^2 + yc^2)
    return _d0;
  }

  // Decide EMTF hit layer number
  int32_t find_emtf_layer(const EMTFHit& conv_hit) const {
    int32_t type       = conv_hit.Subsystem();
    int32_t station    = conv_hit.Station();
    int32_t ring       = conv_hit.Ring();

    int32_t emtf_layer = find_emtf_layer_lut[type][station][ring];
    return emtf_layer;
  }

  // Decide EMTF hit zones
  std::vector<int32_t> find_emtf_zones(const EMTFHit& conv_hit) const {
    std::vector<int32_t> zones;

    int32_t emtf_theta = conv_hit.Theta_fp();
    int32_t type       = conv_hit.Subsystem();
    int32_t station    = conv_hit.Station();
    int32_t ring       = conv_hit.Ring();

    for (size_t zone=0; zone<find_emtf_zones_lut[type][station][ring].size(); zone++) {
      int32_t low  = find_emtf_zones_lut[type][station][ring][zone][0];
      int32_t high = find_emtf_zones_lut[type][station][ring][zone][1];
      if ((low <= emtf_theta) && (emtf_theta <= high)) {
        zones.push_back(zone);
      }
    }
    return zones;
  }

  // Decide EMTF hit bend
  int32_t find_emtf_bend(const EMTFHit& conv_hit) const {
    int32_t emtf_bend  = conv_hit.Bend();
    int32_t type       = conv_hit.Subsystem();
    int32_t station    = conv_hit.Station();
    int32_t ring       = conv_hit.Ring();
    int32_t endcap     = conv_hit.Endcap();
    int32_t quality    = conv_hit.Quality();

    if (type == TriggerPrimitive::kCSC) {
      // Special case for ME1/1a
      // rescale the bend to the same scale as ME1/1b
      if ((station == 1) && (ring == 4)) {
        emtf_bend = static_cast<int32_t>(std::round(static_cast<float>(emtf_bend) * 0.026331/0.014264));
        emtf_bend = std::min(std::max(emtf_bend, -32), 31);
      }
      emtf_bend *= endcap;
      emtf_bend /= 2;  // from 1/32-strip unit to 1/16-strip unit

    } else if (type == TriggerPrimitive::kGEM) {
      emtf_bend *= endcap;

    } else if (type == TriggerPrimitive::kME0) {
      emtf_bend = std::min(std::max(emtf_bend, -64), 63);  // currently in 1/2-strip unit

    } else if (type == TriggerPrimitive::kDT) {
      if (quality >= 4) {
        emtf_bend = std::min(std::max(emtf_bend, -512), 511);
      } else {
        //emtf_bend = 0;
        emtf_bend = std::min(std::max(emtf_bend, -512), 511);
      }

    } else {  // type == TriggerPrimitive::kRPC
      emtf_bend = 0;
    }
    return emtf_bend;
  }

  // Decide EMTF hit bend (old version)
  // Not implemented
  int32_t find_emtf_old_bend(const EMTFHit& conv_hit) const { return 0; }

  // Decide EMTF hit phi (integer unit)
  int32_t find_emtf_phi(const EMTFHit& conv_hit) const {
    int32_t emtf_phi   = conv_hit.Phi_fp();
    int32_t type       = conv_hit.Subsystem();
    int32_t station    = conv_hit.Station();
    int32_t ring       = conv_hit.Ring();
    int32_t endcap     = conv_hit.Endcap();
    int32_t bend       = conv_hit.Bend();

    int32_t fr         = find_fr(conv_hit);

    if (type == TriggerPrimitive::kCSC) {
      if (station == 1) {
        float bend_corr = 0.;
        if (ring == 1) {
          bend_corr = ((static_cast<float>(1-fr) * -2.0832) + (static_cast<float>(fr) * 2.0497)) * bend;  // ME1/1b (r,f)
        } else if (ring == 4) {
          bend_corr = ((static_cast<float>(1-fr) * -2.4640) + (static_cast<float>(fr) * 2.3886)) * bend;  // ME1/1a (r,f)
        } else if (ring == 2) {
          bend_corr = ((static_cast<float>(1-fr) * -1.3774) + (static_cast<float>(fr) * 1.2447)) * bend;  // ME1/2 (r,f)
        } else {
          bend_corr = 0.;  // ME1/3 (r,f): no correction
        }
        bend_corr *= endcap;
        emtf_phi += static_cast<int32_t>(std::round(bend_corr));
      } else {
        // do nothing
      }
    } else {
      // do nothing
    }
    return emtf_phi;
  }

  // Decide EMTF hit phi (integer unit) (old version)
  // Not implemented
  int32_t find_emtf_old_phi(const EMTFHit& conv_hit) const { return 0; }

  // Decide EMTF hit theta (integer unit)
  int32_t find_emtf_theta(const EMTFHit& conv_hit) const {
    int32_t emtf_theta = conv_hit.Theta_fp();
    int32_t type       = conv_hit.Subsystem();
    int32_t station    = conv_hit.Station();
    int32_t wire       = conv_hit.Wire();
    int32_t quality    = conv_hit.Quality();

    if (type == TriggerPrimitive::kDT) {
      // wire -1 means no theta SL
      // quality 0&1 are RPC digis
      if ((wire == -1) || (quality < 2)) {
        if (station == 1) {
          emtf_theta = 112;
        } else if (station == 2) {
          emtf_theta = 122;
        } else if (station == 3) {
          emtf_theta = 131;
        }
      } else {
        // do nothing
      }
    } else {
      // do nothing
    }
    return emtf_theta;
  }

  // Decide EMTF hit z-position (floating-point)
  // Not implemented
  float   find_emtf_zee(const EMTFHit& conv_hit) const { return 0.; }

  // Decide EMTF hit quality
  int32_t find_emtf_qual(const EMTFHit& conv_hit) const {
    int32_t emtf_qual  = conv_hit.Quality();
    int32_t type       = conv_hit.Subsystem();

    int32_t fr         = find_fr(conv_hit);

    if ((type == TriggerPrimitive::kCSC) || (type == TriggerPrimitive::kME0)) {
      // front chamber  -> +1
      // rear chamber   -> -1
      if (fr == 1) {
        emtf_qual *= +1;
      } else {
        emtf_qual *= -1;
      }
    } else if ((type == TriggerPrimitive::kRPC) || (type == TriggerPrimitive::kGEM)) {
      emtf_qual = 0;
    } else {  // type == TriggerPrimitive::kDT
      // do nothing
    }
    return emtf_qual;
  }

  // Decide EMTF hit time (integer unit)
  int32_t find_emtf_time(const EMTFHit& conv_hit) const {
    //int32_t emtf_time  = static_cast<int32_t>(std::round(conv_hit.Time() * 16./25));  // integer unit is 25ns/16 (4-bit)
    int32_t emtf_time  = conv_hit.BX();
    return emtf_time;
  }

  // Decide EMTF hit layer partner (to make pairs and calculate deflection angles)
  int32_t find_emtf_layer_partner(int32_t emtf_layer, int32_t zone) const {
    static const int32_t lut[NLAYERS] = {2, 2, 0, 0, 0, 0, 2, 3, 4, 0, 2, 0, 0, 0, 0, 0};

    int32_t partner = lut[emtf_layer];
    if (zone >= 5) {  // zones 5&6, use ME1/2
      if (partner == 0) {
        partner = 1;
      }
    }
    return partner;
  }

  // Decide EMTF road quality (by pattern straightness)
  int32_t find_emtf_road_quality(int32_t ipt) const {
    static const int32_t best_ipt = 9/2;  // straightest pattern out of 9 patterns

    int32_t quality = best_ipt - std::abs(ipt - best_ipt);
    return quality;
  }

  // Decide EMTF road sort code (by hit composition)
  int32_t find_emtf_road_sort_code(int32_t road_mode, int32_t road_quality, const std::vector<Hit>& road_hits) const {
    // 9    8      7      6    5      4    3    2..0
    //      ME1/1  ME1/2  ME2         ME3  ME4  qual
    //                         RE1&2  RE3  RE4
    // ME0         GE1/1       GE2/1
    // MB1  MB2                MB3&4
    static const int32_t lut[NLAYERS] = {8,7,6,4,3,5,5,4,3,7,5,9,9,8,5,5};

    int32_t sort_code = 0;
    for (const auto& hit : road_hits) {
      int32_t hit_lay = hit.emtf_layer;
      int32_t mlayer = lut[hit_lay];
      sort_code |= (1 << mlayer);
    }
    sort_code |= road_quality;
    return sort_code;
  }

  bool is_emtf_singlemu(int mode) const {
    static const std::set<int> s {11,13,14,15};
    return (s.find(mode) != s.end());  // s.contains(mode);
  }

  bool is_emtf_doublemu(int mode) const {
    //static const std::set<int> s {7,10,12,11,13,14,15};
    static const std::set<int> s {9,10,12,11,13,14,15};  // replace 2-3-4 with 1-4
    return (s.find(mode) != s.end());  // s.contains(mode);
  }

  bool is_emtf_muopen(int mode) const {
    static const std::set<int> s {3,5,6,9,7,10,12,11,13,14,15};
    return (s.find(mode) != s.end());  // s.contains(mode);
  }

  bool is_emtf_singlehit(int mode) const {
    return bool(mode & (1 << 3));
  }

  bool is_emtf_singlehit_me2(int mode) const {
    return bool(mode & (1 << 2));
  }

  // For now, only consider BX=0
  bool is_emtf_legit_hit_check_bx(const EMTFHit& conv_hit) const {
    int32_t type       = conv_hit.Subsystem();
    int32_t bx         = conv_hit.BX();

    if (type == TriggerPrimitive::kCSC) {
      return (bx == -1) || (bx == 0);
    } else if (type == TriggerPrimitive::kDT) {
      return (bx == -1) || (bx == 0);
    }
    return (bx == 0);
  }

  bool is_emtf_legit_hit_check_phi(const EMTFHit& conv_hit) const {
    int32_t type       = conv_hit.Subsystem();
    int32_t emtf_phi   = conv_hit.Phi_fp();

    if (type == TriggerPrimitive::kME0) {
      return (emtf_phi > 0);
    } else if (type == TriggerPrimitive::kDT) {
      return (emtf_phi > 0);
    }
    return true;
  }

  bool is_emtf_legit_hit(const EMTFHit& conv_hit) const {
    return is_emtf_legit_hit_check_bx(conv_hit) && is_emtf_legit_hit_check_phi(conv_hit);
  }

private:
  // 3-D array of size [# types][# stations][# rings]
  typedef std::array<std::array<std::array<int32_t, 5>, 5>, 5> lut_5_5_5_t;
  lut_5_5_5_t find_emtf_layer_lut;

  // 5-D array of size [# types][# stations][# rings][# zones][low, high]
  typedef std::array<std::array<std::array<std::array<std::array<int32_t, 2>, 7>, 5>, 5>, 5> lut_5_5_5_7_2_t;
  lut_5_5_5_7_2_t find_emtf_zones_lut;
};

static const Utility util;

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

    // Convert all the hits again and apply the filter to get the legit hits
    int32_t sector_mode = 0;
    std::vector<Hit> sector_hits;

    for (size_t ihit = 0; ihit < conv_hits.size(); ++ihit) {
      const EMTFHit& conv_hit = conv_hits.at(ihit);

      int32_t dummy_extra_emtf_theta = 0;
      int32_t dummy_sim_tp = -1;

      if (util.is_emtf_legit_hit(conv_hit)) {
        //Hit(int16_t vh_type, int16_t vh_station, int16_t vh_ring,
        //    int16_t vh_endsec, int16_t vh_fr, int16_t vh_bx,
        //    int32_t vh_emtf_layer, int32_t vh_emtf_phi, int32_t vh_emtf_theta,
        //    int32_t vh_emtf_bend, int32_t vh_emtf_qual, int32_t vh_emtf_time,
        //    int32_t vh_old_emtf_phi, int32_t vh_old_emtf_bend, int32_t vh_extra_emtf_theta,
        //    int32_t vh_sim_tp, int32_t vh_ref)
        sector_hits.emplace_back(conv_hit.Subsystem(), conv_hit.Station(), conv_hit.Ring(),
            util.find_endsec(conv_hit), util.find_fr(conv_hit), conv_hit.BX(),
            util.find_emtf_layer(conv_hit), util.find_emtf_phi(conv_hit), util.find_emtf_theta(conv_hit),
            util.find_emtf_bend(conv_hit), util.find_emtf_qual(conv_hit), util.find_emtf_time(conv_hit),
            util.find_emtf_old_phi(conv_hit), util.find_emtf_old_bend(conv_hit), dummy_extra_emtf_theta,
            dummy_sim_tp, ihit);

        // Set sector_mode
        const Hit& hit = sector_hits.back();
        assert(hit.emtf_layer != -99);

        if (hit.type == TriggerPrimitive::kCSC) {
          sector_mode |= (1 << (4 - hit.station));
        } else if (hit.type == TriggerPrimitive::kME0) {
          sector_mode |= (1 << (4 - 1));
        } else if (hit.type == TriggerPrimitive::kDT) {
          sector_mode |= (1 << (4 - 1));
        }
      }
    }  // end loop over conv_hits

    // Provide early exit if no hit in stations 1&2 (check CSC, ME0, DT)
    if (!util.is_emtf_singlehit(sector_mode) && !util.is_emtf_singlehit_me2(sector_mode)) {
      return;
    }

    // Apply patterns to the sector hits
    apply_patterns(sector_hits, roads);
    return;
  }

  void apply_patterns(const std::vector<Hit>& sector_hits, std::vector<Road>& roads) const {
    //TODO
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
