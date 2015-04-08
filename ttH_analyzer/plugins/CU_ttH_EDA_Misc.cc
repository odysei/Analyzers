#ifndef CU_ttH_EDA_Misc_cc
#define CU_ttH_EDA_Misc_cc


/// Includes
#include "CU_ttH_EDA.h"



int CU_ttH_EDA::End_Run_hist_fill()
{
	TAxis *axis = h_hlt->GetXaxis();
	if (!axis)
		return 1;
	
	for (std::map<std::string, unsigned long>::const_iterator iter =
		n_trigger_fired.begin();
		iter != n_trigger_fired.end(); ++iter) {
		
		int bin_num = axis->FindBin((iter->first).c_str());
		h_hlt->Fill(bin_num - 1, (iter->second));
	}
	
	
	axis = h_flt->GetXaxis();
	if (!axis)
		return 1;
	
	for (std::map<std::string, unsigned long>::const_iterator iter =
		n_filter_fired.begin();
		iter != n_filter_fired.end(); ++iter) {
		
		int bin_num = axis->FindBin((iter->first).c_str());
		h_flt->Fill(bin_num - 1, (iter->second));
	}
	
	return 0;
}



void CU_ttH_EDA::Update_common_vars(const edm::Event &iEvent,
	CU_ttH_EDA_event_vars &local)
{
	local.run_nr = iEvent.id().run();
	local.event_nr = iEvent.id().event();
	local.lumisection_nr = iEvent.id().luminosityBlock();
}



int CU_ttH_EDA::Check_beam_spot(edm::Handle<reco::BeamSpot> BS)
{
	if (!BS.isValid())
		return 1;
	
// 	math::XYZPoint beamSpotPosition;
// 	beamSpotPosition.SetCoordinates(0, 0, 0);
// //		double BSx = 0, BSy = 0, BSz = 0;
// 	
// 	double BSx = BS->x0();
// 	double BSy = BS->y0();
// 	double BSz = BS->z0();
// 	beamSpotPosition = BS->position();
//
//		if (verbose_)
//			printf("\t BeamSpot: x = %.2f,\t y = %.2f,\t z = %.2f \n",
//				BSx, BSy, BSz );
	if (verbose_)
		printf("\t BeamSpot: x = %.2f,\t y = %.2f,\t z = %.2f \n",
			BS->x0(), BS->y0(), BS->z0() );
	
	return 0;
}



int CU_ttH_EDA::Check_triggers(
	edm::Handle<edm::TriggerResults> triggerResults,
	CU_ttH_EDA_event_vars &local)
{
	if (!triggerResults.isValid()) {
		std::cerr << "Trigger results not valid for tag " << hltTag <<
			std::endl;
		return 1;
	}
	
	for (unsigned int i = 0; i < trigger_names.size(); ++i) {
		std::string pathName = trigger_names[i];
		unsigned int hltIndex = hlt_config.triggerIndex(pathName);
		
		if (hltIndex >= triggerResults->size() )
			continue;
		
		bool trigger_accept = triggerResults->accept(hltIndex);
		int prescale = -1;	//hlt_config.prescaleValue(iEvent, iSetup, pathName);
		
		if (trigger_accept) {
			if (pathName == "HLT_Ele27_eta2p1_WP85_Gsf_v1")
				local.pass_single_e = true;
			
			if (pathName == "HLT_IsoMu24_eta2p1_IterTrk02_v1")
				local.pass_single_mu = true;
		}
		
		if (verbose_ && dumpHLT_)
			std::cout << " =====>  HLT: path name = " << pathName <<
				",\t prescale = " << prescale << ",\t pass = " << trigger_accept <<
				std::endl;
		
		std::string pathNameNoVer = hlt_config.removeVersion(pathName);
		
		if (trigger_accept)
			++n_trigger_fired[pathNameNoVer];
	}
	
	return 0;
}



int CU_ttH_EDA::Check_filters(
	edm::Handle<edm::TriggerResults> filterResults)
{
	if (!filterResults.isValid()) {
		std::cerr << "Trigger results not valid for tag " << filterTag <<
			std::endl;
		return 1;
	}
	
	for (unsigned int i = 0; i < filter_names.size(); ++i) {
		std::string pathName = filter_names[i];
		unsigned int hltIndex = filter_config.triggerIndex(pathName);
		
		if (hltIndex >= filterResults->size())
			continue;
		
		bool filter_accept = filterResults->accept(hltIndex);
		int prescale = -1;	//filter_config.prescaleValue(iEvent, iSetup, pathName);
		
		if (verbose_ && dumpHLT_)
			std::cout << " =====>  Filter: path name = " << pathName <<
				",\t prescale = " << prescale << ",\t pass = " << filter_accept
				<< std::endl; 
		
		std::string pathNameNoVer = filter_config.removeVersion(pathName);
		
		if (filter_accept)
			++n_filter_fired[pathNameNoVer];
	}
	
	return 0;
}



int CU_ttH_EDA::Check_vertices_set_MAODhelper(
	edm::Handle<reco::VertexCollection> vertices)
{
	/// Primary vertex handling
	if (!vertices.isValid())
		return 1;
	
	reco::Vertex vertex;
	int n_PVs = 0;
	
	for (reco::VertexCollection::const_iterator vtx = vertices->begin();
		vtx != vertices->end(); ++vtx) {
		
		if (vtx->isFake() || vtx->ndof() < 4.0 || abs(vtx->z()) > 24.0 ||
			abs(vtx->position().Rho()) > 2.0)
			continue;
		
		if (n_PVs == 0)
			vertex = *vtx;
		
		++n_PVs;
	}
	
	if (verbose_)
		printf("\t Event PV: x = %.3f,\t y = %.3f,\t z = %.3f \n",
			vertex.x(), vertex.y(), vertex.z());
	
	if (n_PVs > 0)
		miniAODhelper.SetVertex(vertex);	// FIXME?: overload miniAODhelper::SetVertex(reco::Vertex&)
	
	return 0;
}



/// Taggers
int CU_ttH_EDA::Higgs_tagger(Handle<boosted::SubFilterJetCollection> subfilter_jets,
	CU_ttH_EDA_event_vars &local)
{
	local.n_Htags = 0;
	
	if (!subfilter_jets.isValid())
		return 1;
	
	boosted::SubFilterJetCollection subfilterjets =
		BoostedUtils::GetSortedByPt(*subfilter_jets);
	
	for (boosted::SubFilterJetCollection::iterator higgsJet = subfilterjets.begin();
		higgsJet != subfilterjets.end(); ++higgsJet) {
		// pt and eta requirements for top jet
		if (higgsJet->fatjet.pt() <= 250. || abs(higgsJet->fatjet.eta()) >= 1.8)
			continue;
		
		int numBtagFiltJets = 0;
		std::vector<pat::Jet> filterjets = higgsJet->filterjets;
		int numFiltJets = filterjets.size();
		for (int ijet = 0; ijet < numFiltJets; ++ijet) {
			if (verbose_) {
				printf("\t\t filt jet %2d:\t pT = %.1f,\t eta = %.2f,\t phi = %.2f,\t CSVv2 = %+5.3f,\t CSVv1 = %+5.3f \n",
					ijet, filterjets[ijet].pt(), filterjets[ijet].eta(),
					filterjets[ijet].phi(),
					filterjets[ijet].bDiscriminator("combinedInclusiveSecondaryVertexV2BJetTags"),
					filterjets[ijet].bDiscriminator("combinedSecondaryVertexBJetTags"));
			}
			
			if (filterjets[ijet].pt() <= 20. ||
				abs(filterjets[ijet].eta()) >= 2.5)
				continue;
			
			// b-tag medium WP
			if (filterjets[ijet].bDiscriminator("combinedInclusiveSecondaryVertexV2BJetTags") < 0.814)
				continue;
			
			++numBtagFiltJets;
		}
		
		if (verbose_) {
			printf("\t Higgs jet %2d:\t pT = %.1f,\t eta = %.2f,\t phi = %.2f,\t numFiltJets = %2d,\t numBtagFiltJets = %2d\n",
				int(higgsJet - subfilterjets.begin()),
				higgsJet->fatjet.pt(), higgsJet->fatjet.eta(),
				higgsJet->fatjet.phi(), numFiltJets, numBtagFiltJets);
		}
		
		if (numBtagFiltJets >= 2)
			++local.n_Htags;
	}
	
	return 0;
}



int CU_ttH_EDA::Top_tagger(Handle<boosted::HEPTopJetCollection> top_jets,
	CU_ttH_EDA_event_vars &local)
{
	local.n_ttags = 0;
	
	if (!top_jets.isValid())
		return 1;
	
	boosted::HEPTopJetCollection heptopjets =
		BoostedUtils::GetSortedByPt(*top_jets);
	
	for (boosted::HEPTopJetCollection::iterator topJet = heptopjets.begin();
		topJet != heptopjets.end(); ++topJet) {
		// pt and eta requirements on top jet
		if (topJet->fatjet.pt() <= 250. || abs(topJet->fatjet.eta()) >= 1.8)
			continue;
		
		// pt and eta requirements on subjets
		if (topJet->nonW.pt() <= 20 || abs(topJet->nonW.eta()) >= 2.5 ||
			topJet->W1.pt() <= 20 || abs(topJet->W1.eta()) >= 2.5 ||
			topJet->W2.pt() <= 20 || abs(topJet->W2.eta()) >= 2.5)
			continue;
		
		
		// must be top-tagged
		if (!BoostedUtils::GetTopTag(*topJet))
			continue;
		
		++local.n_ttags;
	}
	
	return 0;
}



/// Other functions
void CU_ttH_EDA::Check_Fill_Print_muj(CU_ttH_EDA_event_vars &local)
{
	int fill_itr = 0;
	
	h_tth_syncex1_mu->Fill(0.5 + fill_itr++);	// fills 0.5 first
	
	if (local.pass_single_mu) {
		h_tth_syncex1_mu->Fill(0.5 + fill_itr++);
		Print_event_in_file1(events_mu_cut1,
			local.mu_selected_sorted, local.jets_selected_sorted, local);
	} else
		return;
	
	if (local.n_muons == 1) {
		h_tth_syncex1_mu->Fill(0.5 + fill_itr++);
		Print_event_in_file1(events_mu_cut2,
			local.mu_selected_sorted, local.jets_selected_sorted, local);
	} else
		return;
	
	if (local.n_electrons == 0) {
		h_tth_syncex1_mu->Fill(0.5 + fill_itr++);
		Print_event_in_file1(events_mu_cut3,
			local.mu_selected_sorted, local.jets_selected_sorted, local);
	} else
		return;
	
	if (local.n_jets >= 4) {
		h_tth_syncex1_mu->Fill(0.5 + fill_itr++);
		Print_event_in_file1(events_mu_cut4,
			local.mu_selected_sorted, local.jets_selected_sorted, local);
	} else
		return;
	
	if (local.n_btags >= 2) {
		h_tth_syncex1_mu->Fill(0.5 + fill_itr++);
		Print_event_in_file1(events_mu_cut5,
			local.mu_selected_sorted, local.jets_selected_sorted, local);
	} else
		return;
	
	if (local.n_ttags >= 1) {
		h_tth_syncex1_mu->Fill(0.5 + fill_itr++);
		Print_event_in_file1(events_mu_cut6,
			local.mu_selected_sorted, local.jets_selected_sorted, local);
	} else
		return;
	
	if (local.n_Htags >= 1) {
		h_tth_syncex1_mu->Fill(0.5 + fill_itr);
		Print_event_in_file1(events_mu_cut7,
			local.mu_selected_sorted, local.jets_selected_sorted, local);
	} else
		return;
}



void CU_ttH_EDA::Check_Fill_Print_ej(CU_ttH_EDA_event_vars &local)
{
	int fill_itr = 0;
	
	h_tth_syncex1_ele->Fill(0.5 + fill_itr++);	// fills 0.5 first
	
	if (local.pass_single_e) {
		h_tth_syncex1_ele->Fill(0.5 + fill_itr++);
		Print_event_in_file1(events_e_cut1,
			local.e_selected_sorted, local.jets_selected_sorted, local);
	} else
		return;
		
	if (local.n_electrons == 1) {
		h_tth_syncex1_ele->Fill(0.5 + fill_itr++);
		Print_event_in_file1(events_e_cut2,
			local.e_selected_sorted, local.jets_selected_sorted, local);
	} else
		return;
	
	if (local.n_muons == 0) {
		h_tth_syncex1_ele->Fill(0.5 + fill_itr++);
		Print_event_in_file1(events_e_cut3,
			local.e_selected_sorted, local.jets_selected_sorted, local);
	} else
		return;
	
	if (local.n_jets >= 4) {
		h_tth_syncex1_ele->Fill(0.5 + fill_itr++);
		Print_event_in_file1(events_e_cut4,
			local.e_selected_sorted, local.jets_selected_sorted, local);
	} else
		return;
	
	if (local.n_btags >= 2) {
		h_tth_syncex1_ele->Fill(0.5 + fill_itr++);
		Print_event_in_file1(events_e_cut5,
			local.e_selected_sorted, local.jets_selected_sorted, local);
	} else
		return;
	
	if (local.n_ttags >= 1) {
		h_tth_syncex1_ele->Fill(0.5 + fill_itr++);
		Print_event_in_file1(events_e_cut6,
			local.e_selected_sorted, local.jets_selected_sorted, local);
	} else
		return;
	
	if (local.n_Htags >= 1) {
		h_tth_syncex1_ele->Fill(0.5 + fill_itr);
		Print_event_in_file1(events_e_cut7,
			local.e_selected_sorted, local.jets_selected_sorted, local);
	} else
		return;
}



template<class lepton>
int CU_ttH_EDA::Print_event_in_file1(FILE *file,
	lepton &lpt, std::vector<pat::Jet> &jets, CU_ttH_EDA_event_vars &local)
{
	// print generic event info
	fprintf(file, "%6d %8d %10d   ",
		local.run_nr,
		local.lumisection_nr,
		local.event_nr);
	
	// print lepton info
	if (lpt.size() != 0)
		fprintf(file, "%6.2f %+4.2f %+4.2f   ",
			lpt[0].pt(),
			lpt[0].eta(),
			lpt[0].phi());
	else
		fprintf(file, "%6.2f %+4.2f %+4.2f   ",
			-99.,
			-99.,
			-99.);
	
	// print jet(s) info
	if (jets.size() >= 4) {
		fprintf(file, "%6.2f %6.2f %6.2f %6.2f   %+7.3f %+7.3f %+7.3f %+7.3f   ",
			jets[0].pt(),
			jets[1].pt(),
			jets[2].pt(),
			jets[3].pt(),
			jets[0].bDiscriminator("combinedInclusiveSecondaryVertexV2BJetTags"),
			jets[1].bDiscriminator("combinedInclusiveSecondaryVertexV2BJetTags"),
			jets[2].bDiscriminator("combinedInclusiveSecondaryVertexV2BJetTags"),
			jets[3].bDiscriminator("combinedInclusiveSecondaryVertexV2BJetTags"));
	} else {
		switch (jets.size()) {
			case 3:
			fprintf(file, "%6.2f %6.2f %6.2f %6.2f   %+7.3f %+7.3f %+7.3f %+7.3f   ",
				jets[0].pt(),
				jets[1].pt(),
				jets[2].pt(),
				-99.,
				jets[0].bDiscriminator("combinedInclusiveSecondaryVertexV2BJetTags"),
				jets[1].bDiscriminator("combinedInclusiveSecondaryVertexV2BJetTags"),
				jets[2].bDiscriminator("combinedInclusiveSecondaryVertexV2BJetTags"),
				-99.);
			break;
			
			case 2:
			fprintf(file, "%6.2f %6.2f %6.2f %6.2f   %+7.3f %+7.3f %+7.3f %+7.3f   ",
				jets[0].pt(),
				jets[1].pt(),
				-99.,
				-99.,
				jets[0].bDiscriminator("combinedInclusiveSecondaryVertexV2BJetTags"),
				jets[1].bDiscriminator("combinedInclusiveSecondaryVertexV2BJetTags"),
				-99.,
				-99.);
			break;
			
			case 1:
			fprintf(file, "%6.2f %6.2f %6.2f %6.2f   %+7.3f %+7.3f %+7.3f %+7.3f   ",
				jets[0].pt(),
				-99.,
				-99.,
				-99.,
				jets[0].bDiscriminator("combinedInclusiveSecondaryVertexV2BJetTags"),
				-99.,
				-99.,
				-99.);
			break;
			
			default:
				fprintf(file, "%6.2f %6.2f %6.2f %6.2f   %+7.3f %+7.3f %+7.3f %+7.3f   ",
					-99.,
					-99.,
					-99.,
					-99.,
					-99.,
					-99.,
					-99.,
					-99.);
		}
	}
	
	// print number of tags
	fprintf(file, "%2d  %2d   %2d  %2d\n",
		local.n_jets,
		local.n_btags,
		local.n_ttags,
		local.n_Htags);
	
	
	return ferror(file);
}



#endif