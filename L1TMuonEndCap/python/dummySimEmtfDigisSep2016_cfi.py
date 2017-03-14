import FWCore.ParameterSet.Config as cms

# A dummy module used for debugging/profiling purposes

dummySimEmtfDigisSep2016 = cms.EDProducer("DummyL1TMuonEndCapTrackProducerSep2016",
    # Verbosity level
    verbosity = cms.untracked.int32(0),

    # Input collections
    CSCInput = cms.InputTag('emtfStage2Digis'),
    RPCInput = cms.InputTag('muonRPCDigis'),

    # Run with CSC, RPC
    CSCEnable = cms.bool(True),
    RPCEnable = cms.bool(False),
)
