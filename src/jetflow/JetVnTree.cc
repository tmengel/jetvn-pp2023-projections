#include "JetVnTree.h"

#include <fun4all/Fun4AllReturnCodes.h>
#include <fun4all/PHTFileServer.h>

#include <phool/PHCompositeNode.h>
#include <phool/getClass.h>
#include <phool/phool.h> 

#include <cdbobjects/CDBTTree.h>
#include <cdbobjects/CDBHistos.h>
#include <cdbobjects/CDBTTree.h>
#include <ffamodules/CDBInterface.h>

#include <phhepmc/PHGenIntegral.h>  // for PHGenIntegral
#include <phhepmc/PHGenIntegralv1.h>

//jet base
#include <jetbase/Jet.h>
#include <jetbase/Jetv2.h>
#include <jetbase/JetContainer.h>
#include <jetbase/JetContainerv1.h>

// jet background
#include <jetbackground/TowerRho.h>
#include <jetbackground/TowerRhov1.h>

// calobase
#include <calobase/TowerInfo.h>
#include <calobase/TowerInfoContainer.h>
#include <calobase/TowerInfoDefs.h>

#include <globalvertex/GlobalVertex.h>
#include <globalvertex/GlobalVertexMap.h>
// epd
#include <epd/EpdGeom.h>

// root includes
#include <TTree.h>

// standard includes
#include <iostream>
#include <string>
#include <utility>
#include <cstdlib>  
#include <array>
#include <vector>



JetVnTree::JetVnTree(const std::string & name)
  : SubsysReco(name)
{
}

int JetVnTree::Init(PHCompositeNode * /*topNode*/ )
{

  // std::string calibdir = CDBInterface::instance()->getUrl("sEPD_NMIP_CALIB");
  // if ( !calibdir.empty() ) { cdbttree = new CDBTTree(calibdir);
  // } else {
  //   std::cout << PHWHERE << "No sEPD mapping file for domain sEPD_NMIP_CALIB found" << std::endl;
  //   exit(1);
  // }
  // m_vkey.clear();
  // for (int i = 0; i < 768; i++) {

  //   int keymap = cdbttree->GetIntValue(i, "sepd_calib");
  //   if (keymap == 999) { continue;}
  //   unsigned int key = TowerInfoDefs::encode_epd(keymap);
  //   m_vkey.push_back(key);
  // }


  PHTFileServer::get().open(m_output_filename, "RECREATE");

  m_tree = new TTree("T", "JetVnTree");
  m_tree->Branch("event_id", &m_event_id, "event_id/I");
  m_tree->Branch("mult_rho", &m_mult_rho, "mult_rho/F");
  m_tree->Branch("ref_tower_e", &m_ref_tower_e);
  m_tree->Branch("ref_tower_phi", &m_ref_tower_phi);
  m_tree->Branch("ref_tower_eta", &m_ref_tower_eta);
  m_tree->Branch("truth_jet_pt", &m_truth_jet_pt);
  m_tree->Branch("truth_jet_phi", &m_truth_jet_phi);
  m_tree->Branch("truth_jet_eta", &m_truth_jet_eta);
  m_tree->Branch("reco_jet_pt", &m_reco_jet_pt);
  m_tree->Branch("reco_jet_phi", &m_reco_jet_phi);
  m_tree->Branch("reco_jet_eta", &m_reco_jet_eta);
  m_tree->Branch("reco_jet_nclustered", &m_reco_jet_ntowers);

  if ( m_is_pythia8 ){
    m_mc_tree = new TTree("MC", "JetVnTree_MC");
    m_mc_tree->Branch("N_truth_jets", &m_N_truth_jets, "N_truth_jets/i");
    m_mc_tree->Branch("N_accepted_event", &m_N_accepted_event, "N_accepted_event/i");
    m_mc_tree->Branch("N_processed_event", &m_N_processed_event, "N_processed_event/i");
    m_mc_tree->Branch("Sum_Of_Weight", &m_Sum_Of_Weight, "Sum_Of_Weight/D");
    m_mc_tree->Branch("Integrated_Lumi", &m_Integrated_Lumi, "Integrated_Lumi/D");
  }

  // set event counter
  m_event_id = -1;
  return Fun4AllReturnCodes::EVENT_OK;
}

int JetVnTree::process_event(PHCompositeNode *topNode)
{
  // increment event ID
  m_event_id++;

  auto vertexmap = findNode::getClass<GlobalVertexMap>(topNode, "GlobalVertexMap");
  if ( !vertexmap ) {
    std::cout << PHWHERE << "::ERROR - cannot find GlobalVertexMap node" << std::endl;
    exit(-1);
  }

  if (vertexmap->empty()) {
    if (Verbosity() > 0) {
      std::cout << PHWHERE << "::INFO - vertex map is empty" << std::endl;
    }
    return Fun4AllReturnCodes::ABORTEVENT;
  }

  // first grab the event vertex or bail
  auto vtx = vertexmap->begin()->second;
  float vtxz = NAN;
  if (vtx) {
    vtxz = vtx->get_z();
  } else {
    if (Verbosity() > 0) {
      std::cout << PHWHERE << "::INFO - vertex map is empty" << std::endl;
    }
    return Fun4AllReturnCodes::ABORTEVENT;
  }
  if (std::isnan(vtxz) || std::abs(vtxz) > 1e3) {
    std::cout << PHWHERE << "::ERROR - vertex is NAN" << std::endl;
    vtxz = 0.0;

  } 

  // mult rho
  auto towerrho = findNode::getClass<TowerRhov1>(topNode, m_mult_rho_node);
  if ( !towerrho ) {
    std::cout << PHWHERE << "::ERROR - cannot find " << m_mult_rho_node << std::endl;
    exit(-1);
  }

  // get reference towers
  auto epd_towerinfo = findNode::getClass<TowerInfoContainer>(topNode, "TOWERINFO_CALIB_EPD");
  if ( !epd_towerinfo ) {
    std::cout << PHWHERE << "::ERROR - cannot find TOWERINFO_CALIB_EPD" << std::endl;
    exit(-1);
  }
  auto epdgeom = findNode::getClass<EpdGeom>(topNode, "TOWERGEOM_EPD");
  if ( !epdgeom ) {
    std::cout << PHWHERE << "::ERROR - cannot find TOWERGEOM_EPD" << std::endl;
    exit(-1);
  }
    
  // get truth jets
  auto tjs = findNode::getClass<JetContainer>(topNode, m_truth_jet_input_node);
  if ( !tjs ) {
    std::cout << PHWHERE << "::ERROR - cannot find truth jet nodes " << m_truth_jet_input_node << std::endl;
    exit(-1);
  }

  // get raw jets
  auto rjs = findNode::getClass<JetContainer>(topNode, m_reco_jet_input_node);
  if ( !rjs ) {
    std::cout << PHWHERE << "::ERROR - cannot find raw jet nodes " << m_reco_jet_input_node << std::endl;
    exit(-1);
  }


  // get mult rho
  m_mult_rho = towerrho->get_rho();
  
  // get reference towers
  unsigned int ntowers = epd_towerinfo->size();
  std::cout << "JetVnTree::process_event - Found " << ntowers << " reference towers" << std::endl;
  for (unsigned int ch = 0; ch < ntowers; ch++) {
    auto tower = epd_towerinfo->get_tower_at_channel(ch);
    float epd_e = tower->get_energy();
    // float time = tower->get_time_float();
    unsigned int key = TowerInfoDefs::encode_epd(ch);
    if ( epd_e > 0.5) {
      // unsigned int this_key = epd_towerinfo->encode_key(ch);
      // int ieta = epd_towerinfo->getTowerEtaBin(ch);
      // int iphi = epd_towerinfo->getTowerPhiBin(ch);
      // RawTowerDefs::CalorimeterId::HCALOUT
      // float tile_phi = epdgeom->get_phi(m_vkey[ch]);
      // float tile_r = epdgeom->get_r(m_vkey[ch]);
      // float tile_z = epdgeom->get_z(m_vkey[ch]);
      float tile_phi = epdgeom->get_phi(key);
      float tile_r = epdgeom->get_r(key);
      float tile_z = epdgeom->get_z(key);
      tile_z-=vtxz;
      float tile_eta = std::asinh(tile_z/tile_r);

      float truncated_e =  (epd_e < 6.0) ? epd_e : 6.0;
      m_ref_tower_e.push_back(truncated_e);
      m_ref_tower_phi.push_back(tile_phi);
      m_ref_tower_eta.push_back(tile_eta);
    } else {
      // std::cout << "JetVnTree::process_event - Skipping tower with key " << key << " and energy " << epd_e << " and time " << time << std::endl;
    }
  } // end loop over reference towers

  // get truth jets
  for (auto tj: *tjs)
  {  
    float t_pt = tj->get_pt();
    float t_eta = tj->get_eta();
    float t_phi = tj->get_phi();

    if( std::fabs(t_eta) > m_truth_jet_eta_cut ) { continue; }
    if ( t_pt < m_truth_jet_min_pT ) { continue; }

    m_truth_jet_pt.push_back(t_pt);
    m_truth_jet_phi.push_back(t_phi);
    m_truth_jet_eta.push_back(t_eta);

    m_N_truth_jets++;
  
  }

  // get raw jets
  for (auto rj: *rjs)
  { 

    float r_pt = rj->get_pt();
    float r_eta = rj->get_eta();
    float r_phi = rj->get_phi();
    if ( r_pt < 1.0 ) { continue; }
    int r_ntowers = rj->num_comp();
    m_reco_jet_pt.push_back(r_pt);
    m_reco_jet_phi.push_back(r_phi);
    m_reco_jet_eta.push_back(r_eta);
    m_reco_jet_ntowers.push_back(r_ntowers);
  
  }



  if( m_is_pythia8 ){
    GetStats(topNode);
    m_mc_tree->Fill();
  }

  // fill tree
  m_tree->Fill();

  return Fun4AllReturnCodes::EVENT_OK;
}

void JetVnTree::GetStats(PHCompositeNode *topNode)
{
  PHNodeIterator iter(topNode);
  PHCompositeNode *sumNode = dynamic_cast<PHCompositeNode *>(iter.findFirst("PHCompositeNode", "RUN"));
  if (!sumNode)
  {
    std::cout << PHWHERE << "RUN Node missing doing nothing" << std::endl;
    exit(1);
  }

  PHGenIntegral * m_IntegralNode = findNode::getClass<PHGenIntegral>(sumNode, "PHGenIntegral");
  if( !m_IntegralNode ) {
    std::cout << PHWHERE << "PHGenIntegral Node missing doing nothing" << std::endl;
    exit(1);
  }
    
  m_Sum_Of_Weight = m_IntegralNode->get_Sum_Of_Weight();
  m_Integrated_Lumi = m_IntegralNode->get_Integrated_Lumi();
  m_N_accepted_event = m_IntegralNode->get_N_Generator_Accepted_Event();
  m_N_processed_event = m_IntegralNode->get_N_Processed_Event();

}


int JetVnTree::ResetEvent(PHCompositeNode * /*topNode*/)
{
  // reset event variables
  m_mult_rho = 0.0;
  m_ref_tower_phi.clear();
  m_ref_tower_e.clear();
  m_ref_tower_eta.clear();
  m_truth_jet_pt.clear();
  m_truth_jet_phi.clear();
  m_truth_jet_eta.clear();
  m_reco_jet_pt.clear();
  m_reco_jet_phi.clear();
  m_reco_jet_eta.clear();
  m_reco_jet_ntowers.clear();
  return Fun4AllReturnCodes::EVENT_OK;
}

int JetVnTree::End(PHCompositeNode * topNode)
{
  std::cout << "JetVnTree::End - Writing to " << m_output_filename << std::endl;
  PHTFileServer::get().cd(m_output_filename);
  PHTFileServer::get().write(m_output_filename);
  std::cout << "JetVnTree::End - Wrote to " << m_output_filename << std::endl;
  return Fun4AllReturnCodes::EVENT_OK;
}

