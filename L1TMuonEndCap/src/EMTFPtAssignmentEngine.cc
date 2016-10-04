#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPtAssignmentEngine.hh"

#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPtAssignmentEngineHelper.hh"

#include <cassert>
#include <iostream>


EMTFPtAssignmentEngine::EMTFPtAssignmentEngine() :
    allowedModes_({3,5,9,6,10,12,7,11,13,14,15}),
    forests_(),
    ok_(false)
{

}

EMTFPtAssignmentEngine::~EMTFPtAssignmentEngine() {

}

void EMTFPtAssignmentEngine::read(const std::string& xml_dir) {
  if (ok_)  return;

  //std::string xml_dir_full = "L1Trigger/L1TMuon/data/emtf_luts/" + xml_dir + "/ModeVariables/trees";
  std::string xml_dir_full = "L1TriggerSep2016/L1TMuonEndCap/data/emtf_luts/" + xml_dir + "/ModeVariables/trees";

  for (unsigned i = 0; i < allowedModes_.size(); ++i) {
    int mode_inv = allowedModes_.at(i);  // inverted mode because reasons
    std::stringstream ss;
    ss << xml_dir_full << "/" << mode_inv;
    forests_.at(mode_inv).loadForestFromXML(ss.str().c_str(), 64);
  }

  ok_ = true;
  return;
}

void EMTFPtAssignmentEngine::configure(
    int verbose,
    bool readPtLUTFile, bool fixMode15HighPt, bool fix9bDPhi
) {
  verbose_ = verbose;

  readPtLUTFile_   = readPtLUTFile;
  fixMode15HighPt_ = fixMode15HighPt;
  fix9bDPhi_       = fix9bDPhi;
}

EMTFPtAssignmentEngine::address_t EMTFPtAssignmentEngine::calculate_address(const EMTFTrackExtra& track) const {
  address_t address = 0;

  int mode_inv  = track.mode_inv;
  int theta     = track.theta_int;
  theta >>= 2;

  bool use_eta = false;
  if (use_eta) {
    float ftheta = track.theta_int;
    ftheta = (ftheta*0.2874016 + 8.5)*(3.14159265359/180);
    float eta = (-1)*std::log(std::tan(ftheta/2));
    theta = getEtaInt(eta, 5);
  }

  const EMTFPtLUTData& ptlut_data = track.ptlut_data;

  int dPhi12    = ptlut_data.delta_ph[0];
  int dPhi13    = ptlut_data.delta_ph[1];
  int dPhi14    = ptlut_data.delta_ph[2];
  int dPhi23    = ptlut_data.delta_ph[3];
  int dPhi24    = ptlut_data.delta_ph[4];
  int dPhi34    = ptlut_data.delta_ph[5];
  int dTheta12  = ptlut_data.delta_th[0];
  int dTheta13  = ptlut_data.delta_th[1];
  int dTheta14  = ptlut_data.delta_th[2];
  int dTheta23  = ptlut_data.delta_th[3];
  int dTheta24  = ptlut_data.delta_th[4];
  int dTheta34  = ptlut_data.delta_th[5];
  int FR1       = ptlut_data.fr      [0];
  int FR2       = ptlut_data.fr      [1];
  int FR3       = ptlut_data.fr      [2];
  int FR4       = ptlut_data.fr      [3];

  int sign12       = ptlut_data.sign_ph [0];
  int sign13       = ptlut_data.sign_ph [1];
  int sign14       = ptlut_data.sign_ph [2];
  int sign23       = ptlut_data.sign_ph [3];
  int sign24       = ptlut_data.sign_ph [4];
  int sign34       = ptlut_data.sign_ph [5];
  int dTheta12Sign = ptlut_data.sign_th [0];
  int dTheta13Sign = ptlut_data.sign_th [1];
  int dTheta14Sign = ptlut_data.sign_th [2];
  int dTheta23Sign = ptlut_data.sign_th [3];
  int dTheta24Sign = ptlut_data.sign_th [4];
  int dTheta34Sign = ptlut_data.sign_th [5];

  int CLCT1     = std::abs(getCLCT(ptlut_data.cpattern[0]));
  int CLCT2     = std::abs(getCLCT(ptlut_data.cpattern[1]));
  int CLCT3     = std::abs(getCLCT(ptlut_data.cpattern[2]));
  int CLCT4     = std::abs(getCLCT(ptlut_data.cpattern[3]));
  int CLCT1Sign = (getCLCT(ptlut_data.cpattern[0]) > 0);
  int CLCT2Sign = (getCLCT(ptlut_data.cpattern[1]) > 0);
  int CLCT3Sign = (getCLCT(ptlut_data.cpattern[2]) > 0);
  int CLCT4Sign = (getCLCT(ptlut_data.cpattern[3]) > 0);

  int CSCID1    = ptlut_data.bt_chamber[0];
  int CSCID2    = ptlut_data.bt_chamber[1];
  int CSCID3    = ptlut_data.bt_chamber[2];
  int CSCID4    = ptlut_data.bt_chamber[3];

  dTheta12 = getdTheta(dTheta12 * ((dTheta12Sign) ? -1 : 1));
  dTheta13 = getdTheta(dTheta13 * ((dTheta13Sign) ? -1 : 1));
  dTheta14 = getdTheta(dTheta14 * ((dTheta14Sign) ? -1 : 1));
  dTheta23 = getdTheta(dTheta23 * ((dTheta23Sign) ? -1 : 1));
  dTheta24 = getdTheta(dTheta24 * ((dTheta24Sign) ? -1 : 1));
  dTheta34 = getdTheta(dTheta34 * ((dTheta34Sign) ? -1 : 1));

  bool use_FRLUT = true;
  if (use_FRLUT) {
    int sector = track.sector;
    FR1 = getFRLUT(sector, CSCID1/12, CSCID1%12);
    FR2 = getFRLUT(sector, CSCID2/12, CSCID2%12);
    FR3 = getFRLUT(sector, CSCID3/12, CSCID3%12);
    FR4 = getFRLUT(sector, CSCID4/12, CSCID4%12);
  }

  switch(mode_inv) {
  case 3:   // 1-2
    if (fix9bDPhi_)  dPhi12 = std::min(511, dPhi12);

    address |= (dPhi12      & ((1<<9)-1)) << (0);
    address |= (sign12      & ((1<<1)-1)) << (0+9);
    address |= (dTheta12    & ((1<<3)-1)) << (0+9+1);
    address |= (CLCT1       & ((1<<2)-1)) << (0+9+1+3);
    address |= (CLCT1Sign   & ((1<<1)-1)) << (0+9+1+3+2);
    address |= (CLCT2       & ((1<<2)-1)) << (0+9+1+3+2+1);
    address |= (CLCT2Sign   & ((1<<1)-1)) << (0+9+1+3+2+1+2);
    address |= (FR1         & ((1<<1)-1)) << (0+9+1+3+2+1+2+1);
    address |= (FR2         & ((1<<1)-1)) << (0+9+1+3+2+1+2+1+1);
    address |= (theta       & ((1<<5)-1)) << (0+9+1+3+2+1+2+1+1+1);
    address |= (mode_inv    & ((1<<4)-1)) << (0+9+1+3+2+1+2+1+1+1+5);
    break;

  case 5:   // 1-3
    if (fix9bDPhi_)  dPhi13 = std::min(511, dPhi13);

    address |= (dPhi13      & ((1<<9)-1)) << (0);
    address |= (sign13      & ((1<<1)-1)) << (0+9);
    address |= (dTheta13    & ((1<<3)-1)) << (0+9+1);
    address |= (CLCT1       & ((1<<2)-1)) << (0+9+1+3);
    address |= (CLCT1Sign   & ((1<<1)-1)) << (0+9+1+3+2);
    address |= (CLCT3       & ((1<<2)-1)) << (0+9+1+3+2+1);
    address |= (CLCT3Sign   & ((1<<1)-1)) << (0+9+1+3+2+1+2);
    address |= (FR1         & ((1<<1)-1)) << (0+9+1+3+2+1+2+1);
    address |= (FR3         & ((1<<1)-1)) << (0+9+1+3+2+1+2+1+1);
    address |= (theta       & ((1<<5)-1)) << (0+9+1+3+2+1+2+1+1+1);
    address |= (mode_inv    & ((1<<4)-1)) << (0+9+1+3+2+1+2+1+1+1+5);
    break;

  case 9:   // 1-4
    if (fix9bDPhi_)  dPhi14 = std::min(511, dPhi14);

    address |= (dPhi14      & ((1<<9)-1)) << (0);
    address |= (sign14      & ((1<<1)-1)) << (0+9);
    address |= (dTheta14    & ((1<<3)-1)) << (0+9+1);
    address |= (CLCT1       & ((1<<2)-1)) << (0+9+1+3);
    address |= (CLCT1Sign   & ((1<<1)-1)) << (0+9+1+3+2);
    address |= (CLCT4       & ((1<<2)-1)) << (0+9+1+3+2+1);
    address |= (CLCT4Sign   & ((1<<1)-1)) << (0+9+1+3+2+1+2);
    address |= (FR1         & ((1<<1)-1)) << (0+9+1+3+2+1+2+1);
    address |= (FR4         & ((1<<1)-1)) << (0+9+1+3+2+1+2+1+1);
    address |= (theta       & ((1<<5)-1)) << (0+9+1+3+2+1+2+1+1+1);
    address |= (mode_inv    & ((1<<4)-1)) << (0+9+1+3+2+1+2+1+1+1+5);
    break;

  case 6:   // 2-3
    if (fix9bDPhi_)  dPhi23 = std::min(511, dPhi23);

    address |= (dPhi23      & ((1<<9)-1)) << (0);
    address |= (sign23      & ((1<<1)-1)) << (0+9);
    address |= (dTheta23    & ((1<<3)-1)) << (0+9+1);
    address |= (CLCT2       & ((1<<2)-1)) << (0+9+1+3);
    address |= (CLCT2Sign   & ((1<<1)-1)) << (0+9+1+3+2);
    address |= (CLCT3       & ((1<<2)-1)) << (0+9+1+3+2+1);
    address |= (CLCT3Sign   & ((1<<1)-1)) << (0+9+1+3+2+1+2);
    address |= (FR2         & ((1<<1)-1)) << (0+9+1+3+2+1+2+1);
    address |= (FR3         & ((1<<1)-1)) << (0+9+1+3+2+1+2+1+1);
    address |= (theta       & ((1<<5)-1)) << (0+9+1+3+2+1+2+1+1+1);
    address |= (mode_inv    & ((1<<4)-1)) << (0+9+1+3+2+1+2+1+1+1+5);
    break;

  case 10:  // 2-4
    if (fix9bDPhi_)  dPhi24 = std::min(511, dPhi24);

    address |= (dPhi24      & ((1<<9)-1)) << (0);
    address |= (sign24      & ((1<<1)-1)) << (0+9);
    address |= (dTheta24    & ((1<<3)-1)) << (0+9+1);
    address |= (CLCT2       & ((1<<2)-1)) << (0+9+1+3);
    address |= (CLCT2Sign   & ((1<<1)-1)) << (0+9+1+3+2);
    address |= (CLCT4       & ((1<<2)-1)) << (0+9+1+3+2+1);
    address |= (CLCT4Sign   & ((1<<1)-1)) << (0+9+1+3+2+1+2);
    address |= (FR2         & ((1<<1)-1)) << (0+9+1+3+2+1+2+1);
    address |= (FR4         & ((1<<1)-1)) << (0+9+1+3+2+1+2+1+1);
    address |= (theta       & ((1<<5)-1)) << (0+9+1+3+2+1+2+1+1+1);
    address |= (mode_inv    & ((1<<4)-1)) << (0+9+1+3+2+1+2+1+1+1+5);
    break;

  case 12:  // 3-4
    if (fix9bDPhi_)  dPhi34 = std::min(511, dPhi34);

    address |= (dPhi34      & ((1<<9)-1)) << (0);
    address |= (sign34      & ((1<<1)-1)) << (0+9);
    address |= (dTheta34    & ((1<<3)-1)) << (0+9+1);
    address |= (CLCT3       & ((1<<2)-1)) << (0+9+1+3);
    address |= (CLCT3Sign   & ((1<<1)-1)) << (0+9+1+3+2);
    address |= (CLCT4       & ((1<<2)-1)) << (0+9+1+3+2+1);
    address |= (CLCT4Sign   & ((1<<1)-1)) << (0+9+1+3+2+1+2);
    address |= (FR3         & ((1<<1)-1)) << (0+9+1+3+2+1+2+1);
    address |= (FR4         & ((1<<1)-1)) << (0+9+1+3+2+1+2+1+1);
    address |= (theta       & ((1<<5)-1)) << (0+9+1+3+2+1+2+1+1+1);
    address |= (mode_inv    & ((1<<4)-1)) << (0+9+1+3+2+1+2+1+1+1+5);
    break;

  case 7:   // 1-2-3
    dPhi12 = getNLBdPhiBin(dPhi12, 7, 512);
    dPhi23 = getNLBdPhiBin(dPhi23, 5, 256);

    address |= (dPhi12      & ((1<<7)-1)) << (0);
    address |= (dPhi23      & ((1<<5)-1)) << (0+7);
    address |= (sign12      & ((1<<1)-1)) << (0+7+5);
    address |= (sign23      & ((1<<1)-1)) << (0+7+5+1);
    address |= (dTheta13    & ((1<<3)-1)) << (0+7+5+1+1);
    address |= (CLCT1       & ((1<<2)-1)) << (0+7+5+1+1+3);
    address |= (CLCT1Sign   & ((1<<1)-1)) << (0+7+5+1+1+3+2);
    address |= (FR1         & ((1<<1)-1)) << (0+7+5+1+1+3+2+1);
    address |= (theta       & ((1<<5)-1)) << (0+7+5+1+1+3+2+1+1);
    address |= (mode_inv    & ((1<<4)-1)) << (0+7+5+1+1+3+2+1+1+5);
    break;

  case 11:  // 1-2-4
    dPhi12 = getNLBdPhiBin(dPhi12, 7, 512);
    dPhi24 = getNLBdPhiBin(dPhi24, 5, 256);

    address |= (dPhi12      & ((1<<7)-1)) << (0);
    address |= (dPhi24      & ((1<<5)-1)) << (0+7);
    address |= (sign12      & ((1<<1)-1)) << (0+7+5);
    address |= (sign24      & ((1<<1)-1)) << (0+7+5+1);
    address |= (dTheta14    & ((1<<3)-1)) << (0+7+5+1+1);
    address |= (CLCT1       & ((1<<2)-1)) << (0+7+5+1+1+3);
    address |= (CLCT1Sign   & ((1<<1)-1)) << (0+7+5+1+1+3+2);
    address |= (FR1         & ((1<<1)-1)) << (0+7+5+1+1+3+2+1);
    address |= (theta       & ((1<<5)-1)) << (0+7+5+1+1+3+2+1+1);
    address |= (mode_inv    & ((1<<4)-1)) << (0+7+5+1+1+3+2+1+1+5);
    break;

  case 13:  // 1-3-4
    dPhi13 = getNLBdPhiBin(dPhi13, 7, 512);
    dPhi34 = getNLBdPhiBin(dPhi34, 5, 256);

    address |= (dPhi13      & ((1<<7)-1)) << (0);
    address |= (dPhi34      & ((1<<5)-1)) << (0+7);
    address |= (sign13      & ((1<<1)-1)) << (0+7+5);
    address |= (sign34      & ((1<<1)-1)) << (0+7+5+1);
    address |= (dTheta14    & ((1<<3)-1)) << (0+7+5+1+1);
    address |= (CLCT1       & ((1<<2)-1)) << (0+7+5+1+1+3);
    address |= (CLCT1Sign   & ((1<<1)-1)) << (0+7+5+1+1+3+2);
    address |= (FR1         & ((1<<1)-1)) << (0+7+5+1+1+3+2+1);
    address |= (theta       & ((1<<5)-1)) << (0+7+5+1+1+3+2+1+1);
    address |= (mode_inv    & ((1<<4)-1)) << (0+7+5+1+1+3+2+1+1+5);
    break;

  case 14:  // 2-3-4
    dPhi23 = getNLBdPhiBin(dPhi23, 7, 512);
    dPhi34 = getNLBdPhiBin(dPhi34, 6, 256);

    address |= (dPhi23      & ((1<<7)-1)) << (0);
    address |= (dPhi34      & ((1<<6)-1)) << (0+7);
    address |= (sign23      & ((1<<1)-1)) << (0+7+6);
    address |= (sign34      & ((1<<1)-1)) << (0+7+6+1);
    address |= (dTheta24    & ((1<<3)-1)) << (0+7+6+1+1);
    address |= (CLCT2       & ((1<<2)-1)) << (0+7+6+1+1+3);
    address |= (CLCT2Sign   & ((1<<1)-1)) << (0+7+6+1+1+3+2);
    address |= (theta       & ((1<<5)-1)) << (0+7+6+1+1+3+2+1);
    address |= (mode_inv    & ((1<<4)-1)) << (0+7+6+1+1+3+2+1+5);
    break;

  case 15:  // 1-2-3-4
    // Calculate two signs based on three input signs
    if (sign12==0 && sign23==0 && sign34==0) {
      sign12=1; sign23=1; sign34=1;
    } else if (sign12==0 && sign23==1 && sign34==1) {
      sign12=1; sign23=0; sign34=0;
    } else if (sign12==0 && sign23==0 && sign34==1) {
      sign12=1; sign23=1; sign34=0;
    } else if (sign12==0 && sign23==1 && sign34==0) {
      sign12=1; sign23=0; sign34=1;
    }

    dPhi12 = getNLBdPhiBin(dPhi12, 7, 512);
    dPhi23 = getNLBdPhiBin(dPhi23, 5, 256);
    dPhi34 = getNLBdPhiBin(dPhi34, 6, 256);

    address |= (dPhi12      & ((1<<7)-1)) << (0);
    address |= (dPhi23      & ((1<<5)-1)) << (0+7);
    address |= (dPhi34      & ((1<<6)-1)) << (0+7+5);
    address |= (sign23      & ((1<<1)-1)) << (0+7+5+6);
    address |= (sign34      & ((1<<1)-1)) << (0+7+5+6+1);
    address |= (FR1         & ((1<<1)-1)) << (0+7+5+6+1+1);
    address |= (theta       & ((1<<5)-1)) << (0+7+5+6+1+1+1);
    address |= (mode_inv    & ((1<<4)-1)) << (0+7+5+6+1+1+1+5);
    break;

  default:
    break;
  }

  return address;
}

EMTFPtAssignmentEngine::address_t EMTFPtAssignmentEngine::calculate_address_fw(const EMTFTrackExtra& track) const {
  address_t address = 0;

  // Not implemented

  return address;
}

float EMTFPtAssignmentEngine::calculate_pt(const address_t& address) {
  float pt = 0.;

  if (address == 0)  // invalid address
    return pt;

  uint16_t mode_inv = (address >> (30-4)) & ((1<<4)-1);

  auto contain = [](const auto& vec, const auto& elem) {
    return (std::find(vec.begin(), vec.end(), elem) != vec.end());
  };

  bool is_good_mode = contain(allowedModes_, mode_inv);

  if (!is_good_mode)  // invalid mode
    return pt;

  int dPhi12    = 0;
  int dPhi13    = 0;
  int dPhi14    = 0;
  int dPhi23    = 0;
  int dPhi24    = 0;
  int dPhi34    = 0;
  int dTheta12  = 0;
  int dTheta13  = 0;
  int dTheta14  = 0;
  int dTheta23  = 0;
  int dTheta24  = 0;
  int dTheta34  = 0;
  int CLCT1     = 0;
  int CLCT2     = 0;
  int CLCT3     = 0;
  int CLCT4     = 0;
  int CSCID1    = 0;
  int CSCID2    = 0;
  int CSCID3    = 0;
  int CSCID4    = 0;
  int FR1       = 0;
  int FR2       = 0;
  int FR3       = 0;
  int FR4       = 0;

  int sign12    = 1;
  int sign13    = 1;
  int sign14    = 1;
  int sign23    = 1;
  int sign24    = 1;
  int sign34    = 1;

  int CLCT1Sign = 1;
  int CLCT2Sign = 1;
  int CLCT3Sign = 1;
  int CLCT4Sign = 1;

  int theta = 0;

  switch(mode_inv) {
  case 3:   // 1-2
    dPhi12    = (address >> (0))                    & ((1<<9)-1);
    sign12    = (address >> (0+9))                  & ((1<<1)-1);
    dTheta12  = (address >> (0+9+1))                & ((1<<3)-1);
    CLCT1     = (address >> (0+9+1+3))              & ((1<<2)-1);
    CLCT1Sign = (address >> (0+9+1+3+2))            & ((1<<1)-1);
    CLCT2     = (address >> (0+9+1+3+2+1))          & ((1<<2)-1);
    CLCT2Sign = (address >> (0+9+1+3+2+1+2))        & ((1<<1)-1);
    FR1       = (address >> (0+9+1+3+2+1+2+1))      & ((1<<1)-1);
    FR2       = (address >> (0+9+1+3+2+1+2+1+1))    & ((1<<1)-1);
    theta     = (address >> (0+9+1+3+2+1+2+1+1+1))  & ((1<<5)-1);
    break;

  case 5:   // 1-3
    dPhi13    = (address >> (0))                    & ((1<<9)-1);
    sign13    = (address >> (0+9))                  & ((1<<1)-1);
    dTheta13  = (address >> (0+9+1))                & ((1<<3)-1);
    CLCT1     = (address >> (0+9+1+3))              & ((1<<2)-1);
    CLCT1Sign = (address >> (0+9+1+3+2))            & ((1<<1)-1);
    CLCT3     = (address >> (0+9+1+3+2+1))          & ((1<<2)-1);
    CLCT3Sign = (address >> (0+9+1+3+2+1+2))        & ((1<<1)-1);
    FR1       = (address >> (0+9+1+3+2+1+2+1))      & ((1<<1)-1);
    FR3       = (address >> (0+9+1+3+2+1+2+1+1))    & ((1<<1)-1);
    theta     = (address >> (0+9+1+3+2+1+2+1+1+1))  & ((1<<5)-1);
    break;

  case 9:   // 1-4
    dPhi14    = (address >> (0))                    & ((1<<9)-1);
    sign14    = (address >> (0+9))                  & ((1<<1)-1);
    dTheta14  = (address >> (0+9+1))                & ((1<<3)-1);
    CLCT1     = (address >> (0+9+1+3))              & ((1<<2)-1);
    CLCT1Sign = (address >> (0+9+1+3+2))            & ((1<<1)-1);
    CLCT4     = (address >> (0+9+1+3+2+1))          & ((1<<2)-1);
    CLCT4Sign = (address >> (0+9+1+3+2+1+2))        & ((1<<1)-1);
    FR1       = (address >> (0+9+1+3+2+1+2+1))      & ((1<<1)-1);
    FR4       = (address >> (0+9+1+3+2+1+2+1+1))    & ((1<<1)-1);
    theta     = (address >> (0+9+1+3+2+1+2+1+1+1))  & ((1<<5)-1);
    break;

  case 6:   // 2-3
    dPhi23    = (address >> (0))                    & ((1<<9)-1);
    sign23    = (address >> (0+9))                  & ((1<<1)-1);
    dTheta23  = (address >> (0+9+1))                & ((1<<3)-1);
    CLCT2     = (address >> (0+9+1+3))              & ((1<<2)-1);
    CLCT2Sign = (address >> (0+9+1+3+2))            & ((1<<1)-1);
    CLCT3     = (address >> (0+9+1+3+2+1))          & ((1<<2)-1);
    CLCT3Sign = (address >> (0+9+1+3+2+1+2))        & ((1<<1)-1);
    FR2       = (address >> (0+9+1+3+2+1+2+1))      & ((1<<1)-1);
    FR3       = (address >> (0+9+1+3+2+1+2+1+1))    & ((1<<1)-1);
    theta     = (address >> (0+9+1+3+2+1+2+1+1+1))  & ((1<<5)-1);
    break;

  case 10:  // 2-4
    dPhi24    = (address >> (0))                    & ((1<<9)-1);
    sign24    = (address >> (0+9))                  & ((1<<1)-1);
    dTheta24  = (address >> (0+9+1))                & ((1<<3)-1);
    CLCT2     = (address >> (0+9+1+3))              & ((1<<2)-1);
    CLCT2Sign = (address >> (0+9+1+3+2))            & ((1<<1)-1);
    CLCT4     = (address >> (0+9+1+3+2+1))          & ((1<<2)-1);
    CLCT4Sign = (address >> (0+9+1+3+2+1+2))        & ((1<<1)-1);
    FR2       = (address >> (0+9+1+3+2+1+2+1))      & ((1<<1)-1);
    FR4       = (address >> (0+9+1+3+2+1+2+1+1))    & ((1<<1)-1);
    theta     = (address >> (0+9+1+3+2+1+2+1+1+1))  & ((1<<5)-1);
    break;

  case 12:  // 3-4
    dPhi34    = (address >> (0))                    & ((1<<9)-1);
    sign34    = (address >> (0+9))                  & ((1<<1)-1);
    dTheta34  = (address >> (0+9+1))                & ((1<<3)-1);
    CLCT3     = (address >> (0+9+1+3))              & ((1<<2)-1);
    CLCT3Sign = (address >> (0+9+1+3+2))            & ((1<<1)-1);
    CLCT4     = (address >> (0+9+1+3+2+1))          & ((1<<2)-1);
    CLCT4Sign = (address >> (0+9+1+3+2+1+2))        & ((1<<1)-1);
    FR3       = (address >> (0+9+1+3+2+1+2+1))      & ((1<<1)-1);
    FR4       = (address >> (0+9+1+3+2+1+2+1+1))    & ((1<<1)-1);
    theta     = (address >> (0+9+1+3+2+1+2+1+1+1))  & ((1<<5)-1);
    break;

  case 7:   // 1-2-3
    dPhi12    = (address >> (0))                    & ((1<<7)-1);
    dPhi23    = (address >> (0+7))                  & ((1<<5)-1);
    sign12    = (address >> (0+7+5))                & ((1<<1)-1);
    sign23    = (address >> (0+7+5+1))              & ((1<<1)-1);
    dTheta13  = (address >> (0+7+5+1+1))            & ((1<<3)-1);
    CLCT1     = (address >> (0+7+5+1+1+3))          & ((1<<2)-1);
    CLCT1Sign = (address >> (0+7+5+1+1+3+2))        & ((1<<1)-1);
    FR1       = (address >> (0+7+5+1+1+3+2+1))      & ((1<<1)-1);
    theta     = (address >> (0+7+5+1+1+3+2+1+1))    & ((1<<5)-1);

    dPhi12 = getdPhiFromBin(dPhi12, 7, 512);
    dPhi23 = getdPhiFromBin(dPhi23, 5, 256);
    break;

  case 11:  // 1-2-4
    dPhi12    = (address >> (0))                    & ((1<<7)-1);
    dPhi24    = (address >> (0+7))                  & ((1<<5)-1);
    sign12    = (address >> (0+7+5))                & ((1<<1)-1);
    sign24    = (address >> (0+7+5+1))              & ((1<<1)-1);
    dTheta14  = (address >> (0+7+5+1+1))            & ((1<<3)-1);
    CLCT1     = (address >> (0+7+5+1+1+3))          & ((1<<2)-1);
    CLCT1Sign = (address >> (0+7+5+1+1+3+2))        & ((1<<1)-1);
    FR1       = (address >> (0+7+5+1+1+3+2+1))      & ((1<<1)-1);
    theta     = (address >> (0+7+5+1+1+3+2+1+1))    & ((1<<5)-1);

    dPhi12 = getdPhiFromBin(dPhi12, 7, 512);
    dPhi24 = getdPhiFromBin(dPhi24, 5, 256);
    break;

  case 13:  // 1-3-4
    dPhi13    = (address >> (0))                    & ((1<<7)-1);
    dPhi34    = (address >> (0+7))                  & ((1<<5)-1);
    sign13    = (address >> (0+7+5))                & ((1<<1)-1);
    sign34    = (address >> (0+7+5+1))              & ((1<<1)-1);
    dTheta14  = (address >> (0+7+5+1+1))            & ((1<<3)-1);
    CLCT1     = (address >> (0+7+5+1+1+3))          & ((1<<2)-1);
    CLCT1Sign = (address >> (0+7+5+1+1+3+2))        & ((1<<1)-1);
    FR1       = (address >> (0+7+5+1+1+3+2+1))      & ((1<<1)-1);
    theta     = (address >> (0+7+5+1+1+3+2+1+1))    & ((1<<5)-1);

    dPhi13 = getdPhiFromBin(dPhi13, 7, 512);
    dPhi34 = getdPhiFromBin(dPhi34, 5, 256);
    break;

  case 14:  // 2-3-4
    dPhi23    = (address >> (0))                    & ((1<<7)-1);
    dPhi34    = (address >> (0+7))                  & ((1<<6)-1);
    sign23    = (address >> (0+7+6))                & ((1<<1)-1);
    sign34    = (address >> (0+7+6+1))              & ((1<<1)-1);
    dTheta24  = (address >> (0+7+6+1+1))            & ((1<<3)-1);
    CLCT2     = (address >> (0+7+6+1+1+3))          & ((1<<2)-1);
    CLCT2Sign = (address >> (0+7+6+1+1+3+2))        & ((1<<1)-1);
    theta     = (address >> (0+7+6+1+1+3+2+1))      & ((1<<5)-1);

    dPhi23 = getdPhiFromBin(dPhi23, 7, 512);
    dPhi34 = getdPhiFromBin(dPhi34, 6, 256);
    break;

  case 15:  // 1-2-3-4
    dPhi12    = (address >> (0))                    & ((1<<7)-1);
    dPhi23    = (address >> (0+7))                  & ((1<<5)-1);
    dPhi34    = (address >> (0+7+5))                & ((1<<6)-1);
    sign23    = (address >> (0+7+5+6))              & ((1<<1)-1);
    sign34    = (address >> (0+7+5+6+1))            & ((1<<1)-1);
    FR1       = (address >> (0+7+5+6+1+1))          & ((1<<1)-1);
    theta     = (address >> (0+7+5+6+1+1+1))        & ((1<<5)-1);
    break;

  default:
    dPhi12 = getdPhiFromBin(dPhi12, 7, 512);
    dPhi23 = getdPhiFromBin(dPhi23, 5, 256);
    dPhi34 = getdPhiFromBin(dPhi34, 6, 256);
    break;
  }


  auto get_signed_int = [](int var, int sign) {
    return (sign == 1) ? (var * 1) : (var * -1);
  };

  dPhi12 = get_signed_int(dPhi12, sign12);
  dPhi13 = get_signed_int(dPhi13, sign13);
  dPhi14 = get_signed_int(dPhi14, sign14);
  dPhi23 = get_signed_int(dPhi23, sign23);
  dPhi24 = get_signed_int(dPhi24, sign24);
  dPhi34 = get_signed_int(dPhi34, sign34);

  CLCT1  = get_signed_int(CLCT1, CLCT1Sign);
  CLCT2  = get_signed_int(CLCT2, CLCT2Sign);
  CLCT3  = get_signed_int(CLCT3, CLCT3Sign);
  CLCT4  = get_signed_int(CLCT4, CLCT4Sign);

  //float ftheta = getEtaFromBin(theta, 5);  // eta = getEtaFromBin(eta, 5);
  float ftheta = getEtaIntFromThetaInt(theta, 5);


  // First fix to recover high pT muons with 3 hits in a line and one displaced hit - AWB 28.07.16
  // Done by re-writing a few addresses in the original LUT, according to the following logic
  // Implemented in FW 26.07.16, as of run 2774278 / fill 5119
  if (fixMode15HighPt_) {
    if (mode_inv == 15) {
      bool st2_off = false;
      bool st3_off = false;
      bool st4_off = false;

      int dPhi13 = dPhi12 + dPhi23;
      int dPhi14 = dPhi13 + dPhi34;
      int dPhi24 = dPhi23 + dPhi34;

      int sum_st1 = abs( dPhi12 + dPhi13 + dPhi14);
      int sum_st2 = abs(-dPhi12 + dPhi23 + dPhi24);
      int sum_st3 = abs(-dPhi13 - dPhi23 + dPhi34);
      int sum_st4 = abs(-dPhi14 - dPhi24 - dPhi34);

      if (sum_st2 > sum_st1 && sum_st2 > sum_st3 && sum_st2 > sum_st4) st2_off = true;
      if (sum_st3 > sum_st1 && sum_st3 > sum_st2 && sum_st3 > sum_st4) st3_off = true;
      if (sum_st4 > sum_st1 && sum_st4 > sum_st2 && sum_st4 > sum_st3) st4_off = true;

      if ( st2_off && ( abs(dPhi12) > 9 || abs(dPhi23) > 9 || abs(dPhi24) > 9 ) &&
           abs(dPhi13) < 10 && abs(dPhi14) < 10 && abs(dPhi34) < 10 ) {
        dPhi12 = ceil(dPhi13 / 2); dPhi23 = floor(dPhi13 / 2);
      }
      if ( st3_off && ( abs(dPhi13) > 9 || abs(dPhi23) > 9 || abs(dPhi34) > 9 ) &&
           abs(dPhi12) < 10 && abs(dPhi14) < 10 && abs(dPhi24) < 10 ) {
        dPhi23 = ceil(dPhi24 / 2); dPhi34 = floor(dPhi24 / 2);
      }
      if ( st4_off && ( abs(dPhi14) > 9 || abs(dPhi24) > 9 || abs(dPhi34) > 9 ) &&
           abs(dPhi12) < 10 && abs(dPhi13) < 10 && abs(dPhi23) < 10 ) {
        if ( abs(dPhi13) < abs(dPhi23) )
          dPhi34 = dPhi13;
        else
          dPhi34 = dPhi23;
      }
    }
  }


  const int (*mode_variables)[6] = ModeVariables_Scheme3;

  const int variables[24] = {
    dPhi12, dPhi13, dPhi14, dPhi23, dPhi24, dPhi34, dTheta12, dTheta13, dTheta14, dTheta23, dTheta24, dTheta34,
    CLCT1, CLCT2, CLCT3, CLCT4, CSCID1, CSCID2, CSCID3, CSCID4, FR1, FR2, FR3, FR4
  };

  std::vector<Double_t> tree_data;
  tree_data.push_back(1.0);
  tree_data.push_back(ftheta);  // tree_data.push_back(eta);

  for (int i=0; i<6; i++) {
    int mv = mode_variables[mode_inv-3][i];
    if (mv != -999) {
      int v = variables[mv];
      tree_data.push_back(v);
    }
  }

  //std::cout << "mode_inv: " << mode_inv << " variables: ";
  //for (const auto& v: tree_data)
  //  std::cout << v << " ";
  //std::cout << std::endl;

  auto tree_event = std::make_unique<Event>();
  tree_event->data = tree_data;

  forests_.at(mode_inv).predictEvent(tree_event.get(), 64);
  float tmp_pt = tree_event->predictedValue;
  pt = (tmp_pt != 0) ? 1.0/tmp_pt : tmp_pt;

  return pt;
}
