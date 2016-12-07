import FWCore.ParameterSet.Config as cms

from L1TriggerSep2016.L1TMuonEndCap.simEmtfDigisSep2016_cfi import simEmtfDigisSep2016MC, simEmtfDigisSep2016Data, simEmtfDigisSep2016

simEmtfDigisMC = simEmtfDigisSep2016MC.clone()
simEmtfDigisData = simEmtfDigisSep2016Data.clone()
simEmtfDigis = simEmtfDigisSep2016.clone()

