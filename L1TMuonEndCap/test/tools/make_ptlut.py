import FWCore.ParameterSet.Config as cms

process = cms.Process("Whatever")

process.source = cms.Source("EmptySource")

process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(1))

process.analyzer1 = cms.EDAnalyzer("MakeEMTFPtLUT",
    # Verbosity level
    verbosity = cms.untracked.int32(0),

    # Sector processor pt-assignment parameters
    spPAParams16 = cms.PSet(
        BDTXMLDir = cms.string('v_16_02_21'),
        ReadPtLUTFile = cms.bool(False),
        FixMode15HighPt = cms.bool(True),
        Fix9bDPhi = cms.bool(True),
    ),

    # Output file
    outfile = cms.string(""),
)

import os
outfile = os.environ.get("CMSSW_BASE") + "/"
#outfile += "src/L1TriggerSep2016/L1TMuonEndCap/data/emtf_luts/v_16_02_21_ptlut_jftest/LUT_AndrewFix_25July16.dat"
outfile += "src/L1TriggerSep2016/L1TMuonEndCap/data/emtf_luts/v_16_02_21_ptlut_jftest2/LUT_AndrewFix_25July16.dat"
process.analyzer1.outfile = outfile  # make sure the directory exists


process.path1 = cms.Path(process.analyzer1)
