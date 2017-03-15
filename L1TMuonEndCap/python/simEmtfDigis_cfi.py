import FWCore.ParameterSet.Config as cms

from L1Trigger.L1TMuonEndCap.simEmtfDigis_cfi import simEmtfDigisMC, simEmtfDigisData, simEmtfDigis

simEmtfDigisMC = simEmtfDigisMC.clone()
simEmtfDigisData = simEmtfDigisData.clone()
simEmtfDigis = simEmtfDigis.clone()

