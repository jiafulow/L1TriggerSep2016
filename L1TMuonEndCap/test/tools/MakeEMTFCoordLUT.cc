#include <memory>
#include <vector>
#include <fstream>
#include <iostream>

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/ESHandle.h"

#include "DataFormats/MuonDetId/interface/CSCDetId.h"
#include "DataFormats/MuonDetId/interface/CSCTriggerNumbering.h"
#include "L1Trigger/CSCCommonTrigger/interface/CSCConstants.h"

#include "Geometry/CSCGeometry/interface/CSCGeometry.h"
#include "Geometry/CSCGeometry/interface/CSCChamber.h"
#include "Geometry/CSCGeometry/interface/CSCLayer.h"
#include "Geometry/CSCGeometry/interface/CSCLayerGeometry.h"
#include "Geometry/Records/interface/MuonGeometryRecord.h"

//#include "L1Trigger/CSCCommonTrigger/interface/CSCTriggerGeometry.h"
//#include "L1Trigger/CSCCommonTrigger/interface/CSCConstants.h"
//#include "L1Trigger/CSCCommonTrigger/interface/CSCPatternLUT.h"
//#include "L1Trigger/CSCTrackFinder/interface/CSCSectorReceiverLUT.h"


class MakeEMTFCoordLUT : public edm::EDAnalyzer {
public:
  explicit MakeEMTFCoordLUT(const edm::ParameterSet&);
  virtual ~MakeEMTFCoordLUT();

private:
  //virtual void beginJob();
  //virtual void endJob();

  virtual void beginRun(const edm::Run&, const edm::EventSetup&);
  virtual void endRun(const edm::Run&, const edm::EventSetup&);

  virtual void analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup);

  // Generate LUTs
  void generateLUTs();
  void generateLUTs_init();
  void generateLUTs_th_corr();
  void generateLUTs_th_lut();  // and also ph_init, th_init, etc

  // Construct CSCDetId
  CSCDetId getCSCDetId(int endcap, int sector, int subsector, int station, int cscid, bool isME1A) const;

  // Get global phi in degrees
  double getGlobalPhi(int endcap, int sector, int subsector, int station, int cscid, bool isME1A, int wiregroup, int strip) const;

  // Get global theta in degrees
  double getGlobalTheta(int endcap, int sector, int subsector, int station, int cscid, bool isME1A, int wiregroup, int strip) const;

  // Get sector phi in degrees (NOTE: take half-strip)
  double getSectorPhi(int endcap, int sector, int subsector, int station, int cscid, bool isME1A, bool isNeighbor, int wiregroup, int halfstrip) const;

private:
  const edm::ParameterSet config_;

  int verbose_;

  std::string outdir_;

  int num_of_files_;

  bool done_;

  /// Event setup
  const CSCGeometry * theCSCGeometry_;

  /// Constants
  // [endcap_2+1][sector_6+1][station_4+1][chamber_12+1]
  // chamber includes CSCIDs 10,11,12 for ME1/1A
  int num_of_wires      [3][7][5][13];
  int num_of_wiregroups [3][7][5][13];
  int num_of_strips     [3][7][5][13];
  double strip_phi_pitch[3][7][5][13];
  double strip_dphi     [3][7][5][13];

  // [sector_12][station_5][chamber_16]
  int ph_init     [12][5][16];
  int ph_init_full[12][5][16];
  int ph_cover    [12][5][16];
  int ph_reverse  [12][5][16];
  int th_init     [12][5][16];
  int th_cover    [12][5][16];

  // [station_5][ring_3]
  int ph_coverage_max[5][3];
  int th_coverage_max[5][3];
};


// _____________________________________________________________________________
#define MIN_ENDCAP 1
#define MAX_ENDCAP 2
#define MIN_TRIGSECTOR 1
#define MAX_TRIGSECTOR 6

#define LOWER_THETA 8.5
#define UPPER_THETA 45.0

template <typename T>
T deltaPhiInDegrees(T phi1, T phi2) {
  T result = phi1 - phi2;  // same convention as reco::deltaPhi()
  constexpr T _twopi = 360.;
  result /= _twopi;
  result -= std::round(result);
  result *= _twopi;  // result in [-pi, pi]
  return result;
}

MakeEMTFCoordLUT::MakeEMTFCoordLUT(const edm::ParameterSet& iConfig) :
    config_(iConfig),
    verbose_(iConfig.getUntrackedParameter<int>("verbosity")),
    outdir_(iConfig.getParameter<std::string>("outdir")),
    num_of_files_(0),
    done_(false)
{
  memset(num_of_wires     , 0, sizeof(num_of_wires)     );
  memset(num_of_wiregroups, 0, sizeof(num_of_wiregroups));
  memset(num_of_strips    , 0, sizeof(num_of_strips)    );
  memset(strip_phi_pitch  , 0, sizeof(strip_phi_pitch)  );
  memset(strip_dphi       , 0, sizeof(strip_dphi)       );

  memset(ph_init     , 0, sizeof(ph_init)     );
  memset(ph_init_full, 0, sizeof(ph_init_full));
  memset(ph_cover    , 0, sizeof(ph_cover)    );
  memset(ph_reverse  , 0, sizeof(ph_reverse)  );
  memset(th_init     , 0, sizeof(th_init)     );
  memset(th_cover    , 0, sizeof(th_cover)    );

  memset(ph_coverage_max, 0, sizeof(ph_coverage_max));
  memset(th_coverage_max, 0, sizeof(th_coverage_max));

  assert(CSCConstants::KEY_CLCT_LAYER == CSCConstants::KEY_ALCT_LAYER);
}

MakeEMTFCoordLUT::~MakeEMTFCoordLUT() {}

void MakeEMTFCoordLUT::beginRun(const edm::Run& iRun, const edm::EventSetup& iSetup) {
  /// Geometry setup
  edm::ESHandle<CSCGeometry> cscGeometryHandle;
  iSetup.get<MuonGeometryRecord>().get(cscGeometryHandle);
  if (!cscGeometryHandle.isValid()) {
      std::cout << "ERROR: Unable to get MuonGeometryRecord!" << std::endl;
  } else {
      theCSCGeometry_ = cscGeometryHandle.product();
  }
}

void MakeEMTFCoordLUT::endRun(const edm::Run& iRun, const edm::EventSetup& iSetup) {

}

void MakeEMTFCoordLUT::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  if (done_)  return;

  generateLUTs();

  done_ = true;
  return;
}

// _____________________________________________________________________________
void MakeEMTFCoordLUT::generateLUTs() {
  generateLUTs_init();
  generateLUTs_th_corr();
  generateLUTs_th_lut();  // and also ph_init, th_init, etc
  std::cout << "[INFO] Generated " << num_of_files_ << " LUT files." << std::endl;
}

void MakeEMTFCoordLUT::generateLUTs_init() {

  for (int endcap = MIN_ENDCAP; endcap <= MAX_ENDCAP; ++endcap) {
    for (int sector = MIN_TRIGSECTOR; sector <= MAX_TRIGSECTOR; ++sector) {
      for (int station = 1; station <= 4; ++station) {
        for (int subsector = 0; subsector <= 2; ++subsector) {
          for (int chamber = 1; chamber <= 12; ++chamber) {
            // ME1 has subsectors 1&2, ME2,3,4 has no subsector (=0)
            if ((station == 1 && subsector == 0) || (station != 1 && subsector != 0))  continue;
            // Only ME1 has CSCIDs 10,11,12 (ME1/1a)
            if (station != 1 && chamber > 9)  continue;

            const bool isME1A = (station == 1 && chamber > 9);
            const int cscid = isME1A ? (chamber-9) : chamber;

            const CSCDetId cscDetId = getCSCDetId(endcap, sector, subsector, station, cscid, isME1A);
            const CSCChamber* chamb = theCSCGeometry_->chamber(cscDetId);
            const CSCLayerGeometry* layerGeom = chamb->layer(CSCConstants::KEY_CLCT_LAYER)->geometry();

            double phi1 = getGlobalPhi(endcap, sector, subsector, station, cscid, isME1A, 0, 0);  // wiregroup 0 , strip 0
            double phi2 = getGlobalPhi(endcap, sector, subsector, station, cscid, isME1A, 0, 1);  // wiregroup 0 , strip 1
            double dphi = std::abs(deltaPhiInDegrees(phi2, phi1));

            num_of_wires     [endcap][sector][station][chamber] = layerGeom->numberOfWires();
            num_of_wiregroups[endcap][sector][station][chamber] = layerGeom->numberOfWireGroups();
            num_of_strips    [endcap][sector][station][chamber] = layerGeom->numberOfStrips();
            strip_phi_pitch  [endcap][sector][station][chamber] = layerGeom->stripPhiPitch();
            strip_dphi       [endcap][sector][station][chamber] = dphi;

            if (verbose_ > 0) {
              std::cout << "::generateLUTs_init()"
                  << " -- endcap " << endcap << " sec " << sector << " sub " << subsector << " st " << station << " ch " << chamber
                  << " -- num_of_wires: "   << num_of_wires     [endcap][sector][station][chamber]
                  << " num_of_wiregroups: " << num_of_wiregroups[endcap][sector][station][chamber]
                  << " num_of_strips: "     << num_of_strips    [endcap][sector][station][chamber]
                  << " strip_phi_pitch: "   << strip_phi_pitch  [endcap][sector][station][chamber]
                  << " strip_dphi: "        << strip_dphi       [endcap][sector][station][chamber]
                  << std::endl;
            }
          }  // end loop over chamber
        }  // end loop over subsector
      }  // end loop over station
    }  // end loop over sector
  }  // end loop over endcap

}

// generates theta correction LUTs for ME1/1 where the wires are tilted
void MakeEMTFCoordLUT::generateLUTs_th_corr() {
  const double k = 128./(UPPER_THETA - LOWER_THETA);

  std::stringstream filename;

  for (int endcap = MIN_ENDCAP; endcap <= MAX_ENDCAP; ++endcap) {
    for (int sector = MIN_TRIGSECTOR; sector <= MAX_TRIGSECTOR; ++sector) {
      for (int subsector = 1; subsector <= 2; ++subsector) {
        for (int chamber = 1; chamber <= 4; ++chamber) {
          // Only subsector 1 has neighbor sector chamber
          if (subsector != 1 && chamber > 3)  continue;

          const int station = 1;
          const bool isME1A = false;  // always assume ME1/1b

          // Set 'real' CSCID, sector, subsector
          int rcscid = chamber;
          int rsector = sector;
          int rsubsector = subsector;
          if (station == 1 && subsector == 1 && chamber == 4) {  // is neighbor
            rcscid = 1;
            rsector = (sector == 1) ? 6 : sector - 1;
            rsubsector = 2;
          }

          // Set 'file' CSCID
          int fcscid = chamber;
          if (station == 1 && subsector == 1 && chamber == 4) {  // is neighbor
            fcscid = 13;
          }

          std::ofstream lutfile;
          filename << outdir_ << "/" << "vl_th_corr_lut_endcap_" << endcap << "_sec_" << sector << "_sub_" << subsector << "_st_" << station << "_ch_" << fcscid << ".lut";
          lutfile.open(filename.str().c_str());
          filename.str("");
          filename.clear();


          // select correction points at 1/6, 3/6 and 5/6 of chamber wg range
          // this makes construction of LUT address in firmware much easier
          const int maxWire = 48;  // ME1/1

          for (int wire = maxWire/6; wire < maxWire; wire += maxWire/3) {
            double fth0 = getGlobalTheta(endcap, rsector, rsubsector, station, rcscid, isME1A, wire, 0);  // strip 0

            for (int strip = 0; strip < 64; strip += 2) { // pattern search works in double-strip, so take every other strip
              double fth1 = getGlobalTheta(endcap, rsector, rsubsector, station, rcscid, isME1A, wire, strip);
              double fth_diff = fth1 - fth0;

              if (endcap == 2)  fth_diff = -fth_diff;  // for chambers in negative endcap, the wire tilt is the opposite way
              assert(fth_diff >= 0.);

              int th_diff = std::round(k*fth_diff);
              lutfile << std::hex << th_diff << std::endl;

              if (verbose_ > 0) {
                std::cout << "::generateLUTs_th_corr()"
                    << " -- endcap " << endcap << " sec " << sector << " sub " << subsector << " st " << station << " ch " << fcscid << " wire " << wire << " strip " << strip
                    << " -- fth0: " << fth0 << " fth1: " << fth1 << " fth_diff: " << fth_diff << " th_diff: " << th_diff
                    << std::endl;
              }
            }  // end loop over strip
          }  // end loop over wire

          lutfile.close();
          ++num_of_files_;
        }  // end loop over chamber
      }  // end loop over subsector
    }  // end loop over sector
  }  // end loop over endcap

}

// generate theta LUTs; then generate ph_init, th_init, ph_disp, th_disp LUTs
void MakeEMTFCoordLUT::generateLUTs_th_lut() {
  const double k = 128./(UPPER_THETA - LOWER_THETA);
  const double m = -k*LOWER_THETA;

  std::stringstream filename;

  for (int endcap = MIN_ENDCAP; endcap <= MAX_ENDCAP; ++endcap) {
    for (int sector = MIN_TRIGSECTOR; sector <= MAX_TRIGSECTOR; ++sector) {
      for (int station = 1; station <= 4; ++station) {
        for (int subsector = 0; subsector <= 2; ++subsector) {
          for (int chamber = 1; chamber <= 16; ++chamber) {
            // ME1 has subsectors 1&2, ME2,3,4 has no subsector (=0)
            if ((station == 1 && subsector == 0) || (station != 1 && subsector != 0))  continue;
            // Only ME1 subsector 1 has 16 chambers
            if (station == 1 && subsector == 2 && chamber > 12)  continue;
            // Only ME1 has 12 chambers or more
            if (station != 1 && chamber > 11)  continue;

            bool isME1A = false;
            bool isNeighbor = false;

            // Set 'real' CSCID, sector, subsector
            int rcscid = chamber;
            int rsector = sector;
            int rsubsector = subsector;

            if (station == 1) {  // station 1
              if (chamber <= 9) {
                rcscid = chamber;
              } else if (chamber <= 12) {
                rcscid = (chamber-9);
                isME1A = true;
              } else if (chamber == 13) {
                rcscid = 3;
              } else if (chamber == 14) {
                rcscid = 6;
              } else if (chamber == 15) {
                rcscid = 9;
              } else if (chamber == 16) {
                rcscid = 3;
                isME1A = true;
              }
              if (chamber > 12) {  // is neighbor
                isNeighbor = true;
                rsector = (sector == 1) ? 6 : sector - 1;
                rsubsector = 2;
              }

            } else {  // stations 2,3,4
              if (chamber <= 9) {
                rcscid = chamber;
              } else if (chamber == 10) {
                rcscid = 3;
              } else if (chamber == 11) {
                rcscid = 9;
              }
              if (chamber > 9) {  // is neighbor
                isNeighbor = true;
                rsector = (sector == 1) ? 6 : sector - 1;
              }
            }

            // Set maxWire, maxStrip
            int maxWire = 0;
            if (station == 1) {
              if (rcscid <= 3) {        // ME1/1
                maxWire = 48;
              } else if (rcscid <= 6) { // ME1/2
                maxWire = 64;
              } else {                  // ME1/3
                maxWire = 32;
              }
            } else if (station == 2) {
              if (rcscid <= 3) {        // ME2/1
                maxWire = 112;
              } else {                  // ME2/2
                maxWire = 64;
              }
            } else {  // stations 3,4
              if (rcscid <= 3) {        // ME3/1, ME4/1
                maxWire = 96;
              } else {                  // ME3/2, ME4/2
                maxWire = 64;
              }
            }

            int maxStrip = 0;
            if (station == 1) {
              if (isME1A) {                           // ME1/1a
                maxStrip = 48;
              } else if (rcscid <= 3) {               // ME1/1b
                maxStrip = 64;
              } else if (6 < rcscid && rcscid <= 9) { // ME1/3
                maxStrip = 64;
              } else {
                maxStrip = 80;
              }
            } else {
              maxStrip = 80;
            }

            int top_str = maxStrip/4, bot_str = maxStrip/4;
            if (station == 1 && rcscid <= 3) {  // ME1/1
              // select top and bottom strip according to endcap
              // basically, need to hit the corners of the chamber with truncated wires (relevant for ME1/1 only)
              if (endcap == 1) {
                top_str = 47;
                bot_str = 0;
              } else {
                top_str = 0;
                bot_str = 47;
              }
            }

            // find phi at first and last strips
            double fphi_first = getSectorPhi(endcap, rsector, rsubsector, station, rcscid, isME1A, isNeighbor, 0, 0);  // wire 0 halfstrip 0
            double fphi_last  = getSectorPhi(endcap, rsector, rsubsector, station, rcscid, isME1A, isNeighbor, 0, 2*maxStrip-1);  // wire 0
            double fphi_diff  = std::abs(deltaPhiInDegrees(fphi_last, fphi_first))/2.;  // in double-strip

            // find theta at top and bottom of chamber
            double fth_init  = getGlobalTheta(endcap, rsector, rsubsector, station, rcscid, isME1A, 0, bot_str);  // wire 0
            double fth_cover = getGlobalTheta(endcap, rsector, rsubsector, station, rcscid, isME1A, maxWire-1, top_str);

            std::ofstream lutfile;
            if (subsector != 0) {
              filename << outdir_ << "/" << "vl_th_lut_endcap_" << endcap << "_sec_" <<  sector << "_sub_" << subsector << "_st_" << station << "_ch_" << chamber << ".lut";
            } else {
              filename << outdir_ << "/" << "vl_th_lut_endcap_" << endcap << "_sec_" <<  sector << "_st_" << station << "_ch_" << chamber << ".lut";
            }
            lutfile.open(filename.str().c_str());
            filename.str("");
            filename.clear();


            for (int wire = 0; wire < maxWire; ++wire) {
              // take 1/4 of max strip to minimize displacement due to straight wires in polar coordinates (all chambers except ME1/1)
              int strip = maxStrip/4;
              if (station == 1 && rcscid <= 3) {  // ME1/1
                strip = 0;
              }

              double fth_wire = getGlobalTheta(endcap, rsector, rsubsector, station, rcscid, isME1A, wire, strip);
              double fth_diff = fth_wire - fth_init;
              int th_diff = std::round(k*fth_diff);

              // th_diff can become negative for truncated tilted wires in ME1/1
              // convert negative values into a large number so firmware will cut it off (relevant for ME1/1 only)
              if (station == 1 && rcscid <= 3) {  // ME1/1
                if (th_diff < 0)  th_diff &= 0x3f;  // 63 (6-bit)
              }

              lutfile << std::hex << th_diff << std::endl;

              if (verbose_ > 0) {
                std::cout << "::generateLUTs_th_lut()"
                    << " -- endcap " << endcap << " sec " << sector << " sub " << subsector << " st " << station << " ch " << chamber << " wire " << wire << " strip " << strip
                    << " -- fth_init: " << fth_init << " fth_wire: " << fth_wire << " fth_diff: " << fth_diff << " th_diff: " << th_diff
                    << std::endl;
              }
            }  // end loop over wire

            lutfile.close();
            ++num_of_files_;


            const int es = (endcap-1) * 6 + (sector-1);
            const int st = (station == 1) ? (subsector-1) : station;
            const int ch = (chamber-1);
            assert(es < 12 && st < 5 && ch < 16);

            // using ME2/2 pitch as the nominal pitch
            int my_ph_init      = std::round(fphi_first/(strip_phi_pitch[endcap][1][2][4] * 180./M_PI));
            int my_ph_init_full = std::round(fphi_first/(10./75./8.));  // 10 degree, 75 strips (?), 1/8 strip pitch
            int my_ph_cover     = std::round(fphi_diff/(strip_phi_pitch[endcap][1][2][4] * 180./M_PI));
            int my_ph_reverse   = (fphi_first > fphi_last);
            int my_th_init      = std::round(k*fth_init + m);
            int my_th_cover     = std::round(k*(fth_cover-fth_init));

            // widen ME1/1 coverage slightly, because of odd geometry of truncated wiregroups
            if (station == 1 && rcscid <= 3) {  // ME1/1
              my_th_cover += 2.;
            }

            ph_init     [es][st][ch] = my_ph_init;
            ph_init_full[es][st][ch] = my_ph_init_full;
            ph_cover    [es][st][ch] = my_ph_cover;
            ph_reverse  [es][st][ch] = my_ph_reverse;
            th_init     [es][st][ch] = my_th_init;
            th_cover    [es][st][ch] = my_th_cover;

            if (verbose_ > 0) {
              std::cout << "::generateLUTs_th_lut()"
                  << " -- endcap " << endcap << " sec " << sector << " sub " << subsector << " st " << station << " ch " << chamber << " maxWire " << maxWire << " maxStrip " << maxStrip
                  << " -- fphi_first: " << fphi_first << " fphi_last: " << fphi_last << " fth_init: " << fth_init << " fth_cover: " << fth_cover
                  << " -- ph_init: " << my_ph_init << " ph_init_full: " << my_ph_init_full << " ph_cover: " << my_ph_cover << " ph_reverse: " << my_ph_reverse << " th_init: " << my_th_init << " th_cover: " << my_th_cover
                  << std::endl;
            }
          }  // end loop over chamber
        }  // end loop over subsector
      }  // end loop over station
    }  // end loop over sector
  }  // end loop over endcap

  // ___________________________________________________________________________
  // update max coverages
  for (int es = 0; es < 12; ++es) {
    for (int st = 0; st < 5; ++st) {
      for (int ch = 0; ch < 16; ++ch) {
        if (ch > 9-1)  continue;  // not including neighbors

        int ch_type = ch/3;
        if (st > 1 && ch_type > 1) {
          ch_type = 1; // stations 2,3,4 have only 2 chamber types (a.k.a rings)
        }

        if (ph_coverage_max[st][ch_type] < ph_cover[es][st][ch])
          ph_coverage_max[st][ch_type] = ph_cover[es][st][ch];
        if (th_coverage_max[st][ch_type] < th_cover[es][st][ch])
          th_coverage_max[st][ch_type] = th_cover[es][st][ch];
      }  // end loop over ch
    }  // end loop over st
  }  // end loop over es

  // values for ph and th init values hardcoded in verilog zones.v
  // these are with offset relative to actual init values to allow for chamber displacement
  // [station_5][chamber_16]
  // ME1 chambers 13,14,15,16 are neighbor sector chambers 3,6,9,12
  // ME2 chambers 10,11 are neighbor sector chambers 3,9
  const int ph_init_hard[5][16] = {
    {39,  57,  76, 39,  58,  76, 41,  60,  79, 39,  57,  76, 21, 21, 23, 21},
    {95, 114, 132, 95, 114, 133, 98, 116, 135, 95, 114, 132,  0,  0,  0,  0},
    {38,  76, 113, 39,  58,  76, 95, 114, 132,  1,  21,   0,  0,  0,  0,  0},
    {38,  76, 113, 39,  58,  76, 95, 114, 132,  1,  21,   0,  0,  0,  0,  0},
    {38,  76, 113, 38,  57,  76, 95, 113, 132,  1,  20,   0,  0,  0,  0,  0}
  };

  const int th_init_hard[5][16] = {
    {1,1,1,42,42,42,94,94,94,1,1, 1,1,42,94, 1},
    {1,1,1,42,42,42,94,94,94,1,1, 1,0, 0, 0, 0},
    {1,1,1,48,48,48,48,48,48,1,48,0,0, 0, 0, 0},
    {1,1,1,40,40,40,40,40,40,1,40,0,0, 0, 0, 0},
    {2,2,2,34,34,34,34,34,34,2,34,0,0, 0, 0, 0}
  };

  // hardcoded chamber ph coverage in verilog prim_conv.v
  const int ph_cover_hard[5][16] = {
    {40,40,40,40,40,40,30,30,30,40,40,40,40,40,30,40},
    {40,40,40,40,40,40,30,30,30,40,40,40, 0, 0, 0, 0},
    {80,80,80,40,40,40,40,40,40,80,40, 0, 0, 0, 0, 0},
    {80,80,80,40,40,40,40,40,40,80,40, 0, 0, 0, 0, 0},
    {80,80,80,40,40,40,40,40,40,80,40, 0, 0, 0, 0, 0}
  };

  // ___________________________________________________________________________
  // generate ph_init, th_init, ph_disp, th_disp LUTs
  for (int es = 0; es < 12; ++es) {
    const int endcap = (es/6) + 1;
    const int sector = (es%6) + 1;

    std::ofstream ph_init_fs;
    filename << outdir_ << "/" << "ph_init_endcap_" << endcap << "_sect_" << sector << ".lut";
    ph_init_fs.open(filename.str().c_str());
    filename.str("");
    filename.clear();

    std::ofstream th_init_fs;
    filename << outdir_ << "/" << "th_init_endcap_" << endcap << "_sect_" << sector << ".lut";
    th_init_fs.open(filename.str().c_str());
    filename.str("");
    filename.clear();

    std::ofstream ph_disp_fs;
    filename << outdir_ << "/" << "ph_disp_endcap_" << endcap << "_sect_" << sector << ".lut";
    ph_disp_fs.open(filename.str().c_str());
    filename.str("");
    filename.clear();

    std::ofstream th_disp_fs;
    filename << outdir_ << "/" << "th_disp_endcap_" << endcap << "_sect_" << sector << ".lut";
    th_disp_fs.open(filename.str().c_str());
    filename.str("");
    filename.clear();

    for (int st = 0; st < 5; ++st) {
      std::ofstream ph_init_full_fs;
      filename << outdir_ << "/" << "ph_init_full_endcap_" << endcap << "_sect_" << sector << "_st_" << st << ".lut";
      ph_init_full_fs.open(filename.str().c_str());
      filename.str("");
      filename.clear();

      const int max_ch = (st == 0) ? 16 : (st == 1) ? 12 : 11;

      for (int ch = 0; ch < max_ch; ++ch) {
        int my_ph_init       = ph_init[es][st][ch];
        int my_ph_init_full  = ph_init_full[es][st][ch];
        int my_th_init       = th_init[es][st][ch];
        int my_ph_disp       = (ph_init[es][st][ch]/2 - 2*ph_init_hard[st][ch]);  // in double-strip
        if (ph_reverse[es][st][ch])
          my_ph_disp        -= ph_cover_hard[st][ch];
        int my_th_disp       = (th_init[es][st][ch] - th_init_hard[st][ch]);

        ph_init_fs << std::hex << my_ph_init << std::endl;
        ph_init_full_fs << std::hex << my_ph_init_full << std::endl;
        th_init_fs << std::hex << my_th_init << std::endl;
        ph_disp_fs << std::hex << my_ph_disp << std::endl;
        th_disp_fs << std::hex << my_th_disp << std::endl;

        if (verbose_ > 0) {
          std::cout << "::generateLUTs_th_lut()"
              << " -- endcap " << endcap << " sec " << sector << " sub " << 0 << " st " << st << " ch " << ch
              << " -- ph_init: " << my_ph_init << " ph_init_full: " << my_ph_init_full << " th_init: " << my_th_init << " ph_disp: " << my_ph_disp << " th_disp: " << my_th_disp
              << std::endl;
        }
      }  // end loop over ch

      ph_init_full_fs.close();
      ++num_of_files_;
    }  // end loop over st

    ph_init_fs.close();
    ++num_of_files_;
    th_init_fs.close();
    ++num_of_files_;
    ph_disp_fs.close();
    ++num_of_files_;
    th_disp_fs.close();
    ++num_of_files_;
  }  // end loop over es

}

CSCDetId MakeEMTFCoordLUT::getCSCDetId(int endcap, int sector, int subsector, int station, int cscid, bool isME1A) const {
  int ring = isME1A ? 4 : CSCTriggerNumbering::ringFromTriggerLabels(station, cscid);
  int chamber = CSCTriggerNumbering::chamberFromTriggerLabels(sector, subsector, station, cscid);
  const CSCDetId cscDetId = CSCDetId(endcap, station, ring, chamber, CSCConstants::KEY_CLCT_LAYER);
  return cscDetId;
}

double MakeEMTFCoordLUT::getGlobalPhi(int endcap, int sector, int subsector, int station, int cscid, bool isME1A, int wiregroup, int strip) const {
  const CSCDetId cscDetId = getCSCDetId(endcap, sector, subsector, station, cscid, isME1A);
  const CSCChamber* chamb = theCSCGeometry_->chamber(cscDetId);
  const CSCLayerGeometry* layerGeom = chamb->layer(CSCConstants::KEY_CLCT_LAYER)->geometry();

  const LocalPoint lp = layerGeom->stripWireGroupIntersection(strip+1, wiregroup+1); // strip and wg in geometry routines start from 1
  const GlobalPoint gp = chamb->layer(CSCConstants::KEY_ALCT_LAYER)->surface().toGlobal(lp);

  double phi = double(gp.phi()) * 180./M_PI;
  //if (phi < 0.)
  //  phi += 360.;  // to get a phi value in the range of 0 - 360 degrees
  return phi;
}

double MakeEMTFCoordLUT::getGlobalTheta(int endcap, int sector, int subsector, int station, int cscid, bool isME1A, int wiregroup, int strip) const {
  const CSCDetId cscDetId = getCSCDetId(endcap, sector, subsector, station, cscid, isME1A);
  const CSCChamber* chamb = theCSCGeometry_->chamber(cscDetId);
  const CSCLayerGeometry* layerGeom = chamb->layer(CSCConstants::KEY_CLCT_LAYER)->geometry();

  //const LocalPoint lp = layerGeom->stripWireGroupIntersection(strip+1, wiregroup+1); // strip and wg in geometry routines start from 1
  const LocalPoint lp = layerGeom->intersectionOfStripAndWire(strip+1, layerGeom->middleWireOfGroup(wiregroup+1));
  const GlobalPoint gp = chamb->layer(CSCConstants::KEY_ALCT_LAYER)->surface().toGlobal(lp);

  double theta = double(gp.theta()) * 180./M_PI;
  if (endcap == 2)
    theta = 180. - theta;  // to get a theta value in the range of 0 - 180 degrees for endcap 2
  return theta;
}

double MakeEMTFCoordLUT::getSectorPhi(int endcap, int sector, int subsector, int station, int cscid, bool isME1A, bool isNeighbor, int wiregroup, int halfstrip) const {
  int strip = halfstrip / 2;
  int oddhs = halfstrip % 2;

  const CSCDetId cscDetId = getCSCDetId(endcap, sector, subsector, station, cscid, isME1A);
  const CSCChamber* chamb = theCSCGeometry_->chamber(cscDetId);
  const CSCLayerGeometry* layerGeom = chamb->layer(CSCConstants::KEY_CLCT_LAYER)->geometry();

  double globalPhi = getGlobalPhi(endcap, sector, subsector, station, cscid, isME1A, wiregroup, strip);

  // Is this chamber mounted in reverse direction?
  bool ph_reverse = false;
  if ((endcap == 1 && station >= 3) || (endcap == 2 && station < 3))
    ph_reverse = true;

  double pitch = (layerGeom->stripPhiPitch() * 180./M_PI);
  if (ph_reverse)
    pitch = -pitch;

  if (oddhs == 1)
    globalPhi += pitch/4; // take half-strip into account
  else
    globalPhi -= pitch/4;

  int sector_n = (sector == 1) ? 6 : sector - 1;  // neighbor sector
  if (isNeighbor)
    sector_n = sector;  // same sector

  // sector boundary should not depend on station, cscid, etc. For now, take station 2 csc 1 strip 0 as boundary, -2 deg (Darin, 2009-09-18)
  // correction for sector overlap: take sector boundary at previous sector, station 2 csc 3 strip 0 - 2 deg (Matt, 2016-03-07)
  const int firstWire = 0;
  const int firstStrip = (endcap == 1) ? 0 : 79;
  double sectorStartPhi = getGlobalPhi(endcap, sector_n, 0, 2, 3, false, firstWire, firstStrip) - 2.;

  double res = deltaPhiInDegrees(globalPhi, sectorStartPhi);
  return res;
}

// DEFINE THIS AS A PLUG-IN
#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(MakeEMTFCoordLUT);
