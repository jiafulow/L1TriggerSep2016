import FWCore.ParameterSet.Config as cms

process = cms.Process("L1TMuonEndCap")
runOnMC = False

from FWCore.ParameterSet.VarParsing import VarParsing
options = VarParsing('analysis')
options.parseArguments()
# Modify the defaults
if not options.inputFiles:
    options.inputFiles = ['file:pippo.root']
if options.outputFile == "output.root":
    options.outputFile = "output.root"


# MessageLogger
process.load("FWCore.MessageService.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = 1

# Number of events
process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(options.maxEvents)
)

# Input source
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(options.inputFiles),
)

process.options = cms.untracked.PSet()


# simEmtfDigis
process.load('L1TriggerSep2016.L1TMuonEndCap.simEmtfDigis_cfi')
process.simEmtfDigisData.verbosity = 2

process.p = cms.Path(process.simEmtfDigisData)

process.schedule = cms.Schedule(process.p)

