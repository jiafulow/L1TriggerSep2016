#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPtAssignment.hh"


void EMTFPtAssignment::configure(
    int endcap, int sector, int bx
) {
  endcap_ = endcap;
  sector_ = sector;
  bx_     = bx;
}
