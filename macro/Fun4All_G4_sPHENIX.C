#ifndef MACRO_FUN4ALLG4SPHENIX_C
#define MACRO_FUN4ALLG4SPHENIX_C

#include <GlobalVariables.C>

#include <G4Setup_sPHENIX.C>
#include <G4_Mbd.C>
#include <G4_CaloTrigger.C>
#include <G4_DSTReader.C>
#include <G4_Global.C>
#include <G4_Input.C>
#include <G4_Production.C>
#include <JetFlowAna.C>

#include <ffamodules/FlagHandler.h>
#include <ffamodules/HeadReco.h>
#include <ffamodules/SyncReco.h>
#include <ffamodules/CDBInterface.h>

#include <fun4all/Fun4AllDstOutputManager.h>
#include <fun4all/Fun4AllOutputManager.h>
#include <fun4all/Fun4AllServer.h>

#include <phpythia8/PHPythia8.h>
#include <phpythia8/PHPy8JetTrigger.h>

#include <phool/PHRandomSeed.h>
#include <phool/recoConsts.h>

R__LOAD_LIBRARY(libfun4all.so)
R__LOAD_LIBRARY(libffamodules.so)
R__LOAD_LIBRARY(libPHPythia8.so)


int Fun4All_G4_sPHENIX(
    const int nEvents = 1,
    const std::string & outputFile = "output.root",
    const std::string & pythia_configfile = "/sphenix/user/tmengel/jetvn-pp2023-projections/macro/phpythia8_15GeV_JS_MDC2.cfg"
    )
{


    // =================================================================== 
    // ============================= F4All ===============================
    // ===================================================================
    
    Enable::VERBOSITY  = 0;
    
    auto se = Fun4AllServer::instance();
    se -> Verbosity(Enable::VERBOSITY);

    // PHRandomSeed::Verbosity(1);

    // recoConsts initialization
    auto rc = recoConsts::instance();


    // =================================================================== 
    // ============================= INPUT ===============================
    // ===================================================================

    //Input::PILEUPRATE = 3e6; // 3MHz for pp

    Input::PYTHIA8 = true;
    Input::VERBOSITY = Enable::VERBOSITY; 
    PYTHIA8::config_file = pythia_configfile;
    Input::BEAM_CONFIGURATION = Input::pp_ZEROANGLE;
    // Input::BEAM_CONFIGURATION = Input::pp_COLLISION; // for 2024 sims we want the pp geometry for no pileup sims

    InputInit();
    if ( Input::PYTHIA8 ) {
        auto p8_js_signal_trigger = new PHPy8JetTrigger();
        p8_js_signal_trigger->SetEtaHighLow(1.5, -1.5);  // Set eta acceptance for particles into the jet between +/- 1.5
        p8_js_signal_trigger->SetJetR(0.4);              // Set the radius for the trigger jet
    	p8_js_signal_trigger->SetMinJetPt(10);  // require a 10 GeV minimum pT jet in the event
        INPUTGENERATOR::Pythia8->register_trigger(p8_js_signal_trigger);
        INPUTGENERATOR::Pythia8->set_trigger_AND();
        Input::ApplysPHENIXBeamParameter(INPUTGENERATOR::Pythia8);
    }

    if (Input::PILEUPRATE > 0) {
        Input::ApplysPHENIXBeamParameter(INPUTMANAGER::HepMCPileupInputManager);
    }

    InputRegister();
 
    // =================================================================== 
    // ============================= FLAGS ===============================
    // ===================================================================

    rc -> set_IntFlag("RUNNUMBER",1);

    auto sync = new SyncReco();
    se -> registerSubsystem(sync);

    auto head = new HeadReco();
    se -> registerSubsystem(head);

    auto flag = new FlagHandler();
    se -> registerSubsystem(flag);

    // =================================================================== 
    // ============================= G4 Settings =========================
    // ===================================================================

    // Enable::DSTOUT = true;
    Enable::DSTOUT_COMPRESS = false;

    // Enable::MBD = true;
    // Enable::MBDRECO = Enable::MBD && true;
    Enable::MBDFAKE = true;  

    Enable::PIPE = true;
    Enable::PIPE_ABSORBER = true;

    Enable::CEMC = true;
    Enable::CEMC_ABSORBER = true;
    Enable::CEMC_CELL = Enable::CEMC && true;
    Enable::CEMC_TOWER = Enable::CEMC_CELL && true;

    Enable::HCALIN = true;
    Enable::HCALIN_ABSORBER = true;
    Enable::HCALIN_CELL = Enable::HCALIN && true;
    Enable::HCALIN_TOWER = Enable::HCALIN_CELL && true;

    Enable::MAGNET = true;
    Enable::MAGNET_ABSORBER = true;

    Enable::HCALOUT = true;
    Enable::HCALOUT_ABSORBER = true;
    Enable::HCALOUT_CELL = Enable::HCALOUT && true;
    Enable::HCALOUT_TOWER = Enable::HCALOUT_CELL && true;

    Enable::EPD = true;
    Enable::EPD_TILE = Enable::EPD && true;

    Enable::BEAMLINE = true;
    Enable::ZDC = true;
    Enable::ZDC_TOWER = Enable::ZDC && true;
    Enable::ZDC_EVAL = Enable::ZDC_TOWER && true;

    Enable::PLUGDOOR_ABSORBER = true;

    // Enable::CALOTRIGGER = Enable::CEMC_TOWER && Enable::HCALIN_TOWER && Enable::HCALOUT_TOWER && false;

    // Enable::GLOBAL_FASTSIM = true;
    Enable::GLOBAL_RECO = true;

    // Enable::CENTRALITY = false;
    Enable::BLACKHOLE = true;

    Enable::CDB = true;
    rc -> set_StringFlag("CDB_GLOBALTAG",CDB::global_tag);
    rc -> set_uint64Flag("TIMESTAMP",CDB::timestamp);

    G4Init();
    G4Setup();
  
    // ===================================================================
    // ============================= Subsystems ==========================
    // ===================================================================

    // mbd
    if ( (Enable::MBD && Enable::MBDRECO) || Enable::MBDFAKE ) {
        Mbd_Reco();
    } 

    // calo cells
    if ( Enable::CEMC_CELL ) {
        CEMC_Cells();
    }
    if ( Enable::HCALIN_CELL ) {
        HCALInner_Cells();
    } 
    if ( Enable::HCALOUT_CELL ) {
        HCALOuter_Cells();
    }

    // calo towers 
    if ( Enable::CEMC_TOWER ) {
        CEMC_Towers();
    }

    // epd tiles
    if ( Enable::EPD_TILE ) {
        EPD_Tiles();
    }

    // hcal towers
    if ( Enable::HCALIN_TOWER ) {
        HCALInner_Towers();
    }
    if ( Enable::HCALOUT_TOWER ) {
        HCALOuter_Towers();
    }

    // global reco
    if ( Enable::GLOBAL_RECO ) {
        Global_Reco();
    } else if ( Enable::GLOBAL_FASTSIM ){
        Global_FastSim();
    }

    // calo trigger
    // if ( Enable::CALOTRIGGER ) {
        // CaloTrigger_Sim();
    // }

    // ===================================================================
    // ============================= Analysis ============================
    // ===================================================================

    JetFlow::VERBOSITY = Enable::VERBOSITY;
    JetFlow::is_mc = true;
    JetFlow::has_truth_jets = true;

    JetFlow::do_event_selection = false;
    EventSelection::VERBOSITY = 1;
    EventSelection::do_zvrtx_cut = false;
    EventSelection::SetZertexCutRange(70,-70);

    EventSelection::do_min_bias_cut = false;
    EventSelection::do_truth_jet_cut = false;
    EventSelection::SetPtHardRange(10);

    EventSelection::do_tower_chi2_cut = false;
    EventSelection::tower_chi2_nodes = {
        "TOWERINFO_CALIB_CEMC",
        "TOWERINFO_CALIB_HCALIN",
        "TOWERINFO_CALIB_HCALOUT"
    };

    JetFlow::tower_info_prefix = "TOWERINFO_CALIB";
    JetFlow::calc_rho = true;
    JetBackground::retower_frac_cut = 0.5;
    JetBackground::do_area_rho = false;
    JetBackground::do_mult_rho = true;

    JetFlow::do_truth_jet_reco = true;

    JetFlow::do_iter_sub_jet_reco = false;
    JetBackground::do_flow = false;
    JetBackground::retower_frac_cut = 0.5;
    JetBackground::seed_jet_d = 3;
    JetBackground::seed_jet_pt = 7;

    JetFlow::do_unsub_jet_reco = true;

    JetFlow::SetJetParams({0.4});
    JetFlow::output_file_name = outputFile;

    RunJetFlow();
  
    // ===================================================================
    // ============================= Run4All =============================
    // ===================================================================
    
    InputManagers();

    if ( nEvents <= 0 ) { return 0; }
    
    se -> run( nEvents) ;
    se -> End();

    std::cout << "All done" << std::endl;
    delete se;

    // get the hell out of here
    gSystem->Exit(0);
    
    return 0;

}
#endif