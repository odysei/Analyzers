#ifndef CU_ttH_EDA_h
#define CU_ttH_EDA_h


/// Core libraries
#include <memory>
#include <cstdio>	// printf, fprintf


/// CMSSW user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"

#include "DataFormats/BeamSpot/interface/BeamSpot.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/PatCandidates/interface/Photon.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/GenericParticle.h"
#include "DataFormats/PatCandidates/interface/MET.h"
#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/PatCandidates/interface/Lepton.h"
#include "DataFormats/PatCandidates/interface/Isolation.h"
#include "DataFormats/PatCandidates/interface/Tau.h"
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidateFwd.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidate.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "DataFormats/L1Trigger/interface/L1EmParticle.h"
#include "DataFormats/L1Trigger/interface/L1JetParticle.h"
#include "DataFormats/L1Trigger/interface/L1MuonParticle.h"
#include "DataFormats/L1Trigger/interface/L1EtMissParticle.h"
#include "DataFormats/L1Trigger/interface/L1ParticleMap.h"
#include "DataFormats/L1Trigger/interface/L1ParticleMapFwd.h"
#include "DataFormats/L1Trigger/interface/L1HFRings.h"
#include "DataFormats/L1Trigger/interface/L1HFRingsFwd.h"
#include "DataFormats/HLTReco/interface/TriggerObject.h"
#include "DataFormats/HLTReco/interface/TriggerEvent.h"
#include "DataFormats/HLTReco/interface/TriggerEventWithRefs.h"
#include "DataFormats/HLTReco/interface/TriggerTypeDefs.h"
#include "DataFormats/Candidate/interface/VertexCompositePtrCandidate.h"
#include "DataFormats/Candidate/interface/VertexCompositePtrCandidateFwd.h"

#include "PhysicsTools/SelectorUtils/interface/JetIDSelectionFunctor.h"
#include "PhysicsTools/SelectorUtils/interface/strbitset.h"

#include "SimDataFormats/PileupSummaryInfo/interface/PileupSummaryInfo.h"
#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"

#include "HLTrigger/HLTcore/interface/HLTConfigProvider.h"

#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include "JetMETCorrections/Objects/interface/JetCorrector.h"

#include "MiniAOD/MiniAODHelper/interface/MiniAODHelper.h"


/// ROOT includes
#include "TH1.h"


/// Higgs and top tagger
#include "BoostedTTH/BoostedObjects/interface/HEPTopJet.h"
#include "BoostedTTH/BoostedObjects/interface/SubFilterJet.h"
#include "BoostedTTH/BoostedAnalyzer/interface/BoostedUtils.hpp"


/// structs for holding multiple edm::Handle and EDGetTokenT
#include "CU_ttH_EDA_Handles.h"



/*
 * 
 * struct for per-event variables used in analyze(...)
 * 
 */

struct CU_ttH_EDA_event_vars {
	double weight;	// total event weight (there are partial weights)
	
	/// Common, run parameters
	int run_nr;
	int event_nr;
	int lumisection_nr;
	
	
	/// Number of tags per event
	int n_electrons;
	int n_muons;
	int n_jets;
	int n_btags;
	int n_ttags;
	int n_Htags;
	
	
	/// Passing-trigger flags
	bool pass_single_e;
	bool pass_single_mu;
	
	
	/// Particle container vectors
	std::vector<pat::Electron> e_selected;
	std::vector<pat::Electron> e_selected_sorted;
	std::vector<pat::Muon> mu_selected;
	std::vector<pat::Muon> mu_selected_sorted;
	
	std::vector<pat::Jet> jets_raw;
	std::vector<pat::Jet> jets_no_mu;
	std::vector<pat::Jet> jets_no_mu_e;
	std::vector<pat::Jet> jets_corrected;
	std::vector<pat::Jet> jets_selected;
	std::vector<pat::Jet> jets_selected_sorted;
	std::vector<pat::Jet> jets_selected_tag;
	std::vector<pat::Jet> jets_selected_tag_sorted;
};



/*
 * 
 * Analyzer class
 * 
 */

class CU_ttH_EDA : public edm::EDAnalyzer
{
public:
	explicit CU_ttH_EDA(const edm::ParameterSet&);
	~CU_ttH_EDA();
	
	static void fillDescriptions(edm::ConfigurationDescriptions&);
	
	
private:
	/*
	 * Function/method section
	*/
	
	/// Standard EDAnalyzer functions
	void analyze(const edm::Event&, const edm::EventSetup&) override;
	
	void beginJob() override;
	void endJob() override;
	
	void beginRun(edm::Run const&, edm::EventSetup const&) override;
	void endRun(edm::Run const&, edm::EventSetup const&) override;
	
	//virtual void beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;
	//virtual void endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;
	
	
	/// One-time-run functions
	void Close_output_files();	// at ~CU_ttH_EDA()
	void Set_up_histograms();	// at CU_ttH_EDA()
	void Set_up_output_files();	// at CU_ttH_EDA()
	void Set_up_tokens();	// at CU_ttH_EDA()
	
	int Set_up_Run_histograms();	// at beginRun(), after Set_up_name_vectors()
	
	void Set_up_name_vectors();	// at beginRun()
	
	int End_Run_hist_fill();	// fill histograms at endRun()
	
	
	/// Per-event functions
	void Update_common_vars(const edm::Event&, CU_ttH_EDA_event_vars&);
	
	
	/// Object checks. Returns 1 in case of an error
	int Check_beam_spot(edm::Handle<reco::BeamSpot>);
	int Check_triggers(edm::Handle<edm::TriggerResults>,
		CU_ttH_EDA_event_vars&);	// adjusts event variables
	int Check_filters(edm::Handle<edm::TriggerResults>);
	int Check_vertices_set_MAODhelper(edm::Handle<reco::VertexCollection>);
	
	
	/// Taggers. Returns 1 in case of an error
	int Higgs_tagger(Handle<boosted::SubFilterJetCollection>,
		CU_ttH_EDA_event_vars&);	// FIXME: uses b-tag medium WP
	int Top_tagger(Handle<boosted::HEPTopJetCollection>,
		CU_ttH_EDA_event_vars&);
	
	
	/// Other functions
	void Check_Fill_Print_ej(CU_ttH_EDA_event_vars&);
	void Check_Fill_Print_muj(CU_ttH_EDA_event_vars&);
	
	template<class lepton>
	int Print_event_in_file1(FILE*, lepton&, std::vector<pat::Jet>&,
		CU_ttH_EDA_event_vars&);
	
	
	
	/*
	* Variable section
	*/
	
	/// debug flags, set in constructor
	bool verbose_;
	bool dumpHLT_;
	
	edm_Tokens token;	// common tokens for all events
	
	
	/// Triggers, paths: configs filled/updated via run
	HLTConfigProvider hlt_config;
	HLTConfigProvider filter_config;
	
	
	/// Triggers, paths: variables filled via run
	std::string hltTag;
	std::string filterTag;
	
	// counters
	std::map<std::string, unsigned long> n_trigger_fired;	// HLT
	std::map<std::string, unsigned long> n_filter_fired;
	
	std::vector<std::string> trigger_names;
	std::vector<std::string> trigger_names_no_ver;
	
	std::vector<std::string> filter_names;
	std::vector<std::string> filter_names_no_ver;
	
	
	/// Output file is opened/closed through CMS py config
	edm::Service<TFileService> fs_;
	
	
	/// Common sample parameters
	unsigned long event_count;	// running event counter
	
	double total_xs;	// total cross section
	double int_lumi;	// integrated luminosity
	double sample_n;	// total nr of events. Should be long if compatible
	double weight_sample;	// int lumi * xs / sample_n
// 	double weight_gen;
	
	
	/// Cuts
	double min_tight_lepton_pT;
	
	
	/// Selection helper
	MiniAODHelper miniAODhelper;
	
	int MAODHelper_sample_nr;	// past insample_, in-development var. for MAODHelper?
	
	
	/// Histograms
	TH1D *h_tth_syncex1_ele;
	TH1D *h_tth_syncex1_mu;
	
	TH1D *h_hlt;
	TH1D *h_flt;
	
// 	TH1D* h_electron_selection;
// 	TH1D* h_muon_selection;
	
	
	/// Write-out files
	FILE *events_e_cut1, *events_e_cut2, *events_e_cut3, *events_e_cut4,
		*events_e_cut5, *events_e_cut6, *events_e_cut7;
	
	FILE *events_mu_cut1, *events_mu_cut2, *events_mu_cut3, *events_mu_cut4,
		*events_mu_cut5, *events_mu_cut6, *events_mu_cut7;
	
	
	/// Technical variables
	
// 	int return_code;
	
	
	/// Legacy
// 	int NjetMin;
// 	int NjetMax;
// 	int NjetBins;
// 	int NtagMin;
// 	int NtagMax;
// 	int NtagBins;
// 	int NpuMin;
// 	int NpuMax;
// 	int NpuBins;
// 	
// 	int numEvents_;
// 	int numHbb;
// 	int numHdecayToOneParticle;
// 	int numHdecayToOneGluon;
// 	int numHdecayToOnePhoton;
// 	int numHdecayToOneZBoson;
// 	int numHdecayToOneWBoson;
// 	
// 	double mySample_xSec_;
// 	double mySample_nGen_;
// 	double intLumi_;
// 	
// 	double ptmax;
// 	int NptBins;
// 
// 	double jetptmax;
// 	int NjetptBins;
};


#endif