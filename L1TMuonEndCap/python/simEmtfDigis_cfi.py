import FWCore.ParameterSet.Config as cms

# EMTF emulator configuration
# Three options for CSCInput
#   * 'simCscTriggerPrimitiveDigis','MPCSORTED' : simulated trigger primitives (LCTs) from re-emulating CSC digis
#   * 'csctfDigis' : real trigger primitives as received by CSCTF (legacy trigger)
#   * 'emtfStage2Digis' : real trigger primitives as received by EMTF, unpacked in EventFilter/L1TRawToDigi/
simEmtfDigis = cms.EDProducer("L1TMuonEndCapTrackProducerSep2016",
    # Verbosity level
    verbosity = cms.untracked.int32(0),

    # Input collections
    CSCInput = cms.InputTag('simCscTriggerPrimitiveDigis','MPCSORTED'),
    RPCInput = cms.InputTag('simMuonRPCDigis'),
    #GEMInput = cms.InputTag('simMuonGEMPadDigis'),

    # Run with CSC, RPC
    CSCEnable = cms.bool(True),
    RPCEnable = cms.bool(False),

    # LUT files
    PhThLUT = cms.string('ph_lut_v1'),

    # Sector processor primitive-conversion parameters
    spPCParams16 = cms.PSet(
        IncludeNeighbor = cms.bool(True),
        DuplicateWires = cms.bool(True),
    ),

    # Sector processor pattern-recognition parameters
    spPRParams16 = cms.PSet(
        MinBX = cms.int32(-3),
        MaxBX = cms.int32(+4),
        BXWindow = cms.int32(3),
        ZoneBoundaries1 = cms.vint32(0,42,50,88),
        ZoneBoundaries2 = cms.vint32(41,49,87,127),
        ZoneOverlap = cms.int32(2),
    ),

    # Sector processor pt-assignment parameters
    spPAParams16 = cms.PSet(

    ),

    # Sector processor ghost-cancellation parameters
    spGCParams16 = cms.PSet(

    ),
)

simEmtfDigisData = simEmtfDigis.clone(
    CSCInput = cms.InputTag('emtfStage2Digis'),
    RPCInput = cms.InputTag('muonRPCDigis'),
)
