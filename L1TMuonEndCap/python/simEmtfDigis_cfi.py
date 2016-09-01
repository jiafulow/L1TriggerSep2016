import FWCore.ParameterSet.Config as cms

# EMTF emulator configuration
# Three options for CSCInput
#   * 'simCscTriggerPrimitiveDigis','MPCSORTED' : simulated trigger primitives (LCTs) from re-emulating CSC digis
#   * 'csctfDigis' : real trigger primitives as received by CSCTF (legacy trigger)
#   * 'emtfStage2Digis' : real trigger primitives as received by EMTF, unpacked in EventFilter/L1TRawToDigi/
simEmtfDigis = cms.EDProducer("L1TMuonEndCapTrackProducerSep2016",
    CSCInput = cms.InputTag('simCscTriggerPrimitiveDigis','MPCSORTED'),
    RPCInput = cms.InputTag('simMuonRPCDigis'),
)
