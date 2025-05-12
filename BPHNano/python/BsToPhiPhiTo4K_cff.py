import FWCore.ParameterSet.Config as cms
from PhysicsTools.BPHNano.common_cff import *


PhiToKK = cms.EDProducer(
    'DiTrackBuilder',
    tracks = cms.InputTag('tracksBPHSingleMuon', 'SelectedTracks'),
    transientTracks = cms.InputTag('tracksBPHSingleMuon', 'SelectedTransientTracks'),
    trk1Selection = cms.string(''),
    trk2Selection = cms.string(''),
    trk1Mass = cms.double(0.493677),
    trk2Mass = cms.double(0.493677),
    preVtxSelection = cms.string(' && '.join([
            'abs(mass-1.0195)<0.05',
            'userFloat("trk_deltaR")<0.5',
            'charge()==0',
        ])
    ),
    postVtxSelection = cms.string(' && '.join([
            'abs(userFloat("fitted_mass")-1.0195)<0.015',
            'userFloat("fitted_k1_pt")>0.8',
            'userFloat("fitted_k2_pt")>0.7',
            'userFloat("deltaR_postfit")<0.25',
            'userFloat("sv_prob")>0.01',
        ])
    ),
)

BsToPhiPhiTo4K = cms.EDProducer(
    'BTo4TrkBuilder',
    phis = cms.InputTag('PhiToKK'),
    phisTransientTracks = cms.InputTag('tracksBPHSingleMuon', 'SelectedTransientTracks'),
    genParticles = cms.InputTag("finalGenParticlesBPark"),
    beamSpot = cms.InputTag("offlineBeamSpot"),
    pre_vtx_selection = cms.string('abs(mass-5.367)<3'),
    post_vtx_selection = cms.string(' && '.join([
        'abs(userFloat("phi1_fitted_mass") - 1.0195) < 0.012', 
        'abs(userFloat("phi2_fitted_mass") - 1.0195) < 0.012', 
        'userFloat("phi1_fitted_pt") > 2.5',
        'userFloat("phi2_fitted_pt") > 1.8', 
        '(userFloat("phi1_fitted_pt") * userFloat("phi1_fitted_pt")) > 6',
        'userFloat("Bs_fitted_pt") > 1.5',
        'abs(userFloat("Bs_fitted_eta")) < 2.5',
        'userFloat("deltaR_min") < 0.15',
        'userFloat("deltaR_max") < 2.5', 
        'userFloat("Bs_lxy_sig") > 1',
        'userFloat("Bs_sv_prob") > 0.001', 
        'abs(userFloat("Bs_fitted_mass")-5.367)<0.3',
        ])
    ),
)

PhiToKKTable = cms.EDProducer(
    'SimpleCompositeCandidateFlatTableProducer',
    src = cms.InputTag("PhiToKK"),
    cut = cms.string(""),
    name = cms.string("PhiToKK"),
    doc = cms.string("PhiToKK Variable"),
    singleton=cms.bool(False),
    extension=cms.bool(False),
    variables=cms.PSet(
        sv_chi2 = ufloat('sv_chi2'),
        sv_ndof = ufloat('sv_ndof'),
        sv_prob = ufloat('sv_prob'),
        mass = ufloat('fitted_mass'),
        pt = ufloat('fitted_pt'),
        eta = ufloat('fitted_eta'),
        phi = ufloat('fitted_phi'),
        k1_idx = uint('trk1_idx'),
        k2_idx = uint('trk2_idx'),
        k1_pt = ufloat('fitted_k1_pt'),
        k2_pt = ufloat('fitted_k2_pt'),
        deltaR_prefit = ufloat('trk_deltaR'),
        deltaR_postfit = ufloat('deltaR_postfit'),
    )
)

BsToPhiPhiTo4KTable = cms.EDProducer(
    'SimpleCompositeCandidateFlatTableProducer',
    src = cms.InputTag("BsToPhiPhiTo4K"),
    cut = cms.string(""),
    name = cms.string("BsToPhiPhiTo4K"),
    doc = cms.string("BsToPhiPhiTo4K Variable"),
    singleton=cms.bool(False),
    extension=cms.bool(False),
    variables=cms.PSet(
        Bs_vx = ufloat('Bs_vx'),
        Bs_vy = ufloat('Bs_vy'),
        Bs_vz = ufloat('Bs_vz'),
        Bs_cxx = ufloat('Bs_vtx_cxx'),
        Bs_cyy = ufloat('Bs_vtx_cyy'),
        Bs_czz = ufloat('Bs_vtx_czz'),
        Bs_cyx = ufloat('Bs_vtx_cyx'),
        Bs_czx = ufloat('Bs_vtx_czx'),
        Bs_czy = ufloat('Bs_vtx_czy'),
        Bs_sv_chi2 = ufloat('Bs_sv_chi2'),
        Bs_sv_ndof = ufloat('Bs_sv_ndof'),
        Bs_sv_prob = ufloat('Bs_sv_prob'),
        Bs_mass = ufloat('Bs_fitted_mass'),
        Bs_mass_corr = ufloat('Bs_fitted_mass_corr'),
        Bs_massErr = ufloat('Bs_fitted_massErr'),
        Bs_pt = ufloat('Bs_fitted_pt'),
        Bs_eta = ufloat('Bs_fitted_eta'),
        Bs_phi = ufloat('Bs_fitted_phi'),
        Bs_charge = ufloat('Bs_charge'),
        Bs_cos2D = ufloat('Bs_cos_theta_2D'),
        Bs_lxy = ufloat('Bs_lxy'),
        Bs_lxy_sig = ufloat('Bs_lxy_sig'),

        k1_idx = uint('k1_idx'),
        k2_idx = uint('k2_idx'),
        k3_idx = uint('k3_idx'),
        k4_idx = uint('k4_idx'),
        phi1_idx = uint('phi1_idx'),
        phi2_idx = uint('phi2_idx'),

        deltaR_phi1phi2 = ufloat('deltaR_phi1phi2'), 
        deltaR_k1k3 = ufloat('deltaR_k1k3'),
        deltaR_k1k4 = ufloat('deltaR_k1k4'),
        deltaR_k2k3 = ufloat('deltaR_k2k3'),
        deltaR_k2k4 = ufloat('deltaR_k2k4'),
        deltaR_k1k2 = ufloat('deltaR_k1k2'),
        deltaR_k3k4 = ufloat('deltaR_k3k4'),
        deltaR_max = ufloat('deltaR_max'),
        deltaR_min = ufloat('deltaR_min'),

        k1k3_mass = ufloat('k1k3_mass'),
        k1k4_mass = ufloat('k1k4_mass'),
        k2k3_mass = ufloat('k2k3_mass'),
        k2k4_mass = ufloat('k2k4_mass'),
        k1k3_pt = ufloat('k1k3_pt'),
        k1k4_pt = ufloat('k1k4_pt'),
        k2k3_pt = ufloat('k2k3_pt'),
        k2k4_pt = ufloat('k2k4_pt'),

        phi1_mass = ufloat('phi1_fitted_mass'),
        phi1_pt = ufloat('phi1_fitted_pt'),
        phi1_eta = ufloat('phi1_fitted_eta'),
        phi1_phi = ufloat('phi1_fitted_phi'),
        phi2_mass = ufloat('phi2_fitted_mass'),
        phi2_pt = ufloat('phi2_fitted_pt'),
        phi2_eta = ufloat('phi2_fitted_eta'),
        phi2_phi = ufloat('phi2_fitted_phi'),

        k1_mass = ufloat('k1_fitted_mass'),
        k1_pt = ufloat('k1_fitted_pt'),
        k1_eta = ufloat('k1_fitted_eta'),
        k1_phi = ufloat('k1_fitted_phi'),
        k2_mass = ufloat('k2_fitted_mass'),
        k2_pt = ufloat('k2_fitted_pt'),
        k2_eta = ufloat('k2_fitted_eta'),
        k2_phi = ufloat('k2_fitted_phi'),
        k3_mass = ufloat('k3_fitted_mass'),
        k3_pt = ufloat('k3_fitted_pt'),
        k3_eta = ufloat('k3_fitted_eta'),
        k3_phi = ufloat('k3_fitted_phi'),
        k4_mass = ufloat('k4_fitted_mass'),
        k4_pt = ufloat('k4_fitted_pt'),
        k4_eta = ufloat('k4_fitted_eta'),
        k4_phi = ufloat('k4_fitted_phi'),
    )
)

CountPhiToKK = cms.EDFilter("PATCandViewCountFilter",
    minNumber = cms.uint32(1),
    maxNumber = cms.uint32(999999),
    src = cms.InputTag("PhiToKK")
) 

CountBsToPhiPhiTo4K = cms.EDFilter("PATCandViewCountFilter",
    minNumber = cms.uint32(1),
    maxNumber = cms.uint32(999999),
    src = cms.InputTag("BsToPhiPhiTo4K")
) 


PhiToKKSequence = cms.Sequence(PhiToKK)
PhiToKKTables = cms.Sequence(PhiToKKTable)
BsToPhiPhiTo4KSequence = cms.Sequence(BsToPhiPhiTo4K)
BsToPhiPhiTo4KTables = cms.Sequence(BsToPhiPhiTo4KTable)
