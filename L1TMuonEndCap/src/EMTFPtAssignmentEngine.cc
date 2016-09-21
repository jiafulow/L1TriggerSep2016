#include "L1TriggerSep2016/L1TMuonEndCap/interface/EMTFPtAssignmentEngine.hh"

#include <cassert>
#include <iostream>
#include <fstream>

#include "FWCore/ParameterSet/interface/FileInPath.h"
#include "FWCore/Utilities/interface/Exception.h"


EMTFPtAssignmentEngine::EMTFPtAssignmentEngine() :
    ok_(false)
{

}

EMTFPtAssignmentEngine::~EMTFPtAssignmentEngine() {

}
