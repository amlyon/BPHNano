from __future__ import print_function
import FWCore.ParameterSet.Config as cms
from PhysicsTools.NanoAOD.common_cff import *
from PhysicsTools.NanoAOD.globals_cff import *
from PhysicsTools.NanoAOD.nano_cff import *
from PhysicsTools.NanoAOD.vertices_cff import *
from PhysicsTools.NanoAOD.NanoAODEDMEventContent_cff import *
from PhysicsTools.NanoAOD.triggerObjects_cff import *


##for gen and trigger muon
from PhysicsTools.BPHNano.pverticesBPH_cff import *
from PhysicsTools.BPHNano.genparticlesBPH_cff import *
from PhysicsTools.BPHNano.particlelevelBPH_cff import *

## BPH collections
from PhysicsTools.BPHNano.muons_cff import *
from PhysicsTools.BPHNano.MuMu_cff import *
from PhysicsTools.BPHNano.tracks_cff import *
from PhysicsTools.BPHNano.KstarToKPi_cff import *
from PhysicsTools.BPHNano.KshortToPiPi_cff import *
from PhysicsTools.BPHNano.BToKLL_cff import *
from PhysicsTools.BPHNano.BToKstarLL_cff import *
from PhysicsTools.BPHNano.BToKshortLL_cff import *
from PhysicsTools.BPHNano.BsToPhiPhiTo4K_cff import *


vertexTable.svSrc = cms.InputTag("slimmedSecondaryVertices")



nanoSequence = cms.Sequence(nanoMetadata + 
                            cms.Sequence(vertexTask) +
                            cms.Sequence(globalTablesTask)+ 
                            cms.Sequence(vertexTablesTask) +
                            cms.Sequence(pVertexTable) 
                          )



def nanoAOD_customizeMC(process):
    process.nanoSequence = cms.Sequence(process.nanoSequence +particleLevelBPHSequence + genParticleBPHSequence+ genParticleBPHTables )
    return process


def nanoAOD_customizeSingleMuonBPH(process, isMC):
    if isMC:
       process.nanoSequence = cms.Sequence( process.nanoSequence + muonBPHSequenceMC + muonBPHTablesMC )
    else:
       process.nanoSequence = cms.Sequence( process.nanoSequence + muonBPHSequence + countTrgSingleMuons + muonBPHTables )
    return process



def nanoAOD_customizeDiMuonBPH(process, isMC):
    if isMC:
       process.nanoSequence = cms.Sequence( process.nanoSequence + muonBPHSequenceMC + muonBPHTablesMC + MuMuSequence + MuMuTables )
    else:
       process.nanoSequence = cms.Sequence( process.nanoSequence + muonBPHSequence + countTrgDiMuons + muonBPHTables + MuMuSequence + CountDiMuonBPH + MuMuTables )
    return process



def nanoAOD_customizeTrackBPHSingleMuon(process,isMC):
    if isMC:
       process.nanoSequence =  cms.Sequence( process.nanoSequence + tracksBPHSequenceMCSingleMuon + tracksBPHSingleMuonTablesMC )
    else:
       process.nanoSequence = cms.Sequence( process.nanoSequence + tracksBPHSequenceSingleMuon + tracksBPHSingleMuonTables )
    return process


def nanoAOD_customizeTrackBPHDiMuon(process,isMC):
    if isMC:
       process.nanoSequence =  cms.Sequence( process.nanoSequence + tracksBPHSequenceMCDiMuon + tracksBPHDiMuonTablesMC )
    else:
       process.nanoSequence = cms.Sequence( process.nanoSequence + tracksBPHSequenceDiMuon + tracksBPHDiMuonTables )
    return process


def nanoAOD_customizeBToKLL(process,isMC):
    if isMC:
       process.nanoSequence = cms.Sequence( process.nanoSequence + BToKMuMuSequence + BToKMuMuTables  )
    else:
       process.nanoSequence = cms.Sequence( process.nanoSequence + BToKMuMuSequence +CountBToKmumu + BToKMuMuTables)
    return process



def nanoAOD_customizeBToKstarLL(process,isMC):
    if isMC:
       process.nanoSequence = cms.Sequence( process.nanoSequence + KstarPiKSequence + KstarPiKTables + BToKstarMuMuSequence + BToKstarMuMuTables  )
    else:
       process.nanoSequence = cms.Sequence( process.nanoSequence + KstarPiKSequence + CountKstarPiK+ KstarPiKTables+ BToKstarMuMuSequence + BToKstarMuMuTables  )
    return process




def nanoAOD_customizeBToKshortLL(process, isMC):
    if isMC:
       process.nanoSequence = cms.Sequence( process.nanoSequence+ KshortToPiPiSequenceMC + KshortToPiPiTablesMC + BToKshortMuMuSequence + BToKshortMuMuTables  )
    else:
       process.nanoSequence = cms.Sequence( process.nanoSequence+ KshortToPiPiSequence + CountKshortToPiPi+ KshortToPiPiTables + BToKshortMuMuSequence + CountBToKshortMuMu +BToKshortMuMuTables  )
    return process




def nanoAOD_customizeBToXLL(process,isMC):
    if isMC:
       process.nanoSequence = cms.Sequence( process.nanoSequence + BToKMuMuSequence + BToKMuMuTables + KshortToPiPiSequenceMC + KshortToPiPiTablesMC + BToKshortMuMuSequence + BToKshortMuMuTables +  KstarPiKTables +KstarPiKTables+ BToKstarMuMuSequence + BToKstarMuMuTables  )
    else:
       process.nanoSequence = cms.Sequence( process.nanoSequence + BToKMuMuSequence + BToKMuMuTables + KshortToPiPiSequence + KshortToPiPiTables + BToKshortMuMuSequence +BToKshortMuMuTables + KstarPiKSequence +  KstarPiKTables +KstarPiKTables+ BToKstarMuMuSequence + BToKstarMuMuTables )
    return process



def nanoAOD_customizeBsToPhiPhiTo4K(process, isMC):
    if isMC:
       process.nanoSequence = cms.Sequence( process.nanoSequence + PhiToKKSequence + BsToPhiPhiTo4KSequence + PhiToKKTables + BsToPhiPhiTo4KTable ) #FIXME is it correct?
    else:
       process.nanoSequence = cms.Sequence( process.nanoSequence + PhiToKKSequence + BsToPhiPhiTo4KSequence + PhiToKKTables + BsToPhiPhiTo4KTable + CountPhiToKK + CountBsToPhiPhiTo4K )
    return process



