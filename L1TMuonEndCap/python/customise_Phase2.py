import FWCore.ParameterSet.Config as cms

def customise(process):

    ## EMTF
    ## - see python/simEmtfDigis_cfi.py
    if hasattr(process, 'simEmtfDigis'):
        #process.simEmtfDigis.spPCParams16.ZoneBoundaries = [0,36,54,96,127]
        #process.simEmtfDigis.spPCParams16.UseNewZones    = True
        process.simEmtfDigis.DTEnable                    = True
        process.simEmtfDigis.CSCEnable                   = True
        process.simEmtfDigis.RPCEnable                   = True
        process.simEmtfDigis.GEMEnable                   = True
        process.simEmtfDigis.IRPCEnable                  = True
        process.simEmtfDigis.ME0Enable                   = True
        process.simEmtfDigis.Era                         = cms.string('Phase2_timing')
        process.simEmtfDigis.spPAParams16.PtLUTVersion   = cms.int32(7)

    ## CSCTriggerPrimitives
    ## - see L1Trigger/CSCTriggerPrimitives/python/cscTriggerPrimitiveDigis_cfi.py
    if hasattr(process, 'simCscTriggerPrimitiveDigis'):
        process.simCscTriggerPrimitiveDigis.commonParam.runME11ILT = cms.bool(False)
        process.simCscTriggerPrimitiveDigis.commonParam.runME21ILT = cms.bool(False)
    else:
        process.load('L1Trigger.CSCTriggerPrimitives.cscTriggerPrimitiveDigis_cfi')
        process.simCscTriggerPrimitiveDigis.commonParam.runME11ILT = cms.bool(False)
        process.simCscTriggerPrimitiveDigis.commonParam.runME21ILT = cms.bool(False)

    ## RPCRecHit
    if hasattr(process, 'rpcRecHits'):
        process.rpcRecHits.rpcDigiLabel = 'simMuonRPCDigis'
    else:
        process.load('RecoLocalMuon.RPCRecHit.rpcRecHits_cfi')
        process.rpcRecHits.rpcDigiLabel = 'simMuonRPCDigis'

    ## ME0TriggerDigi added in 10_5_X
    if hasattr(process, 'me0TriggerPseudoDigis105X'):
        pass
    else:
        process.load('L1Trigger.L1TMuonEndCap.me0TriggerPseudoDigis105X_cff')

    ## Backward compatibility with 10_4_0
    process.muonGEMDigiTask = cms.Task(process.simMuonGEMDigis, process.simMuonGEMPadDigis, process.simMuonGEMPadDigiClusters)
    process.muonME0RealDigiTask = cms.Task(process.simMuonME0Digis, process.simMuonME0PadDigis, process.simMuonME0PadDigiClusters)
    process.muonME0PseudoDigiTask = cms.Task(process.simMuonME0PseudoDigis, process.simMuonME0PseudoReDigis)
    process.muonME0DigiTask = cms.Task(process.muonME0RealDigiTask, process.muonME0PseudoDigiTask)
    process.muonME0DigiTask.add(process.me0TriggerPseudoDigiTask105X)
    process.muonDigiTask = cms.Task(process.simMuonCSCDigis, process.simMuonDTDigis, process.simMuonRPCDigis, process.muonGEMDigiTask, process.muonME0DigiTask)
    process.muonDigi = cms.Sequence(process.muonDigiTask)
    #process.SimL1TMuonCommonTask = cms.Task(process.simDtTriggerPrimitiveDigis, process.simCscTriggerPrimitiveDigis)
    process.SimL1TMuonCommonTask = cms.Task(process.simDtTriggerPrimitiveDigis, process.simCscTriggerPrimitiveDigis, process.rpcRecHits)
    process.SimL1TMuonCommon = cms.Sequence(process.SimL1TMuonCommonTask)
    return process
