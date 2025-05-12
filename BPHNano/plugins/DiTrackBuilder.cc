/////////////////////////////// DiTrackBuilder ///////////////////////////////
/// original authors: G Karathanasis (CERN),  G Melachroinos (NKUA)
// takes selected track collection and a mass hypothesis and produces ditrack ca
// -ndidates



#include "FWCore/Framework/interface/global/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"

#include <vector>
#include <memory>
#include <map>
#include <string>
#include "CommonTools/Utils/interface/StringCutObjectSelector.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "CommonTools/Statistics/interface/ChiSquaredProbability.h"
#include "TrackingTools/TransientTrack/interface/TransientTrack.h"
#include "helper.h"
#include <limits>
#include <algorithm>
#include "KinVtxFitter.h"



class DiTrackBuilder : public edm::global::EDProducer<> {


public:

  typedef std::vector<reco::TransientTrack> TransientTrackCollection;

  explicit DiTrackBuilder(const edm::ParameterSet &cfg):
    trk1_selection_{cfg.getParameter<std::string>("trk1Selection")},
    trk2_selection_{cfg.getParameter<std::string>("trk2Selection")},
    pre_vtx_selection_{cfg.getParameter<std::string>("preVtxSelection")},
    post_vtx_selection_{cfg.getParameter<std::string>("postVtxSelection")},
    pfcands_{consumes<pat::CompositeCandidateCollection>( cfg.getParameter<edm::InputTag>("tracks") )},
    ttracks_{consumes<TransientTrackCollection>( cfg.getParameter<edm::InputTag>("transientTracks") )},
    trk1_mass_{ cfg.getParameter<double>("trk1Mass")},
    trk2_mass_{ cfg.getParameter<double>("trk2Mass")}
  {

    //output
    produces<pat::CompositeCandidateCollection>();

  }

  ~DiTrackBuilder() override {}

  void produce(edm::StreamID, edm::Event&, const edm::EventSetup&) const override;

  static void fillDescriptions(edm::ConfigurationDescriptions &descriptions) {}

private:
  const StringCutObjectSelector<pat::CompositeCandidate> trk1_selection_; // cuts on leading cand
  const StringCutObjectSelector<pat::CompositeCandidate> trk2_selection_; // sub-leading cand
  const StringCutObjectSelector<pat::CompositeCandidate> pre_vtx_selection_; // cut on the di-lepton before the SV fit
  const StringCutObjectSelector<pat::CompositeCandidate> post_vtx_selection_; // cut on the di-lepton after the SV fit
  const edm::EDGetTokenT<pat::CompositeCandidateCollection> pfcands_; //input PF cands this is sorted in pT in previous step
  const edm::EDGetTokenT<TransientTrackCollection> ttracks_; //input TTracks of PF cands
  double trk1_mass_;
  double trk2_mass_;
};


void DiTrackBuilder::produce(edm::StreamID, edm::Event &evt, edm::EventSetup const &) const {

  //inputs
  edm::Handle<pat::CompositeCandidateCollection> pfcands;
  evt.getByToken(pfcands_, pfcands);
  edm::Handle<TransientTrackCollection> ttracks;
  evt.getByToken(ttracks_, ttracks);


  // output
  std::unique_ptr<pat::CompositeCandidateCollection> cand_out(new pat::CompositeCandidateCollection());

  // needed to sort in pt
  std::vector<pat::CompositeCandidate> vector_candidates;

  // main loop
  for (size_t trk1_idx = 0; trk1_idx < pfcands->size(); ++trk1_idx ) {

    edm::Ptr<pat::CompositeCandidate> trk1_ptr( pfcands, trk1_idx );
    if (!trk1_selection_(*trk1_ptr)) continue;

    for (size_t trk2_idx = trk1_idx + 1; trk2_idx < pfcands->size(); ++trk2_idx) {

      edm::Ptr<pat::CompositeCandidate> trk2_ptr( pfcands, trk2_idx );
      //if (trk1_ptr->charge() == trk2_ptr->charge()) continue;
      if (!trk2_selection_(*trk2_ptr)) continue;

      bool UsedAgain = false;

      std::vector< std::pair<double, double> > list_masses;
      if (trk1_mass_ == trk2_mass_) {
        list_masses.push_back(std::pair<double, double>(trk1_mass_, trk2_mass_));
      }
      else{
        list_masses.push_back(std::pair<double, double>(trk1_mass_, trk2_mass_));
        list_masses.push_back(std::pair<double, double>(trk2_mass_, trk1_mass_));
      }

      // Loop in all possible hypothesis
      for ( std::pair<double, double> masses : list_masses ) {
        // create a candidate; add first quantities that can be used for pre fit selection
        pat::CompositeCandidate cand;

        auto trk1_p4 = trk1_ptr->polarP4();
        auto trk2_p4 = trk2_ptr->polarP4();
        trk1_p4.SetM(masses.first);
        trk2_p4.SetM(masses.second);

        //adding stuff for pre fit selection
        cand.setP4(trk1_p4 + trk2_p4);
        cand.setCharge(trk1_ptr->charge() + trk2_ptr->charge());
        cand.addUserFloat("trk_deltaR", reco::deltaR(*trk1_ptr, *trk2_ptr));

        // save indices
        cand.addUserInt("trk1_idx", trk1_idx );
        cand.addUserInt("trk2_idx", trk2_idx );
        cand.addUserFloat("trk1_mass", masses.first);
        cand.addUserFloat("trk2_mass", masses.second);

        // save cands
        cand.addUserCand("trk1", trk1_ptr );
        cand.addUserCand("trk2", trk2_ptr );

        // selection before fit
        if ( !pre_vtx_selection_(cand) ) continue;
        //std::cout<<"trk1 "<<trk1_idx<<" trk2 "<<trk2_idx<<" dr "<< reco::deltaR(*trk1_ptr, *trk2_ptr)<<" pt1 "<<trk1_p4.pt()<<" pt2 "<<trk2_p4.pt()<<" mass "<<(trk1_p4+trk2_p4).mass()<<std::endl;

        KinVtxFitter fitter(
            {ttracks->at(trk1_idx), ttracks->at(trk2_idx)},
            { masses.first, masses.second },
            {K_SIGMA, K_SIGMA} //K and PI sigma equal...
                           );

        if ( !fitter.success() ) continue;
        cand.setVertex(
          reco::Candidate::Point(
            fitter.fitted_vtx().x(),
            fitter.fitted_vtx().y(),
            fitter.fitted_vtx().z()
          )
        );
        // save quantities after fit
        cand.addUserInt("sv_ok", fitter.success() ? 1 : 0);
        cand.addUserFloat("sv_chi2", fitter.chi2());
        cand.addUserFloat("sv_ndof", fitter.dof());
        cand.addUserFloat("sv_prob", fitter.prob());
        cand.addUserFloat("fitted_mass", fitter.fitted_candidate().mass() );
        cand.addUserFloat("fitted_pt",
                                fitter.fitted_candidate().globalMomentum().perp() );

        cand.addUserFloat("fitted_eta",
                                fitter.fitted_candidate().globalMomentum().eta() );

        cand.addUserFloat("fitted_phi",
                                fitter.fitted_candidate().globalMomentum().phi() );

        cand.addUserInt("second_mass_hypothesis", UsedAgain );
        cand.addUserFloat("vtx_x", cand.vx());
        cand.addUserFloat("vtx_y", cand.vy());
        cand.addUserFloat("vtx_z", cand.vz());
        cand.addUserFloat("deltaR_postfit", reco::deltaR(fitter.daughter_p4(0), fitter.daughter_p4(1)));
        cand.addUserFloat("fitted_k1_pt", fitter.daughter_p4(0).pt()); 
        cand.addUserFloat("fitted_k2_pt", fitter.daughter_p4(1).pt()); 

        // after fit selection
        if ( !post_vtx_selection_(cand) ) continue;
        vector_candidates.emplace_back(cand);   
        UsedAgain = true;

      } // end for ( auto & masses:
    } // end for(size_t trk2_idx = trk1_idx + 1
  } //for(size_t trk1_idx = 0

  // sort candidate collection in pt
  std::sort(vector_candidates.begin(), vector_candidates.end(), 
             [] (auto & cand1, auto & cand2) -> 
                  bool {return (cand1.pt() > cand2.pt());} 
           );

  for (auto & cand: vector_candidates){
    cand_out->emplace_back(cand);
  }
  evt.put(std::move(cand_out));
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(DiTrackBuilder);
