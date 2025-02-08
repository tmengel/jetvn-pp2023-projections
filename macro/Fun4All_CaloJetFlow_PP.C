#pragma once
#if ROOT_VERSION_CODE >= ROOT_VERSION(6,00,0)

// f4a macros
#include <G4_Global.C>
#include <G4_Magnet.C>
#include <GlobalVariables.C>

// fun4all includes
#include <fun4all/Fun4AllServer.h>
#include <fun4all/SubsysReco.h>
#include <fun4all/Fun4AllUtils.h>
#include <fun4all/Fun4AllInputManager.h>
#include <fun4all/Fun4AllDstInputManager.h>
#include <fun4all/Fun4AllDstOutputManager.h>
#include <fun4all/Fun4AllOutputManager.h>

// coresoftware headers
#include <ffamodules/FlagHandler.h>
#include <ffamodules/HeadReco.h>
#include <ffamodules/SyncReco.h>
#include <ffamodules/CDBInterface.h>

// phool includes
#include <phool/PHRandomSeed.h>
#include <phool/recoConsts.h>

#include <globalvertex/GlobalVertexReco.h>

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

#include <jetflow/JetVnTree.h>

// #include <eventplaneinfo/Eventplaneinfo.h>
// #include <eventplaneinfo/EventPlaneReco.h>

// standard includes
#include <iostream>
#include <string>
#include <utility>

// load libraries
R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libffamodules.so)
R__LOAD_LIBRARY(libglobalvertex.so)
R__LOAD_LIBRARY(libjetbase.so)
R__LOAD_LIBRARY(libg4jets.so)
R__LOAD_LIBRARY(libjetbackground.so)
R__LOAD_LIBRARY(libeventselection.so)
R__LOAD_LIBRARY(libjetflow.so)

#endif



void Fun4All_CaloJetFlow_PP(
    const int nEvents = 1,
    const char * filelisttruth = "dst_truth_jet.list",
    const char * filelistcalo = "dst_calo_cluster.list",
    const char * filelistglobal = "dst_global.list",
    const char * output_file_name = "output.root"
)
{

    // set analysis parameters
    Enable::VERBOSITY = 0;

    bool is_data = false;
    bool is_mc = !is_data;
    bool has_truth_jets = true && is_mc;
  
    std::string tower_info_prefix = "TOWERINFO_CALIB";

    bool do_zvrtx_cut = true;
    std::pair<float,float> z_vrtx_cut_range = {30,-30};
    
    bool do_min_bias_cut = false && is_data;

    bool do_truth_jet_cut = true && has_truth_jets;
    std::pair<float,float> truth_jet_pT_hat_range =  {10, 30};

    bool do_tower_chi2_cut = true;
    std::vector<std::string> tower_chi2_nodes = {
        "TOWERINFO_CALIB_CEMC",
        "TOWERINFO_CALIB_HCALIN",
        "TOWERINFO_CALIB_HCALOUT"
    };


    std::vector<float> jet_params = {0.4};
    bool do_truth_jet_reco = false && has_truth_jets;
    bool do_iter_sub_jet_reco = false;
    bool do_unsub_jet_reco = true;

    bool calc_iter_bkgd = true && do_iter_sub_jet_reco;
    bool calc_area_rho = true;
    bool calc_mult_rho = true;
    

    

    //------------------------------------------------
    // initialize F4A server
    //------------------------------------------------
    auto se = Fun4AllServer::instance();
    se->Verbosity(Enable::VERBOSITY);

    // PHRandomSeed::Verbosity(1);

    auto rc = recoConsts::instance();
    rc->set_IntFlag("RUNNUMBER",1);
    Enable::CDB = true;
    rc -> set_StringFlag("CDB_GLOBALTAG", CDB::global_tag);
    rc -> set_uint64Flag("TIMESTAMP", CDB::timestamp);
    
    auto sync = new SyncReco();
    se -> registerSubsystem(sync);
    // auto head = new HeadReco();
    // se -> registerSubsystem(head);
    // auto flag = new FlagHandler();
    // se -> registerSubsystem(flag);

    //------------------------------------------------
    // event selection
    //------------------------------------------------

    auto es = new EventSelector();
    es->Verbosity( 1 );
    if ( do_min_bias_cut ) {
        auto mbc = new MinBiasCut();
        mbc -> SetNodeName( "MinimumBiasInfo" );
        es -> AddCut( mbc );
    }
    if ( do_tower_chi2_cut ) {
        auto tcc = new TowerChi2Cut();
        tcc -> SetNodeNames( tower_chi2_nodes );
        es -> AddCut( tcc );
    }
    if ( do_zvrtx_cut ) {
        auto zvc =  new ZVertexCut( z_vrtx_cut_range.first, z_vrtx_cut_range.second );
        zvc -> SetNodeName( "GlobalVertexMap" );
        es -> AddCut( zvc );
    }
    if( do_truth_jet_cut ) {
        auto ltjc = new LeadTruthJetCut( truth_jet_pT_hat_range.first, truth_jet_pT_hat_range.second );
        ltjc -> SetNodeName( "AntiKt_Truth_r04" );
        es -> AddCut( ltjc );
    }
    se->registerSubsystem(es);

    // -----------------------------------
    // background reco
    // -----------------------------------

    auto rcemc = new RetowerCEMC(); 
    rcemc -> Verbosity( 0 ); 
    rcemc -> set_towerinfo( true );
    rcemc -> set_frac_cut( 0.5 ); 
    rcemc -> set_towerNodePrefix( tower_info_prefix );
    se -> registerSubsystem( rcemc );

    // // iterative background subtraction
    if( calc_iter_bkgd ) {

        auto ijr = new JetReco();
        ijr -> add_input( new TowerJetInput( Jet::CEMC_TOWERINFO_RETOWER, tower_info_prefix ) );
        ijr -> add_input( new TowerJetInput( Jet::HCALIN_TOWERINFO, tower_info_prefix ) );
        ijr -> add_input( new TowerJetInput( Jet::HCALOUT_TOWERINFO, tower_info_prefix ) );
        ijr -> add_algo( new FastJetAlgoSub( Jet::ANTIKT, 0.2 ), "AntiKt_TowerInfo_HIRecoSeedsRaw_r02" );
        ijr -> set_algo_node("ANTIKT");
        ijr -> set_input_node("TOWER");
        ijr -> Verbosity(0);
        se -> registerSubsystem(ijr);

        auto dtb = new DetermineTowerBackground();
        dtb -> SetBackgroundOutputName( "TowerInfoBackground_Sub1" );
        dtb -> SetFlow( false );
        dtb -> SetSeedType( 0 );
        dtb -> SetSeedJetD( 3 );
        dtb -> set_towerinfo( true );
        dtb -> Verbosity( 0 ); 
        dtb -> set_towerNodePrefix( tower_info_prefix );
        se -> registerSubsystem( dtb );

        auto casj = new CopyAndSubtractJets();
        casj -> SetFlowModulation( false );
        casj -> Verbosity( 0 ); 
        casj -> set_towerinfo( true );
        casj -> set_towerNodePrefix( tower_info_prefix );
        se -> registerSubsystem( casj );

        auto dtb2 = new DetermineTowerBackground();
        dtb2 -> SetBackgroundOutputName( "TowerInfoBackground_Sub2" );
        dtb2 -> SetFlow( false );
        dtb2 -> SetSeedType( 1 );
        dtb2 -> SetSeedJetPt( 7 );
        dtb2 -> set_towerinfo( true );
        dtb2 -> Verbosity( 0 ); 
        dtb2 -> set_towerNodePrefix( tower_info_prefix );
        se -> registerSubsystem( dtb2 );

        auto st = new SubtractTowers();
        st -> SetFlowModulation( false );
        st -> Verbosity( 0 );
        st -> set_towerinfo( true );
        st -> set_towerNodePrefix( tower_info_prefix );
        se -> registerSubsystem( st );

    }
    
    // rho background subtraction
    if( calc_area_rho || calc_mult_rho ) {

        auto dtr = new DetermineTowerRho();
        if( calc_area_rho ) { 
            dtr -> add_method( TowerRho::Method::AREA, "TowerRho_AREA" ); 
        }
        if ( calc_mult_rho ) { 
            dtr -> add_method( TowerRho::Method::MULT, "TowerRho_MULT" ); 
        }
        dtr -> add_tower_input( new TowerJetInput( Jet::CEMC_TOWERINFO_RETOWER, tower_info_prefix ) );
        dtr -> add_tower_input( new TowerJetInput( Jet::HCALIN_TOWERINFO, tower_info_prefix ) );
        dtr -> add_tower_input( new TowerJetInput( Jet::HCALOUT_TOWERINFO, tower_info_prefix ) );
        dtr -> Verbosity( 0 );
        se -> registerSubsystem( dtr );
    }

    // -----------------------------------
    // Jet reco
    // -----------------------------------
    if ( do_truth_jet_reco ) {
        auto tjr = new JetReco();
        TruthJetInput * tji = new TruthJetInput( Jet::PARTICLE );
        tji -> add_embedding_flag( 0 );  // changes depending on signal vs. embedded
        tjr->add_input( tji );
        for ( auto param : jet_params ) { 
            tjr -> add_algo( new FastJetAlgo( Jet::ANTIKT, param), 
                            "AntiKt_Truth_r0" + std::to_string(int(param*10) ) ); 
        }
        tjr -> set_algo_node( "ANTIKT" );
        tjr -> set_input_node( "TRUTH" );
        tjr -> Verbosity( 0 );
        se -> registerSubsystem( tjr );
    }

    if ( do_iter_sub_jet_reco )  {
        auto ijr  = new JetReco();
        ijr -> add_input( new TowerJetInput( Jet::CEMC_TOWERINFO_SUB1, tower_info_prefix ) );
        ijr -> add_input( new TowerJetInput( Jet::HCALIN_TOWERINFO_SUB1, tower_info_prefix ) );
        ijr -> add_input( new TowerJetInput( Jet::HCALOUT_TOWERINFO_SUB1, tower_info_prefix ) );
        for ( auto param : jet_params ) { 
            ijr -> add_algo( new FastJetAlgoSub( Jet::ANTIKT, param, 0 ), 
                            "AntiKt_Tower_r0" + std::to_string(int(param*10)) + "_Sub1" ); 
        }
        ijr -> set_algo_node( "ANTIKT" );
        ijr -> set_input_node( "TOWER" );
        ijr -> Verbosity( 0 );
        se -> registerSubsystem( ijr );
    }

    if( do_unsub_jet_reco ) {
        auto rjr = new JetReco();
        rjr -> add_input( new TowerJetInput( Jet::CEMC_TOWERINFO_RETOWER, tower_info_prefix) );
        rjr -> add_input( new TowerJetInput( Jet::HCALIN_TOWERINFO, tower_info_prefix) );
        rjr -> add_input( new TowerJetInput( Jet::HCALOUT_TOWERINFO, tower_info_prefix) );
        for ( auto param : jet_params ) { 
            rjr -> add_algo( new FastJetAlgoSub( Jet::ANTIKT, param, 0 ),  
                            "AntiKt_Tower_r0" + std::to_string(int(param*10) ) ); }
        rjr -> set_algo_node( "ANTIKT" );
        rjr -> set_input_node( "TOWER" );
        rjr -> Verbosity( 0 );
        se->registerSubsystem( rjr );
    }


    // -----------------------------------
    // Jet flow
    // -----------------------------------
    auto jvt = new JetVnTree();
    jvt -> set_reco_truth_jet_nodes ( "AntiKt_Tower_r04", "AntiKt_Truth_r04" );
    jvt -> add_mult_rho_node( "TowerRho_MULT" );
    jvt -> set_max_eta( 1.1 );
    jvt -> add_truth_jet_eta_cut( 1.1 );
    jvt -> add_truth_jet_min_pT( 10.0 );
    jvt -> set_output_filename( output_file_name );
    jvt -> Verbosity( 1 );
    se -> registerSubsystem( jvt );


    //-----------------------------------
    // Input managers
    //-----------------------------------

    Fun4AllInputManager *intrue = new Fun4AllDstInputManager("DSTtruth");
    intrue->AddListFile(filelisttruth,1);
    se->registerInputManager(intrue);

    Fun4AllInputManager *in2 = new Fun4AllDstInputManager("DSTcalo");
    in2->AddListFile(filelistcalo,1);
    se->registerInputManager(in2);

    Fun4AllInputManager *in3 = new Fun4AllDstInputManager("DSTglobal");
    in3->AddListFile(filelistglobal,1);
    se->registerInputManager(in3);

  //-----------------------------------
  // Run the analysis
  //-----------------------------------
  
  se->run(nEvents);
  se->End();

  gSystem->Exit(0);
  return 0;

}

