import FWCore.ParameterSet.Config as cms

def customise(process):

    # From python/simEmtfDigis_cfi.py
    if hasattr(process, 'simEmtfDigis'):
        process.simEmtfDigis.spPCParams16.ZoneBoundaries = [0,36,54,96,127]
        process.simEmtfDigis.spPCParams16.UseNewZones    = True
        process.simEmtfDigis.RPCEnable                   = True
        process.simEmtfDigis.GEMEnable                   = True
        process.simEmtfDigis.Era                         = cms.string('Phase2C2')
        process.simEmtfDigis.spPAParams16.PtLUTVersion   = cms.int32(5)

    # From python/fakeEmtfParams_cff.py
    if hasattr(process, 'emtfParams'):
        process.emtfParams.PtAssignVersion = cms.int32(5)

    if hasattr(process, 'emtfForestsDB'):
        process.emtfForestsDB = cms.ESSource(
            "EmptyESSource",
            recordName = cms.string('L1TMuonEndCapForestRcd'),
            iovIsRunNotTime = cms.bool(True),
            firstValid = cms.vuint32(1)
            )

        process.emtfForests = cms.ESProducer(
            "L1TMuonEndCapForestESProducer",
            PtAssignVersion = cms.int32(5),
            bdtXMLDir = cms.string("v_16_02_21")  # corresponding to pT LUT v5
            )

    return process

