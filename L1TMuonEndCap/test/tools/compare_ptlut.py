import FWCore.ParameterSet.Config as cms

process = cms.Process("Whatever")

process.source = cms.Source("EmptySource")

process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(1))

process.analyzer1 = cms.EDAnalyzer("CompareEMTFPtLUT",

)

process.path1 = cms.Path(process.analyzer1)
