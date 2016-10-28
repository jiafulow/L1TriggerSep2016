import FWCore.ParameterSet.Config as cms

process = cms.Process("Whatever")

process.load('Configuration.Geometry.GeometryExtended2016Reco_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")

from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:run2_mc', '')
print "Using GlobalTag: %s" % process.GlobalTag.globaltag.value()

process.source = cms.Source("EmptySource")

process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(1))

process.analyzer1 = cms.EDAnalyzer("MakeEMTFCoordLUT",
    # Verbosity level
    verbosity = cms.untracked.int32(1),

    # Output diectory
    outdir = cms.string("./"),

    # Produce "validate.root" to validate the LUTs
    please_validate = cms.bool(True),
)

process.path1 = cms.Path(process.analyzer1)
