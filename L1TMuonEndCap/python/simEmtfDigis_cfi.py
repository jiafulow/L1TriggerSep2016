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

    # BX
    MinBX = cms.int32(-3),
    MaxBX = cms.int32(+4),

    # Versioning
    Version = cms.int32(1),
    PtLUTVersion = cms.int32(1),

    # Sector processor primitive-conversion parameters
    spPCParams16 = cms.PSet(
        PhThLUT = cms.string('ph_lut_v1'),
        IncludeNeighbor = cms.bool(True),
        DuplicateTheta = cms.bool(True),
        FixZonePhi = cms.bool(False), ## False in FW through the present - AWB 04.10.16
    ),

    # Sector processor pattern-recognition parameters
    spPRParams16 = cms.PSet(
        ZoneBoundaries1 = cms.vint32(0,42,50,88),   ## Are these used for anything? - AWB 07.10.16
        # ZoneBoundaries2 = cms.vint32(41,49,87,127),
        ZoneBoundaries2 = cms.vint32(36,54,96,127), ## Proposed for new_zones_AWB - AWB 07.10.16
        ZoneOverlap = cms.int32(2),
        PatternDefinitions = cms.vstring(
            # straightness, hits in ME1, hits in ME2, hits in ME3, hits in ME4
            # ME1 vaues centered at 15, range from 0 - 30
            # ME2,3,4 values centered at 7, range from 0 - 14
            # Is the "center" assumed somewhere, or do we need to make it configurable? - AWB 29.09.16
            "4,15:15,7:7,7:7,7:7",
            "3,16:16,7:7,7:6,7:6",
            "3,14:14,7:7,8:7,8:7",
            "2,18:17,7:7,7:5,7:5",
            "2,13:12,7:7,10:7,10:7",  # should be 9:7 in ME3,4 (bug in FW or emulator? - AWB 29.09.16)
            "1,22:19,7:7,7:0,7:0",
            "1,11:8,7:7,14:7,14:7",
            "0,30:23,7:7,7:0,7:0",
            "0,7:0,7:7,14:7,14:7",
        ),
        SymPatternDefinitions = cms.vstring(
            # straightness, hits in ME1, hits in ME2, hits in ME3, hits in ME4
            "4,15:15:15:15,7:7:7:7,7:7:7:7,7:7:7:7",
            "3,16:16:14:14,7:7:7:7,8:7:7:6,8:7:7:6",
            "2,18:17:13:12,7:7:7:7,10:7:7:4,10:7:7:4",
            "1,22:19:11:8,7:7:7:7,14:7:7:0,14:7:7:0",
            "0,30:23:7:0,7:7:7:7,14:7:7:0,14:7:7:0",
        ),
        MaxRoadsPerZone = cms.int32(3),
        ThetaWindow = cms.int32(4),
        MaxTracks = cms.int32(3),
        UseSecondEarliest = cms.bool(False), ## Code for this not 100% at the moment - AWB 07.10.16
        UseSymmetricalPatterns = cms.bool(False), ## False in FW through the present - AWB 04.10.16
    ),

    # Sector processor pt-assignment parameters
    spPAParams16 = cms.PSet(
        BDTXMLDir = cms.string('v_16_02_21'),
        ReadPtLUTFile = cms.bool(False),
        FixMode15HighPt = cms.bool(True),
        Fix9bDPhi = cms.bool(False), ## False in FW through present - AWB 06.10.16
    ),

    # Sector processor ghost-cancellation parameters
    spGCParams16 = cms.PSet(

    ),
)

simEmtfDigisData = simEmtfDigis.clone(
    CSCInput = cms.InputTag('emtfStage2Digis'),
    RPCInput = cms.InputTag('muonRPCDigis'),
)
