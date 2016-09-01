import FWCore.ParameterSet.Config as cms

process = cms.Process("L1TMuonEndCap")

process.load("FWCore.MessageService.MessageLogger_cfi")

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(100)
)

# Input source
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring('file:pippo.root'),
)

process.options = cms.untracked.PSet()

process.load('L1TriggerSep2016.L1TMuonEndCap.simEmtfDigis_cfi')
process.simEmtfDigisData.verbosity = 2

process.p = cms.Path(process.simEmtfDigisData)

process.schedule = cms.Schedule(process.p)

