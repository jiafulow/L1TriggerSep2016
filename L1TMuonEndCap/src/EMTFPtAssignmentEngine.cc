#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPtAssignmentEngine.hh"

#include <cassert>
#include <iostream>

#include "FWCore/Utilities/interface/Exception.h"


EMTFPtAssignmentEngine::EMTFPtAssignmentEngine() :
    allowedModes_({3,5,9,6,10,12,7,11,13,14,15}),
    ok_(false)
{

}

EMTFPtAssignmentEngine::~EMTFPtAssignmentEngine() {

}

void EMTFPtAssignmentEngine::read(const std::string& tree_ver) {
  if (ok_)  return;

  //std::string tree_dir = "L1Trigger/L1TMuon/data/emtf_luts/" + tree_ver + "/ModeVariables/trees";
  std::string tree_dir = "L1TriggerSep2016/L1TMuonEndCap/data/emtf_luts/" + tree_ver + "/ModeVariables/trees";

  for (unsigned i = 0; i < allowedModes_.size(); ++i) {
    int mode_inv = allowedModes_[i];  // inverted mode because reasons
    std::stringstream ss;
    ss << tree_dir << "/" << mode_inv;
    forest_[mode_inv].loadForestFromXML(ss.str().c_str(), 64);
  }

  ok_ = true;
  return;
}

EMTFPtAssignmentEngine::address_t EMTFPtAssignmentEngine::calculate_address(const EMTFTrackExtra& track) const {
  address_t address = 0;

  int mode_inv  = track.mode_inv;
  int theta     = track.theta_int;

  const EMTFPtLUTData& ptlut_data = track.ptlut_data;

  // FIXME
  uint16_t dPhi12   = ptlut_data.delta_ph[0];
  uint16_t dPhi13   = ptlut_data.delta_ph[1];
  uint16_t dPhi14   = ptlut_data.delta_ph[2];
  uint16_t dPhi23   = ptlut_data.delta_ph[3];
  uint16_t dPhi24   = ptlut_data.delta_ph[4];
  uint16_t dPhi34   = ptlut_data.delta_ph[5];
  uint16_t dTheta12 = ptlut_data.delta_th[0];
  uint16_t dTheta13 = ptlut_data.delta_th[1];
  uint16_t dTheta14 = ptlut_data.delta_th[2];
  uint16_t dTheta23 = ptlut_data.delta_th[3];
  uint16_t dTheta24 = ptlut_data.delta_th[4];
  uint16_t dTheta34 = ptlut_data.delta_th[5];
  uint16_t CLCT1    = ptlut_data.cpattern[0];
  uint16_t CLCT2    = ptlut_data.cpattern[1];
  uint16_t CLCT3    = ptlut_data.cpattern[2];
  uint16_t CLCT4    = ptlut_data.cpattern[3];
  uint16_t FR1      = ptlut_data.fr      [0];
  uint16_t FR2      = ptlut_data.fr      [1];
  uint16_t FR3      = ptlut_data.fr      [2];
  uint16_t FR4      = ptlut_data.fr      [3];

  // FIXME
  uint16_t CLCT1Sign = 0;
  uint16_t CLCT2Sign = 0;
  uint16_t CLCT3Sign = 0;
  uint16_t CLCT4Sign = 0;

  uint16_t sign12 = 0;
  uint16_t sign13 = 0;
  uint16_t sign14 = 0;
  uint16_t sign23 = 0;
  uint16_t sign24 = 0;
  uint16_t sign34 = 0;

  uint16_t eta = 0;

  uint16_t Mode = 0;

  switch(mode_inv) {
  case 3:   // 1-2
    address |= (dPhi12      & ((1<<9)-1)) << (0);
    address |= (sign12      & ((1<<1)-1)) << (0+9);
    address |= (dTheta12    & ((1<<3)-1)) << (0+9+1);
    address |= (CLCT1       & ((1<<2)-1)) << (0+9+1+3);
    address |= (CLCT1Sign   & ((1<<1)-1)) << (0+9+1+3+2);
    address |= (CLCT2       & ((1<<2)-1)) << (0+9+1+3+2+1);
    address |= (CLCT2Sign   & ((1<<1)-1)) << (0+9+1+3+2+1+2);
    address |= (FR1         & ((1<<1)-1)) << (0+9+1+3+2+1+2+1);
    address |= (FR2         & ((1<<1)-1)) << (0+9+1+3+2+1+2+1+1);
    address |= (eta         & ((1<<5)-1)) << (0+9+1+3+2+1+2+1+1+1);
    address |= (Mode        & ((1<<4)-1)) << (0+9+1+3+2+1+2+1+1+1+5);
    break;

  case 5:   // 1-3
    address |= (dPhi13      & ((1<<9)-1)) << (0);
    address |= (sign13      & ((1<<1)-1)) << (0+9);
    address |= (dTheta13    & ((1<<3)-1)) << (0+9+1);
    address |= (CLCT1       & ((1<<2)-1)) << (0+9+1+3);
    address |= (CLCT1Sign   & ((1<<1)-1)) << (0+9+1+3+2);
    address |= (CLCT3       & ((1<<2)-1)) << (0+9+1+3+2+1);
    address |= (CLCT3Sign   & ((1<<1)-1)) << (0+9+1+3+2+1+2);
    address |= (FR1         & ((1<<1)-1)) << (0+9+1+3+2+1+2+1);
    address |= (FR3         & ((1<<1)-1)) << (0+9+1+3+2+1+2+1+1);
    address |= (eta         & ((1<<5)-1)) << (0+9+1+3+2+1+2+1+1+1);
    address |= (Mode        & ((1<<4)-1)) << (0+9+1+3+2+1+2+1+1+1+5);
    break;

  case 9:   // 1-4
    address |= (dPhi14      & ((1<<9)-1)) << (0);
    address |= (sign14      & ((1<<1)-1)) << (0+9);
    address |= (dTheta14    & ((1<<3)-1)) << (0+9+1);
    address |= (CLCT1       & ((1<<2)-1)) << (0+9+1+3);
    address |= (CLCT1Sign   & ((1<<1)-1)) << (0+9+1+3+2);
    address |= (CLCT4       & ((1<<2)-1)) << (0+9+1+3+2+1);
    address |= (CLCT4Sign   & ((1<<1)-1)) << (0+9+1+3+2+1+2);
    address |= (FR1         & ((1<<1)-1)) << (0+9+1+3+2+1+2+1);
    address |= (FR4         & ((1<<1)-1)) << (0+9+1+3+2+1+2+1+1);
    address |= (eta         & ((1<<5)-1)) << (0+9+1+3+2+1+2+1+1+1);
    address |= (Mode        & ((1<<4)-1)) << (0+9+1+3+2+1+2+1+1+1+5);
    break;

  case 6:   // 2-3
    address |= (dPhi23      & ((1<<9)-1)) << (0);
    address |= (sign23      & ((1<<1)-1)) << (0+9);
    address |= (dTheta23    & ((1<<3)-1)) << (0+9+1);
    address |= (CLCT2       & ((1<<2)-1)) << (0+9+1+3);
    address |= (CLCT2Sign   & ((1<<1)-1)) << (0+9+1+3+2);
    address |= (CLCT3       & ((1<<2)-1)) << (0+9+1+3+2+1);
    address |= (CLCT3Sign   & ((1<<1)-1)) << (0+9+1+3+2+1+2);
    address |= (FR2         & ((1<<1)-1)) << (0+9+1+3+2+1+2+1);
    address |= (FR3         & ((1<<1)-1)) << (0+9+1+3+2+1+2+1+1);
    address |= (eta         & ((1<<5)-1)) << (0+9+1+3+2+1+2+1+1+1);
    address |= (Mode        & ((1<<4)-1)) << (0+9+1+3+2+1+2+1+1+1+5);
    break;

  case 10:  // 2-4
    address |= (dPhi24      & ((1<<9)-1)) << (0);
    address |= (sign24      & ((1<<1)-1)) << (0+9);
    address |= (dTheta24    & ((1<<3)-1)) << (0+9+1);
    address |= (CLCT2       & ((1<<2)-1)) << (0+9+1+3);
    address |= (CLCT2Sign   & ((1<<1)-1)) << (0+9+1+3+2);
    address |= (CLCT4       & ((1<<2)-1)) << (0+9+1+3+2+1);
    address |= (CLCT4Sign   & ((1<<1)-1)) << (0+9+1+3+2+1+2);
    address |= (FR2         & ((1<<1)-1)) << (0+9+1+3+2+1+2+1);
    address |= (FR4         & ((1<<1)-1)) << (0+9+1+3+2+1+2+1+1);
    address |= (eta         & ((1<<5)-1)) << (0+9+1+3+2+1+2+1+1+1);
    address |= (Mode        & ((1<<4)-1)) << (0+9+1+3+2+1+2+1+1+1+5);
    break;

  case 12:  // 3-4
    address |= (dPhi34      & ((1<<9)-1)) << (0);
    address |= (sign34      & ((1<<1)-1)) << (0+9);
    address |= (dTheta34    & ((1<<3)-1)) << (0+9+1);
    address |= (CLCT3       & ((1<<2)-1)) << (0+9+1+3);
    address |= (CLCT3Sign   & ((1<<1)-1)) << (0+9+1+3+2);
    address |= (CLCT4       & ((1<<2)-1)) << (0+9+1+3+2+1);
    address |= (CLCT4Sign   & ((1<<1)-1)) << (0+9+1+3+2+1+2);
    address |= (FR3         & ((1<<1)-1)) << (0+9+1+3+2+1+2+1);
    address |= (FR4         & ((1<<1)-1)) << (0+9+1+3+2+1+2+1+1);
    address |= (eta         & ((1<<5)-1)) << (0+9+1+3+2+1+2+1+1+1);
    address |= (Mode        & ((1<<4)-1)) << (0+9+1+3+2+1+2+1+1+1+5);
    break;

  case 7:   // 1-2-3
    address |= (dPhi12      & ((1<<7)-1)) << (0);
    address |= (dPhi23      & ((1<<5)-1)) << (0+7);
    address |= (sign12      & ((1<<1)-1)) << (0+7+5);
    address |= (sign23      & ((1<<1)-1)) << (0+7+5+1);
    address |= (dTheta13    & ((1<<3)-1)) << (0+7+5+1+1);
    address |= (CLCT1       & ((1<<2)-1)) << (0+7+5+1+1+3);
    address |= (CLCT1Sign   & ((1<<1)-1)) << (0+7+5+1+1+3+2);
    address |= (FR1         & ((1<<1)-1)) << (0+7+5+1+1+3+2+1);
    address |= (eta         & ((1<<5)-1)) << (0+7+5+1+1+3+2+1+1);
    address |= (Mode        & ((1<<4)-1)) << (0+7+5+1+1+3+2+1+1+5);
    break;

  case 11:  // 1-2-4
    address |= (dPhi12      & ((1<<7)-1)) << (0);
    address |= (dPhi24      & ((1<<5)-1)) << (0+7);
    address |= (sign12      & ((1<<1)-1)) << (0+7+5);
    address |= (sign24      & ((1<<1)-1)) << (0+7+5+1);
    address |= (dTheta14    & ((1<<3)-1)) << (0+7+5+1+1);
    address |= (CLCT1       & ((1<<2)-1)) << (0+7+5+1+1+3);
    address |= (CLCT1Sign   & ((1<<1)-1)) << (0+7+5+1+1+3+2);
    address |= (FR1         & ((1<<1)-1)) << (0+7+5+1+1+3+2+1);
    address |= (eta         & ((1<<5)-1)) << (0+7+5+1+1+3+2+1+1);
    address |= (Mode        & ((1<<4)-1)) << (0+7+5+1+1+3+2+1+1+5);
    break;

  case 13:  // 1-3-4
    address |= (dPhi13      & ((1<<7)-1)) << (0);
    address |= (dPhi34      & ((1<<5)-1)) << (0+7);
    address |= (sign13      & ((1<<1)-1)) << (0+7+5);
    address |= (sign34      & ((1<<1)-1)) << (0+7+5+1);
    address |= (dTheta14    & ((1<<3)-1)) << (0+7+5+1+1);
    address |= (CLCT1       & ((1<<2)-1)) << (0+7+5+1+1+3);
    address |= (CLCT1Sign   & ((1<<1)-1)) << (0+7+5+1+1+3+2);
    address |= (FR1         & ((1<<1)-1)) << (0+7+5+1+1+3+2+1);
    address |= (eta         & ((1<<5)-1)) << (0+7+5+1+1+3+2+1+1);
    address |= (Mode        & ((1<<4)-1)) << (0+7+5+1+1+3+2+1+1+5);
    break;

  case 14:  // 2-3-4
    address |= (dPhi23      & ((1<<7)-1)) << (0);
    address |= (dPhi34      & ((1<<6)-1)) << (0+7);
    address |= (sign23      & ((1<<1)-1)) << (0+7+6);
    address |= (sign34      & ((1<<1)-1)) << (0+7+6+1);
    address |= (dTheta24    & ((1<<3)-1)) << (0+7+6+1+1);
    address |= (CLCT2       & ((1<<2)-1)) << (0+7+6+1+1+3);
    address |= (CLCT2Sign   & ((1<<1)-1)) << (0+7+6+1+1+3+2);
    address |= (eta         & ((1<<5)-1)) << (0+7+6+1+1+3+2+1);
    address |= (Mode        & ((1<<4)-1)) << (0+7+6+1+1+3+2+1+5);
    break;

  case 15:  // 1-2-3-4
    address |= (dPhi12      & ((1<<7)-1)) << 0;
    address |= (dPhi23      & ((1<<5)-1)) << (0+7);
    address |= (dPhi34      & ((1<<6)-1)) << (0+7+5);
    address |= (sign23      & ((1<<1)-1)) << (0+7+5+6);
    address |= (sign34      & ((1<<1)-1)) << (0+7+5+6+1);
    address |= (FR1         & ((1<<1)-1)) << (0+7+5+6+1+1);
    address |= (eta         & ((1<<5)-1)) << (0+7+5+6+1+1+1);
    address |= (Mode        & ((1<<4)-1)) << (0+7+5+6+1+1+1+5);
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

//ModeVariables is a 2D arrary indexed by [TrackMode(13 Total Listed Below)][VariableNumber(20 Total Constructed Above)]
// Variable numbering
// 0 = dPhi12
// 1 = dPhi13
// 2 = dPhi14
// 3 = dPhi23
// 4 = dPhi24
// 5 = dPhi34
// 6 = dEta12
// 7 = dEta13
// 8 = dEta14
// 9 = dEta23
// 10 = dEta24
// 11 = dEta34
// 12 = CLCT1
// 13 = CLCT2
// 14 = CLCT3
// 15 = CLCT4
// 16 = CSCID1
// 17 = CSCID2
// 18 = CSCID3
// 19 = CSCID4
// 20 = FR1
// 21 = FR2
// 22 = FR3
// 23 = FR4

// Bobby's Scheme3 (or "SchemeC"), with 30 bit compression //
//3:TrackEta:dPhi12:dEta12:CLCT1:CLCT2:FR1
//4:Single Station Track Not Possible
//5:TrackEta:dPhi13:dEta13:CLCT1:CLCT3:FR1
//6:TrackEta:dPhi23:dEta23:CLCT2:CLCT3:FR2
//7:TrackEta:dPhi12:dPhi23:dEta13:CLCT1:FR1
//8:Single Station Track Not Possible
//9:TrackEta:dPhi14:dEta14:CLCT1:CLCT4:FR1
//10:TrackEta:dPhi24:dEta24:CLCT2:CLCT4:FR2
//11:TrackEta:dPhi12:dPhi24:dEta14:CLCT1:FR1
//12:TrackEta:dPhi34:dEta34:CLCT3:CLCT4:FR3
//13:TrackEta:dPhi13:dPhi34:dEta14:CLCT1:FR1
//14:TrackEta:dPhi23:dPhi34:dEta24:CLCT2
//15:TrackEta:dPhi12:dPhi23:dPhi34:FR1

static const int ModeVariables_Scheme3[13][6] =
{
    {0,6,12,13,20,-999},              // 3
    {-999,-999,-999,-999,-999,-999},  // 4
    {1,7,12,14,20,-999},              // 5
    {3,9,13,14,21,-999},              // 6
    {0,3,7,12,20,-999},               // 7
    {-999,-999,-999,-999,-999,-999},  // 8
    {2,8,12,15,20,-999},              // 9
    {4,10,13,15,21,-999},             // 10
    {0,4,8,12,20,-999},               // 11
    {5,11,14,15,22,-999},             // 12
    {1,5,8,16,20,-999},               // 13
    {3,5,10,13,-999,-999},            // 14
    {0,3,5,20,-999,-999}              // 15
};

float EMTFPtAssignmentEngine::calculate_pt(const address_t& address) {
  float pt = 0;

  uint16_t mode_inv = (address >> (30-4)) & ((1<<4)-1);

  auto contain = [](const auto& vec, const auto& elem) { return (std::find(vec.begin(), vec.end(), elem) != vec.end()); };

  bool is_good_mode = contain(allowedModes_, mode_inv);

  if (!is_good_mode)  // early exit
    return pt;

  // FIXME
  uint16_t dPhi12   = 0;
  uint16_t dPhi13   = 0;
  uint16_t dPhi14   = 0;
  uint16_t dPhi23   = 0;
  uint16_t dPhi24   = 0;
  uint16_t dPhi34   = 0;
  uint16_t dTheta12 = 0;
  uint16_t dTheta13 = 0;
  uint16_t dTheta14 = 0;
  uint16_t dTheta23 = 0;
  uint16_t dTheta24 = 0;
  uint16_t dTheta34 = 0;
  uint16_t CLCT1    = 0;
  uint16_t CLCT2    = 0;
  uint16_t CLCT3    = 0;
  uint16_t CLCT4    = 0;
  uint16_t CSCID1   = 0;
  uint16_t CSCID2   = 0;
  uint16_t CSCID3   = 0;
  uint16_t CSCID4   = 0;
  uint16_t FR1      = 0;
  uint16_t FR2      = 0;
  uint16_t FR3      = 0;
  uint16_t FR4      = 0;

  // FIXME
  uint16_t CLCT1Sign = 0;
  uint16_t CLCT2Sign = 0;
  uint16_t CLCT3Sign = 0;
  uint16_t CLCT4Sign = 0;

  uint16_t sign12 = 0;
  uint16_t sign13 = 0;
  uint16_t sign14 = 0;
  uint16_t sign23 = 0;
  uint16_t sign24 = 0;
  uint16_t sign34 = 0;

  uint16_t TrackEta = 0;

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
    TrackEta  = (address >> (0+9+1+3+2+1+2+1+1+1))  & ((1<<5)-1);
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
    TrackEta  = (address >> (0+9+1+3+2+1+2+1+1+1))  & ((1<<5)-1);
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
    TrackEta  = (address >> (0+9+1+3+2+1+2+1+1+1))  & ((1<<5)-1);
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
    TrackEta  = (address >> (0+9+1+3+2+1+2+1+1+1))  & ((1<<5)-1);
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
    TrackEta  = (address >> (0+9+1+3+2+1+2+1+1+1))  & ((1<<5)-1);
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
    TrackEta  = (address >> (0+9+1+3+2+1+2+1+1+1))  & ((1<<5)-1);
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
    TrackEta  = (address >> (0+7+5+1+1+3+2+1+1))    & ((1<<5)-1);
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
    TrackEta  = (address >> (0+7+5+1+1+3+2+1+1))    & ((1<<5)-1);
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
    TrackEta  = (address >> (0+7+5+1+1+3+2+1+1))    & ((1<<5)-1);
    break;

  case 14:  // 2-3-4
    dPhi23    = (address >> (0))                    & ((1<<7)-1);
    dPhi34    = (address >> (0+7))                  & ((1<<6)-1);
    sign23    = (address >> (0+7+6))                & ((1<<1)-1);
    sign34    = (address >> (0+7+6+1))              & ((1<<1)-1);
    dTheta24  = (address >> (0+7+6+1+1))            & ((1<<3)-1);
    CLCT2     = (address >> (0+7+6+1+1+3))          & ((1<<2)-1);
    CLCT2Sign = (address >> (0+7+6+1+1+3+2))        & ((1<<1)-1);
    TrackEta  = (address >> (0+7+6+1+1+3+2+1))      & ((1<<5)-1);
    break;

  case 15:  // 1-2-3-4
    dPhi12    = (address >> (0))                    & ((1<<7)-1);
    dPhi23    = (address >> (0+7))                  & ((1<<5)-1);
    dPhi34    = (address >> (0+7+5))                & ((1<<6)-1);
    sign23    = (address >> (0+7+5+6))              & ((1<<1)-1);
    sign34    = (address >> (0+7+5+6+1))            & ((1<<1)-1);
    FR1       = (address >> (0+7+5+6+1+1))          & ((1<<1)-1);
    TrackEta  = (address >> (0+7+5+6+1+1+1))        & ((1<<5)-1);
    break;

  default:
    break;
  }

  bool era_3 = true;

  // First fix to recover high pT muons with 3 hits in a line and one displaced hit - AWB 28.07.16
  // Done by re-writing a few addresses in the original LUT, according to the following logic
  // Implemented in FW 26.07.16, as of run 2774278 / fill 5119
  if (era_3) {
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
  } // end if eta_3

  const int (*mode_variables)[6] = ModeVariables_Scheme3;

  const int variables[24] = {
    dPhi12, dPhi13, dPhi14, dPhi23, dPhi24, dPhi34, dTheta12, dTheta13, dTheta14, dTheta23, dTheta24, dTheta34,
    CLCT1, CLCT2, CLCT3, CLCT4, CSCID1, CSCID2, CSCID3, CSCID4, FR1, FR2, FR3, FR4
  };

  uint16_t eta = 0;

  uint16_t Mode = 0;

  if (CLCT1Sign && CLCT2Sign && CLCT3Sign && CLCT4Sign && sign12 && sign13 && sign14 && sign23 && sign24 && sign34 && TrackEta && Mode) {}  // FIXME: variables set but not used


  std::vector<Double_t> tree_data;
  tree_data.push_back(1.0);
  tree_data.push_back(eta);

  for (int i=0; i<6; i++) {
    int mv = mode_variables[mode_inv-3][i];
    if (mv != -999) {
      int v = variables[mv];
      tree_data.push_back(v);
    }
  }

  if (true) {  // debug
    std::cout << "mode_inv: " << mode_inv << " variables: ";
    for (const auto& v: tree_data)
      std::cout << v << " ";
    std::cout << std::endl;
  }

  std::unique_ptr<Event> tree_event(new Event());
  tree_event->data = tree_data;

  forest_[mode_inv].predictEvent(tree_event.get(), 64);
  float tmp_pt = tree_event->predictedValue;
  tmp_pt = (tmp_pt != 0) ? 1.0/tmp_pt : tmp_pt;

  pt = tmp_pt;
  return pt;
}


// _____________________________________________________________________________
namespace {


// 256 max units----

static const int dPhiNLBMap_5bit_256Max[32] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 19, 20, 21, 23, 25, 28, 31, 34, 39, 46, 55, 68, 91, 136};

static const int dPhiNLBMap_6bit_256Max[64] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 36, 37, 38, 39, 40, 42, 43, 45, 47, 49, 51, 53, 56, 58, 61, 65, 68, 73, 78, 83, 89, 97, 106, 116, 129, 145, 166, 193, 232};

static const int dPhiNLBMap_7bit_256Max[128] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 77, 78, 79, 80, 81, 82, 83, 84, 86, 87, 88, 90, 91, 93, 94, 96, 97, 99, 101, 103, 105, 107, 109, 111, 113, 115, 118, 120, 123, 125, 128, 131, 134, 138, 141, 145, 149, 153, 157, 161, 166, 171, 176, 182, 188, 194, 201, 209, 217, 225, 235, 245};

// 512 max units----

static const int dPhiNLBMap_7bit_512Max[128] =  {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 71, 72, 73, 74, 75, 76, 77, 79, 80, 81, 83, 84, 86, 87, 89, 91, 92, 94, 96, 98, 100, 102, 105, 107, 110, 112, 115, 118, 121, 124, 127, 131, 135, 138, 143, 147, 152, 157, 162, 168, 174, 181, 188, 196, 204, 214, 224, 235, 247, 261, 276, 294, 313, 336, 361, 391, 427, 470};

static const int dPhiNLBMap_8bit_512Max[256] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161, 162, 164, 165, 166, 167, 168, 170, 171, 172, 174, 175, 176, 178, 179, 180, 182, 183, 185, 186, 188, 190, 191, 193, 194, 196, 198, 200, 201, 203, 205, 207, 209, 211, 213, 215, 217, 219, 221, 223, 225, 228, 230, 232, 235, 237, 240, 242, 245, 248, 250, 253, 256, 259, 262, 265, 268, 272, 275, 278, 282, 285, 289, 293, 297, 300, 305, 309, 313, 317, 322, 327, 331, 336, 341, 347, 352, 358, 363, 369, 375, 382, 388, 395, 402, 410, 417, 425, 433, 442, 450, 460, 469, 479, 490, 500};

static const int dPhiNLBMap_5bit[32] = {0, 1, 2, 4, 5, 7, 9, 11, 13, 15, 18, 21, 24, 28, 32, 37, 41, 47, 53, 60, 67, 75, 84, 94, 105, 117, 131, 145, 162, 180, 200, 222};

// Array that maps the 7-bit integer dPhi --> dPhi-units. It is assumed that this is used for dPhi12,
// which has a maximum value of 7.67 degrees (511 units) in the extrapolation units.
static const int dPhiNLBMap_7bit[128] = {0, 1, 2, 3, 4, 5, 6, 8, 9, 10, 11, 12, 14, 15, 16, 17, 19, 20, 21, 23, 24, 26, 27, 29, 30, 32, 33, 35, 37, 38, 40, 42, 44, 45, 47, 49, 51, 53, 55, 57, 59, 61, 63, 65, 67, 70, 72, 74, 77, 79, 81, 84, 86, 89, 92, 94, 97, 100, 103, 105, 108, 111, 114, 117, 121, 124, 127, 130, 134, 137, 141, 144, 148, 151, 155, 159, 163, 167, 171, 175, 179, 183, 188, 192, 197, 201, 206, 210, 215, 220, 225, 230, 235, 241, 246, 251, 257, 263, 268, 274, 280, 286, 292, 299, 305, 312, 318, 325, 332, 339, 346, 353, 361, 368, 376, 383, 391, 399, 408, 416, 425, 433, 442, 451, 460, 469, 479, 489};

// Array that maps the 8-bit integer dPhi --> dPhi-units. It is assumed that this is used for dPhi12,
// which has a maximum value of 7.67 degrees (511 units) in the extrapolation units.
static const int dPhiNLBMap_8bit[256] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 27, 28, 29, 30, 31, 32, 33, 35, 36, 37, 38, 39, 40, 42, 43, 44, 45, 46, 48, 49, 50, 51, 53, 54, 55, 56, 58, 59, 60, 61, 63, 64, 65, 67, 68, 69, 70, 72, 73, 74, 76, 77, 79, 80, 81, 83, 84, 85, 87, 88, 90, 91, 92, 94, 95, 97, 98, 100, 101, 103, 104, 105, 107, 108, 110, 111, 113, 115, 116, 118, 119, 121, 122, 124, 125, 127, 129, 130, 132, 133, 135, 137, 138, 140, 141, 143, 145, 146, 148, 150, 151, 153, 155, 157, 158, 160, 162, 163, 165, 167, 169, 171, 172, 174, 176, 178, 180, 181, 183, 185, 187, 189, 191, 192, 194, 196, 198, 200, 202, 204, 206, 208, 210, 212, 214, 216, 218, 220, 222, 224, 226, 228, 230, 232, 234, 236, 238, 240, 242, 244, 246, 249, 251, 253, 255, 257, 259, 261, 264, 266, 268, 270, 273, 275, 277, 279, 282, 284, 286, 289, 291, 293, 296, 298, 300, 303, 305, 307, 310, 312, 315, 317, 320, 322, 324, 327, 329, 332, 334, 337, 340, 342, 345, 347, 350, 352, 355, 358, 360, 363, 366, 368, 371, 374, 376, 379, 382, 385, 387, 390, 393, 396, 398, 401, 404, 407, 410, 413, 416, 419, 421, 424, 427, 430, 433, 436, 439, 442, 445, 448, 451, 454, 457, 461, 464, 467, 470, 473, 476, 479, 483};

static float getNLBdPhi(float dPhi, int bits, int max=512)
{
  float dPhi_= max;
  float sign_ = 1;
  if (dPhi<0)
    sign_ = -1;
  dPhi = fabs(dPhi);

  if (max==256)
  {
    if (bits == 5)
    {
      dPhi_ = dPhiNLBMap_5bit_256Max[(1<<bits)-1];
      for (int edge=0; edge<(1<<bits)-1; edge++)
      {
        if (dPhiNLBMap_5bit_256Max[edge]<=dPhi && dPhiNLBMap_5bit_256Max[edge+1]>dPhi)
        {
          dPhi_ = dPhiNLBMap_5bit_256Max[edge];
          break;
        }
      }
    }
    if (bits == 6)
    {
      dPhi_ = dPhiNLBMap_6bit_256Max[(1<<bits)-1];
      for (int edge=0; edge<(1<<bits)-1; edge++)
      {
        if (dPhiNLBMap_6bit_256Max[edge]<=dPhi && dPhiNLBMap_6bit_256Max[edge+1]>dPhi)
        {
          dPhi_ = dPhiNLBMap_6bit_256Max[edge];
          break;
        }
      }
    }
    if (bits == 7)
    {
      dPhi_ = dPhiNLBMap_7bit_256Max[(1<<bits)-1];
      for (int edge=0; edge<(1<<bits)-1; edge++)
      {
        if (dPhiNLBMap_7bit_256Max[edge]<=dPhi && dPhiNLBMap_7bit_256Max[edge+1]>dPhi)
        {
          dPhi_ = dPhiNLBMap_7bit_256Max[edge];
          break;
        }
      }
    }
  }

  else if (max==512)
  {
    if (bits == 7)
    {
      dPhi_ = dPhiNLBMap_7bit_512Max[(1<<bits)-1];
      for (int edge=0; edge<(1<<bits)-1; edge++)
      {
        if (dPhiNLBMap_7bit_512Max[edge]<=dPhi && dPhiNLBMap_7bit_512Max[edge+1]>dPhi)
        {
          dPhi_ = dPhiNLBMap_7bit_512Max[edge];
          break;
        }
      }
    }
    if (bits == 8)
    {
      dPhi_ = dPhiNLBMap_8bit_512Max[(1<<bits)-1];
      for (int edge=0; edge<(1<<bits)-1; edge++)
      {
        if (dPhiNLBMap_8bit_512Max[edge]<=dPhi && dPhiNLBMap_8bit_512Max[edge+1]>dPhi)
        {
          dPhi_ = dPhiNLBMap_8bit_512Max[edge];
          break;
        }
      }
    }
  }

  if (dPhi>=max) dPhi_ = max;
  return (sign_ * dPhi_);
}

static int getNLBdPhiBin(float dPhi, int bits, int max=512)
{
  int dPhiBin_= (1<<bits)-1;
  //float sign_ = 1;
  //if (dPhi<0)
  //  sign_ = -1;
  dPhi = fabs(dPhi);

  if (max==256)
  {
    if (bits == 5)
    {
      for (int edge=0; edge<(1<<bits)-1; edge++)
      {
        if (dPhiNLBMap_5bit_256Max[edge]<=dPhi && dPhiNLBMap_5bit_256Max[edge+1]>dPhi)
        {
          dPhiBin_ = edge;
          break;
        }
      }
    }
    if (bits == 6)
    {
      for (int edge=0; edge<(1<<bits)-1; edge++)
      {
        if (dPhiNLBMap_6bit_256Max[edge]<=dPhi && dPhiNLBMap_6bit_256Max[edge+1]>dPhi)
        {
          dPhiBin_ = edge;
          break;
        }
      }
    }
    if (bits == 7)
    {
      for (int edge=0; edge<(1<<bits)-1; edge++)
      {
        if (dPhiNLBMap_7bit_256Max[edge]<=dPhi && dPhiNLBMap_7bit_256Max[edge+1]>dPhi)
        {
          dPhiBin_ = edge;
          break;
        }
      }
    }
  }

  else if (max==512)
  {
    if (bits == 7)
    {
      for (int edge=0; edge<(1<<bits)-1; edge++)
      {
        if (dPhiNLBMap_7bit_512Max[edge]<=dPhi && dPhiNLBMap_7bit_512Max[edge+1]>dPhi)
        {
          dPhiBin_ = edge;
          break;
        }
      }
    }
    if (bits == 8)
    {
      for (int edge=0; edge<(1<<bits)-1; edge++)
      {
        if (dPhiNLBMap_8bit_512Max[edge]<=dPhi && dPhiNLBMap_8bit_512Max[edge+1]>dPhi)
        {
          dPhiBin_ = edge;
          break;
        }
      }
    }
  }

  return (dPhiBin_);
}

static float getdPhiFromBin(int dPhiBin, int bits, int max=512)
{
  int dPhi_= (1<<bits)-1;

  if (dPhiBin>(1<<bits)-1)
    dPhiBin = (1<<bits)-1;

  if (max==256)
  {
    if (bits == 5)
      dPhi_ = dPhiNLBMap_5bit_256Max[dPhiBin];

    if (bits == 6)
      dPhi_ = dPhiNLBMap_6bit_256Max[dPhiBin];

    if (bits == 7)
      dPhi_ = dPhiNLBMap_7bit_256Max[dPhiBin];
  }

  else if (max==512)
  {
    if (bits == 7)
      dPhi_ = dPhiNLBMap_7bit_512Max[dPhiBin];

    if (bits == 8)
      dPhi_ = dPhiNLBMap_8bit_512Max[dPhiBin];
  }

  return (dPhi_);
}

static int getCLCT(int clct)
{
  int clct_ = 0;
  int sign_ = 1;

  switch (clct) {
  case 10: clct_ = 0; sign_ =  1; break;
  case  9: clct_ = 1; sign_ =  1; break;
  case  8: clct_ = 1; sign_ = -1; break;
  case  7: clct_ = 2; sign_ =  1; break;
  case  6: clct_ = 2; sign_ = -1; break;
  case  5: clct_ = 3; sign_ =  1; break;
  case  4: clct_ = 3; sign_ = -1; break;
  case  3: clct_ = 3; sign_ =  1; break;
  case  2: clct_ = 3; sign_ = -1; break;
  case  1: clct_ = 3; sign_ = -1; break;  //case  1: clct_ = 3; sign_ =  1; break;
  case  0: clct_ = 3; sign_ = -1; break;
  default: clct_ = 3; sign_ = -1; break;
  }

  return (sign_ * clct_);
}

static int getdTheta(float dTheta)
{
  int dTheta_ = 0;
  //int sign_ = 1;
  //if (dTheta<0)
  //  sign_ = -1;
  if (dTheta<=-3)
    dTheta_ = 0;
  else if (dTheta<=-2)
    dTheta_ = 1;
  else if (dTheta<=-1)
    dTheta_ = 2;
  else if (dTheta<=0)
    dTheta_ = 3;
  else if (dTheta<=1)
    dTheta_ = 4;
  else if (dTheta<=2)
    dTheta_ = 5;
  else if (dTheta<=3)
    dTheta_ = 6;
  else
    dTheta_ = 7;

  return (dTheta_);
}

//static float getdEta(float deta)
//{
//  float deta_ = 0;
//
//  if (deta<=-5)
//    deta_ = 0;
//  else if (deta<=-2)
//    deta_ = 1;
//  else if (deta<=-1)
//    deta_ = 2;
//  else if (deta<=0)
//    deta_ = 3;
//  else if (deta<=1)
//    deta_ = 4;
//  else if (deta<=3)
//    deta_ = 5;
//  else if (deta<=6)
//    deta_ = 6;
//  else
//    deta_ = 7;
//
//  return (deta_);
//}

static float getEta(float eta, int bits=5)
{
  float eta_ = 0;
  float sign_ = 1;
  if (eta<0)
    sign_ = -1;

  if (bits>5) bits = 5;
  int shift = 5 - bits;
  int etaInt = (fabs(eta) - 0.9)*(32.0/(1.6))-0.5;
  etaInt = (etaInt>>shift)<<shift;

  eta_ = 0.9 + (etaInt + 0.5)*(1.6/32.0);
  return (eta_*sign_);
}

static int getEtaInt(float eta, int bits=5)
{
  if (bits>5) bits = 5;
  int shift = 5 - bits;
  int etaInt = (fabs(eta) - 0.9)*(32.0/(1.6))-0.5;
  etaInt = (etaInt>>shift);
  if(etaInt > 31){etaInt = 31;}

  return (etaInt);
}

static float getEtafromBin(int etaBin, int bits=5)
{
  if (etaBin>((1<<5)-1))
    etaBin = ((1<<5)-1);
  if (etaBin<0)
    etaBin = 0;

  if (bits>5) bits = 5;
  int shift = 5 - bits;
  int etaInt_ = etaBin << shift;
  float eta_ = 0.9 + (etaInt_ + 0.5)*(1.6/32.0);
  return (eta_);
}


}  // namespace
