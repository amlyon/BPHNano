#include "FWCore/Framework/interface/global/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/Utilities/interface/InputTag.h"

#include <vector>
#include <memory>
#include <map>
#include <string>
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"
#include "CommonTools/Utils/interface/StringCutObjectSelector.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "CommonTools/Statistics/interface/ChiSquaredProbability.h"
#include "TrackingTools/TransientTrack/interface/TransientTrack.h"
#include "helper.h"
#include <limits>
#include <algorithm>
#include "KinVtxFitter.h"
#include "RecoVertex/VertexTools/interface/VertexDistance3D.h"

/*
 * This class was originally designed to reconstruct the Bs -> phi(KK)phi/KK) decay.
 * It can be generalised to other fully-hadronic B decays.
 */

class BTo4TrkBuilder : public edm::global::EDProducer<> {

public:
  typedef std::vector<reco::TransientTrack> TransientTrackCollection;

  explicit BTo4TrkBuilder(const edm::ParameterSet &cfg):
    phi_candidates_{consumes<pat::CompositeCandidateCollection>( cfg.getParameter<edm::InputTag>("phis") )},
    kaon_ttracks_{consumes<TransientTrackCollection>( cfg.getParameter<edm::InputTag>("phisTransientTracks") )},
    pre_vtx_selection_ {cfg.getParameter<std::string>("pre_vtx_selection")},
    post_vtx_selection_ {cfg.getParameter<std::string>("post_vtx_selection")},
    beamspot_{consumes<reco::BeamSpot>(cfg.getParameter<edm::InputTag>("beamSpot"))}{ 
       produces<pat::CompositeCandidateCollection>();
    }

  ~BTo4TrkBuilder() override {}
  
  void produce(edm::StreamID, edm::Event&, const edm::EventSetup&) const override;

  static void fillDescriptions(edm::ConfigurationDescriptions &descriptions) {}
  
private:
  const edm::EDGetTokenT<pat::CompositeCandidateCollection> phi_candidates_;
  const edm::EDGetTokenT<TransientTrackCollection> kaon_ttracks_;
  const StringCutObjectSelector<pat::CompositeCandidate> pre_vtx_selection_;
  const StringCutObjectSelector<pat::CompositeCandidate> post_vtx_selection_; 
  const edm::EDGetTokenT<reco::BeamSpot> beamspot_;  
};

void BTo4TrkBuilder::produce(edm::StreamID, edm::Event &evt, edm::EventSetup const &) const {

  edm::Handle<pat::CompositeCandidateCollection> phi_candidates;
  evt.getByToken(phi_candidates_, phi_candidates);
  
  edm::Handle<TransientTrackCollection> kaon_ttracks;
  evt.getByToken(kaon_ttracks_, kaon_ttracks);

  edm::Handle<reco::BeamSpot> beamspot;
  evt.getByToken(beamspot_, beamspot);  

  // output
  std::unique_ptr<pat::CompositeCandidateCollection> ret_value(new pat::CompositeCandidateCollection());

  // loop on first phi candidate (phi1)
  for(size_t phi1_idx = 0; phi1_idx < phi_candidates->size(); ++phi1_idx) {
    edm::Ptr<pat::CompositeCandidate> phi1_ptr(phi_candidates, phi1_idx);

    // retrieve kaons
    edm::Ptr<reco::Candidate> k1_ptr = phi1_ptr->userCand("trk1");
    edm::Ptr<reco::Candidate> k2_ptr = phi1_ptr->userCand("trk2");
    int k1_idx = phi1_ptr->userInt("trk1_idx");
    int k2_idx = phi1_ptr->userInt("trk2_idx");

    // loop on second phi candidate (phi2)
    for(size_t phi2_idx = phi1_idx + 1; phi2_idx < phi_candidates->size(); ++phi2_idx) {
      edm::Ptr<pat::CompositeCandidate> phi2_ptr(phi_candidates, phi2_idx);

      // retrieve kaons
      edm::Ptr<reco::Candidate> k3_ptr = phi2_ptr->userCand("trk1");
      edm::Ptr<reco::Candidate> k4_ptr = phi2_ptr->userCand("trk2");
      int k3_idx = phi2_ptr->userInt("trk1_idx");
      int k4_idx = phi2_ptr->userInt("trk2_idx");

      if(phi2_ptr->pt() > phi1_ptr->pt()){
        std::cout << "not ordered in pt!" << std::endl;
      }

      if(k3_idx == k1_idx || k3_idx == k2_idx) {
        continue;
      }
      if(k4_idx == k1_idx || k4_idx == k2_idx){
        continue;
      }

      // build Bs candidate
      pat::CompositeCandidate Bs_cand;
      Bs_cand.setP4(phi1_ptr->p4() + phi2_ptr->p4());
      Bs_cand.setCharge(phi1_ptr->charge() + phi2_ptr->charge());
      
      // apply pre-fit selection on Bs candidate
      if( !pre_vtx_selection_(Bs_cand) ) continue;

      // save indices
      Bs_cand.addUserInt("k1_idx", k1_idx);
      Bs_cand.addUserInt("k2_idx", k2_idx);
      Bs_cand.addUserInt("k3_idx", k3_idx);
      Bs_cand.addUserInt("k4_idx", k4_idx);
      Bs_cand.addUserInt("phi1_idx", phi1_idx);
      Bs_cand.addUserInt("phi2_idx", phi2_idx);

      // save candidates
      Bs_cand.addUserCand("k1", k1_ptr);
      Bs_cand.addUserCand("k2", k2_ptr);
      Bs_cand.addUserCand("k3", k3_ptr);
      Bs_cand.addUserCand("k4", k4_ptr);
      Bs_cand.addUserCand("phi1", phi1_ptr);
      Bs_cand.addUserCand("phi2", phi2_ptr);

      // fit the four kaon tracks to a common vertex
      KinVtxFitter fitter_Bs(
        {kaon_ttracks->at(k1_idx), kaon_ttracks->at(k2_idx), kaon_ttracks->at(k3_idx), kaon_ttracks->at(k4_idx)},
        {K_MASS, K_MASS, K_MASS, K_MASS}, // force kaon mass
        {K_SIGMA, K_SIGMA, K_SIGMA, K_SIGMA} 
        );

      if(!fitter_Bs.success()) continue;

      Bs_cand.setVertex( 
        reco::Candidate::Point( 
          fitter_Bs.fitted_vtx().x(),
          fitter_Bs.fitted_vtx().y(),
          fitter_Bs.fitted_vtx().z()
          )  
        );

      Bs_cand.addUserFloat("Bs_vx", fitter_Bs.fitted_vtx().x());
      Bs_cand.addUserFloat("Bs_vy", fitter_Bs.fitted_vtx().y());
      Bs_cand.addUserFloat("Bs_vz", fitter_Bs.fitted_vtx().z());

      const auto& covMatrix = fitter_Bs.fitted_vtx_uncertainty();
      Bs_cand.addUserFloat("Bs_vtx_cxx", covMatrix.cxx());
      Bs_cand.addUserFloat("Bs_vtx_cyy", covMatrix.cyy());
      Bs_cand.addUserFloat("Bs_vtx_czz", covMatrix.czz());
      Bs_cand.addUserFloat("Bs_vtx_cyx", covMatrix.cyx());
      Bs_cand.addUserFloat("Bs_vtx_czx", covMatrix.czx());
      Bs_cand.addUserFloat("Bs_vtx_czy", covMatrix.czy());

      Bs_cand.addUserFloat("Bs_sv_chi2", fitter_Bs.chi2());
      Bs_cand.addUserFloat("Bs_sv_ndof", fitter_Bs.dof()); 
      Bs_cand.addUserFloat("Bs_sv_prob", fitter_Bs.prob());

      auto fit_p4 = fitter_Bs.fitted_p4();
      Bs_cand.addUserFloat("Bs_fitted_pt"  , fit_p4.pt()); 
      Bs_cand.addUserFloat("Bs_fitted_eta" , fit_p4.eta());
      Bs_cand.addUserFloat("Bs_fitted_phi" , fit_p4.phi());
      Bs_cand.addUserFloat("Bs_fitted_mass", fit_p4.mass());      
      Bs_cand.addUserFloat("Bs_fitted_mass_corr", fit_p4.mass() - (fitter_Bs.daughter_p4(0) + fitter_Bs.daughter_p4(1)).mass() + 1.0195 - (fitter_Bs.daughter_p4(2) + fitter_Bs.daughter_p4(3)).mass() + 1.0195); 
      Bs_cand.addUserFloat("Bs_fitted_massErr", sqrt(fitter_Bs.fitted_candidate().kinematicParametersError().matrix()(6,6))); 
      Bs_cand.addUserFloat("Bs_charge", Bs_cand.charge());
      Bs_cand.addUserFloat("Bs_cos_theta_2D", cos_theta_2D(fitter_Bs, *beamspot, fit_p4));
      auto Bs_lxy = l_xy(fitter_Bs, *beamspot);
      Bs_cand.addUserFloat("Bs_lxy", Bs_lxy.value());
      Bs_cand.addUserFloat("Bs_lxy_sig", Bs_lxy.value() / Bs_lxy.error());

      auto k1_p4 = fitter_Bs.daughter_p4(0);
      auto k2_p4 = fitter_Bs.daughter_p4(1);
      auto k3_p4 = fitter_Bs.daughter_p4(2);
      auto k4_p4 = fitter_Bs.daughter_p4(3);
      auto phi1_p4 = k1_p4 + k2_p4;
      auto phi2_p4 = k3_p4 + k4_p4;

      // phi candidates
      std::vector<std::string> phi_names{"phi1", "phi2"};
      for (size_t i = 0; i < phi_names.size(); i++){
        int idx1 = -99;
        int idx2 = -99;
        if(phi_names[i] == "phi1"){
          idx1 = 0;
          idx2 = 1;
        }
        else if(phi_names[i] == "phi2"){
          idx1 = 2;
          idx2 = 3;
        }
        Bs_cand.addUserFloat(phi_names[i] + "_fitted_mass", (fitter_Bs.daughter_p4(idx1) + fitter_Bs.daughter_p4(idx2)).mass());
        Bs_cand.addUserFloat(phi_names[i] + "_fitted_pt", (fitter_Bs.daughter_p4(idx1) + fitter_Bs.daughter_p4(idx2)).pt());
        Bs_cand.addUserFloat(phi_names[i] + "_fitted_eta", (fitter_Bs.daughter_p4(idx1) + fitter_Bs.daughter_p4(idx2)).eta());
        Bs_cand.addUserFloat(phi_names[i] + "_fitted_phi", (fitter_Bs.daughter_p4(idx1) + fitter_Bs.daughter_p4(idx2)).phi());
      }

      // compute deltaR
      Bs_cand.addUserFloat("deltaR_phi1phi2", reco::deltaR(phi1_p4, phi2_p4));
      Bs_cand.addUserFloat("deltaR_k1k3", reco::deltaR(k1_p4, k3_p4));
      Bs_cand.addUserFloat("deltaR_k1k4", reco::deltaR(k1_p4, k4_p4));
      Bs_cand.addUserFloat("deltaR_k2k3", reco::deltaR(k2_p4, k3_p4));
      Bs_cand.addUserFloat("deltaR_k2k4", reco::deltaR(k2_p4, k4_p4));

      Bs_cand.addUserFloat("deltaR_k1k2", reco::deltaR(k1_p4, k2_p4));
      Bs_cand.addUserFloat("deltaR_k3k4", reco::deltaR(k3_p4, k4_p4));

      auto dr_info = min_max_dr({k1_ptr, k2_ptr, k3_ptr, k4_ptr});
      Bs_cand.addUserFloat("deltaR_min", dr_info.first);
      Bs_cand.addUserFloat("deltaR_max", dr_info.second);

      // compute invariant masses
      Bs_cand.addUserFloat("k1k3_mass", (k1_p4 + k3_p4).mass());
      Bs_cand.addUserFloat("k1k4_mass", (k1_p4 + k4_p4).mass());
      Bs_cand.addUserFloat("k2k3_mass", (k2_p4 + k3_p4).mass());
      Bs_cand.addUserFloat("k2k4_mass", (k2_p4 + k4_p4).mass());

      Bs_cand.addUserFloat("k1k3_pt", (k1_p4 + k3_p4).pt());
      Bs_cand.addUserFloat("k1k4_pt", (k1_p4 + k4_p4).pt());
      Bs_cand.addUserFloat("k2k3_pt", (k2_p4 + k3_p4).pt());
      Bs_cand.addUserFloat("k2k4_pt", (k2_p4 + k4_p4).pt());

      // kaons
      std::vector<std::string> kaon_names{"k1", "k2", "k3", "k4"};
      
      for (size_t i = 0; i < kaon_names.size(); i++){
	      Bs_cand.addUserFloat(kaon_names[i] + "_fitted_mass" ,fitter_Bs.daughter_p4(i).mass());
	      Bs_cand.addUserFloat(kaon_names[i] + "_fitted_pt" ,fitter_Bs.daughter_p4(i).pt());
        Bs_cand.addUserFloat(kaon_names[i] + "_fitted_eta",fitter_Bs.daughter_p4(i).eta());
        Bs_cand.addUserFloat(kaon_names[i] + "_fitted_phi",fitter_Bs.daughter_p4(i).phi());
      }
      
      // apply pots-fit selection on phi1 candidate
      if(!post_vtx_selection_(Bs_cand)) continue;

      // save candidate
      ret_value->push_back(Bs_cand);
    }
  }
  
  evt.put(std::move(ret_value));
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(BTo4TrkBuilder);
