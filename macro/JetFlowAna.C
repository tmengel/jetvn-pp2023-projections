#ifndef MACRO_JETFLOWANA_C
#define MACRO_JETFLOWANA_C

#include <GlobalVariables.C>

#include <eventselection/EventSelector.h>
#include <eventselection/MinBiasCut.h>
#include <eventselection/TowerChi2Cut.h>
#include <eventselection/ZVertexCut.h>
#include <eventselection/LeadTruthJetCut.h>

#include <jetbase/FastJetAlgo.h>
#include <jetbase/JetReco.h>
#include <jetbase/Jetv2.h>
#include <jetbase/TowerJetInput.h>
#include <g4jets/TruthJetInput.h>

#include <jetbackground/CopyAndSubtractJets.h>
#include <jetbackground/DetermineTowerBackground.h>
#include <jetbackground/DetermineTowerRho.h>
#include <jetbackground/FastJetAlgoSub.h>
#include <jetbackground/RetowerCEMC.h>
#include <jetbackground/SubtractTowers.h>
#include <jetbackground/TowerRho.h>

#include <jetbase/FastJetAlgo.h>
#include <jetbase/JetReco.h>
#include <jetbase/TowerJetInput.h>
#include <g4jets/TruthJetInput.h>

#include <jetflow/JetVnTree.h>

#include <fun4all/Fun4AllServer.h>

#include <algorithm>
#include <vector>

R__LOAD_LIBRARY(libjetbase.so)
R__LOAD_LIBRARY(libg4jets.so)
R__LOAD_LIBRARY(libjetbackground.so)
R__LOAD_LIBRARY(libeventselection.so)
R__LOAD_LIBRARY(libjetflow.so)

namespace JetFlow
{
    int VERBOSITY = 0;
    bool is_data = false;
    bool is_mc = false;
    bool has_truth_jets = false;

    bool do_event_selection = false;

    std::string tower_info_prefix = "TOWERINFO_CALIB";

    bool calc_rho = false;
 

    std::vector<float> jet_params = {0.4};
    void SetJetParams(std::vector<float> params) { jet_params = params; }
    void AddJetParam(float param) { if (std::find(jet_params.begin(), jet_params.end(), param) == jet_params.end()) jet_params.push_back(param); }


    bool do_truth_jet_reco = false;
    bool do_iter_sub_jet_reco = false;
    bool do_unsub_jet_reco = false;

    std::string output_file_name = "output.root";

}  // namespace JetFlow


namespace EventSelection 
{
    int VERBOSITY = 0;
    bool do_zvrtx_cut = false;
    std::pair<float,float> z_vrtx_cut_range = {30,-10};
    std::string zvrtx_node = "GlobalVertexMap";
    
    bool do_min_bias_cut = false;
    std::string min_bias_node = "MinimumBiasInfo";
    
    bool do_truth_jet_cut = false;
    std::pair<float,float> truth_jet_pT_hat_range =  {10, 30};
    std::string truth_jet_node = "AntiKt_Truth_r04";
    
    bool do_tower_chi2_cut = false;
    std::vector<std::string> tower_chi2_nodes = {
        "TOWERINFO_CALIB_CEMC",
        "TOWERINFO_CALIB_HCALIN",
        "TOWERINFO_CALIB_HCALOUT"
    };    

    void SetPtHardRange(float pT_hat)
    {
        float pT_min = 0;
        float pT_max = 0;
        if (pT_hat == 10){
            pT_min = 10;
            pT_max = 30;
        } else if (pT_hat == 30){
            pT_min = 30;
            pT_max = 1000;
        } else {
            std::cout << "Invalid pT range for truth jets" << std::endl;
            gSystem -> Exit(0);
        }

        truth_jet_pT_hat_range = {pT_min, pT_max};
        return;
    }

    void SetZertexCutRange(float max, float min = -999.0)
    {
        if (min == -999.0){
            min = -max;
        }
        z_vrtx_cut_range = {max, min};
        return;
    }

} // namespace EventSelection

namespace JetBackground
{
    bool do_flow = false;
    float retower_frac_cut = 0.5;
    float seed_jet_d = 3;
    float seed_jet_pt = 7;
    bool do_mult_rho = false;
    bool do_area_rho = false;
    std::string area_rho_node = "TowerRho_AREA";
    std::string mult_rho_node = "TowerRho_MULT";
}  // namespace JetBackground


void RunEventSelector()
{

    if ( !JetFlow::do_event_selection ) {
        std::cout << "EventSelector: Event selection not enabled. Skipping." << std::endl;
        return ;
    }

    int verbosity = std::max( JetFlow::VERBOSITY, EventSelection::VERBOSITY );

    auto se = Fun4AllServer::instance();

    auto es = new EventSelector();
    es->Verbosity( verbosity );
    if ( EventSelection::do_min_bias_cut  && JetFlow::is_data ) {
        auto mbc = new MinBiasCut();
        mbc -> SetNodeName( EventSelection::min_bias_node );
        es -> AddCut( mbc );
    }
    if ( EventSelection::do_tower_chi2_cut ) {
        auto tcc = new TowerChi2Cut();
        tcc -> SetNodeNames( EventSelection::tower_chi2_nodes );
        es -> AddCut( tcc );
    }
    if ( EventSelection::do_zvrtx_cut ) {
        auto zvc =  new ZVertexCut( EventSelection::z_vrtx_cut_range.first, 
                                    EventSelection::z_vrtx_cut_range.second );
        zvc -> SetNodeName( EventSelection::zvrtx_node );
        es -> AddCut( zvc );
    }
    if( JetFlow::has_truth_jets && EventSelection::do_truth_jet_cut ) {
        auto ltjc = new LeadTruthJetCut( EventSelection::truth_jet_pT_hat_range.first, 
                                    EventSelection::truth_jet_pT_hat_range.second );
        ltjc -> SetNodeName( EventSelection::truth_jet_node );
        es -> AddCut( ltjc );
    }
    se->registerSubsystem(es);

    if ( verbosity > 0 ) { es -> PrintCuts(); }

    return ;

}

void RunJetReco()
{

    int verbosity = std::max( JetFlow::VERBOSITY, Enable::VERBOSITY );

    auto se = Fun4AllServer::instance();

    

    if (JetFlow::do_iter_sub_jet_reco || JetFlow::do_unsub_jet_reco  || JetFlow::calc_rho ) {
        auto rcemc = new RetowerCEMC(); 
        rcemc -> Verbosity( verbosity );
        rcemc -> set_towerinfo( true );
        rcemc -> set_frac_cut( JetBackground::retower_frac_cut );
        rcemc -> set_towerNodePrefix( JetFlow::tower_info_prefix );
        se -> registerSubsystem( rcemc );
    }


    if ( JetFlow::do_iter_sub_jet_reco )  {
        
        auto ijr = new JetReco();
        ijr -> add_input( new TowerJetInput( Jet::CEMC_TOWERINFO_RETOWER, JetFlow::tower_info_prefix ) );
        ijr -> add_input( new TowerJetInput( Jet::HCALIN_TOWERINFO, JetFlow::tower_info_prefix ) );
        ijr -> add_input( new TowerJetInput( Jet::HCALOUT_TOWERINFO,JetFlow:: tower_info_prefix ) );
        ijr -> add_algo( new FastJetAlgoSub( Jet::ANTIKT, 0.2 ),
                         "AntiKt_TowerInfo_HIRecoSeedsRaw_r02" );
        ijr -> set_algo_node("ANTIKT");
        ijr -> set_input_node("TOWER");
        ijr -> Verbosity(0);
        se -> registerSubsystem(ijr);

        auto dtb = new DetermineTowerBackground();
        dtb -> SetBackgroundOutputName( "TowerInfoBackground_Sub1" );
        dtb -> SetFlow( JetBackground::do_flow );
        dtb -> SetSeedType( 0 );
        dtb -> SetSeedJetD( JetBackground::seed_jet_d );
        dtb -> set_towerinfo( true );
        dtb -> Verbosity( 0 ); 
        dtb -> set_towerNodePrefix( JetFlow::tower_info_prefix );
        se -> registerSubsystem( dtb );

        auto casj = new CopyAndSubtractJets();
        casj -> SetFlowModulation( JetBackground::do_flow );
        casj -> Verbosity( 0 ); 
        casj -> set_towerinfo( true );
        casj -> set_towerNodePrefix( JetFlow::tower_info_prefix );
        se -> registerSubsystem( casj );

        auto dtb2 = new DetermineTowerBackground();
        dtb2 -> SetBackgroundOutputName( "TowerInfoBackground_Sub2" );
        dtb2 -> SetFlow( JetBackground::do_flow );
        dtb2 -> SetSeedType( 1 );
        dtb2 -> SetSeedJetPt( JetBackground::seed_jet_pt );
        dtb2 -> set_towerinfo( true );
        dtb2 -> Verbosity( 0 ); 
        dtb2 -> set_towerNodePrefix( JetFlow::tower_info_prefix );
        se -> registerSubsystem( dtb2 );

        auto st = new SubtractTowers();
        st -> SetFlowModulation( JetBackground::do_flow );
        st -> Verbosity( 0 );
        st -> set_towerinfo( true );
        st -> set_towerNodePrefix( JetFlow::tower_info_prefix );
        se -> registerSubsystem( st );

        ijr  = new JetReco();
        ijr -> add_input( new TowerJetInput( Jet::CEMC_TOWERINFO_SUB1, JetFlow::tower_info_prefix ) );
        ijr -> add_input( new TowerJetInput( Jet::HCALIN_TOWERINFO_SUB1, JetFlow::tower_info_prefix ) );
        ijr -> add_input( new TowerJetInput( Jet::HCALOUT_TOWERINFO_SUB1, JetFlow::tower_info_prefix ) );
        for ( auto param : JetFlow::jet_params ) { 
            ijr -> add_algo( new FastJetAlgoSub( Jet::ANTIKT, param, 0 ), 
                            "AntiKt_Tower_r0" + std::to_string(int(param*10)) + "_Sub1" ); 
        }
        ijr -> set_algo_node( "ANTIKT" );
        ijr -> set_input_node( "TOWER" );
        ijr -> Verbosity( verbosity );
        se -> registerSubsystem( ijr );
    }

    if ( JetFlow::calc_rho ) {
        auto dtr = new DetermineTowerRho();
        if( JetBackground::do_area_rho ) {
            dtr -> add_method( TowerRho::Method::AREA, JetBackground::area_rho_node );
        }
        if ( JetBackground::do_mult_rho ) {
            dtr -> add_method( TowerRho::Method::MULT, JetBackground::mult_rho_node );
        }
        dtr -> add_tower_input( new TowerJetInput( Jet::CEMC_TOWERINFO_RETOWER, JetFlow::tower_info_prefix ) );
        dtr -> add_tower_input( new TowerJetInput( Jet::HCALIN_TOWERINFO, JetFlow::tower_info_prefix ) );
        dtr -> add_tower_input( new TowerJetInput( Jet::HCALOUT_TOWERINFO, JetFlow::tower_info_prefix ) );
        dtr -> Verbosity( 0 );
        se -> registerSubsystem( dtr );
    }

    if( JetFlow::do_unsub_jet_reco ) {

        auto rjr = new JetReco();
        rjr -> add_input( new TowerJetInput( Jet::CEMC_TOWERINFO_RETOWER, JetFlow::tower_info_prefix) );
        rjr -> add_input( new TowerJetInput( Jet::HCALIN_TOWERINFO, JetFlow::tower_info_prefix) );
        rjr -> add_input( new TowerJetInput( Jet::HCALOUT_TOWERINFO, JetFlow::tower_info_prefix) );
        for ( auto param : JetFlow::jet_params ) { 
            rjr -> add_algo( new FastJetAlgoSub( Jet::ANTIKT, param, 0 ),  
                            "AntiKt_Tower_r0" + std::to_string(int(param*10) ) ); }
        rjr -> set_algo_node( "ANTIKT" );
        rjr -> set_input_node( "TOWER" );
        rjr -> Verbosity( 0 );
        se->registerSubsystem( rjr );
    }

    return ;

}

void RunJetFlow()
{
    if (JetFlow::is_data && JetFlow::is_mc) {
        std::cout << "JetFlow: Cannot run on both data and MC. Exiting." << std::endl;
        gSystem -> Exit(1);
    } else if( !JetFlow::is_data && !JetFlow::is_mc ) {
        std::cout << "JetFlow: Must specify data or MC. Exiting." << std::endl;
        gSystem -> Exit(1);
    }

    if( !JetFlow::has_truth_jets && JetFlow::do_truth_jet_reco ) {
        if ( JetFlow::is_data ) {
            std::cout << "JetFlow: Cannot run truth jet reco on data. Exiting." << std::endl;
            JetFlow::do_truth_jet_reco = false;
        }
        JetFlow::has_truth_jets = true;
    }

    // need to do truth jet reco before event selection if we are going to do it
   
    if ( JetFlow::do_truth_jet_reco  && JetFlow::is_mc )
    {
        auto se = Fun4AllServer::instance();
        auto tjr = new JetReco();
        TruthJetInput * tji = new TruthJetInput( Jet::PARTICLE );
        tji -> add_embedding_flag( 0 ); 
        tjr->add_input( tji );
        for ( auto param : JetFlow::jet_params ) { 
            tjr -> add_algo( new FastJetAlgo( Jet::ANTIKT, param), 
                            "AntiKt_Truth_r0" + std::to_string(int(param*10) ) ); 
        }
        tjr -> set_algo_node( "ANTIKT" );
        tjr -> set_input_node( "TRUTH" );
        tjr -> Verbosity( JetFlow::VERBOSITY );
        se -> registerSubsystem( tjr );
    }


    if ( JetFlow::do_event_selection ) { RunEventSelector(); }
    if ( JetFlow::do_truth_jet_reco || JetFlow::do_iter_sub_jet_reco || JetFlow::do_unsub_jet_reco ) { RunJetReco(); }

    auto se = Fun4AllServer::instance();
    auto jvt = new JetVnTree();
    jvt -> set_reco_truth_jet_nodes ( "AntiKt_Tower_r04", "AntiKt_Truth_r04" );
    jvt -> add_mult_rho_node( "TowerRho_MULT" );
    jvt -> set_max_eta( 1.1 );
    jvt -> add_truth_jet_eta_cut( 1.1 );
    jvt -> add_truth_jet_min_pT( 10.0 );
    jvt -> set_output_filename( JetFlow::output_file_name );
    jvt -> is_pythia8( JetFlow::is_mc );
    jvt -> Verbosity( JetFlow::VERBOSITY );
    se -> registerSubsystem( jvt );

    return ;
}



#endif // MACRO_JETFLOWANA_C