import FWCore.ParameterSet.Config as cms

def customise(process):

    # From python/simEmtfDigis_cfi.py
    if hasattr(process, 'simEmtfDigis'):
        process.simEmtfDigis.RPCEnable = cms.bool(False)
        process.simEmtfDigis.Era = cms.string('Run2_2016')
        process.simEmtfDigis.spTBParams16.ThetaWindow = cms.int32(4)
        process.simEmtfDigis.spPCParams16.FixME11Edges = cms.bool(False)
        process.simEmtfDigis.spPAParams16.PtLUTVersion = cms.int32(5)
        process.simEmtfDigis.spPAParams16.BugGMTPhi = cms.bool(True)

    # From python/fakeEmtfParams_cff.py
    if hasattr(process, 'emtfParams'):
        process.emtfParams.PtAssignVersion = cms.int32(5)
        process.emtfParams.FirmwareVersion = cms.int32(49999) ## Settings as of end-of-year 2016
        process.emtfParams.PrimConvVersion = cms.int32(0)

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

