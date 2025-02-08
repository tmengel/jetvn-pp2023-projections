#ifndef JETFLOW_JETVNTREE_H
#define JETFLOW_JETVNTREE_H

#include <fun4all/SubsysReco.h>

#include <memory>
#include <string>
#include <utility>  // std::pair, std::make_pair
#include <vector>

class PHCompositeNode;
class TTree;
class CDBTTree;

class JetVnTree : public SubsysReco
{
public:

    JetVnTree( const std::string & name = "JetVnTree");
    ~JetVnTree() override {}

    void set_output_filename( const std::string & name ) { m_output_filename = name; }
   
    void set_reco_truth_jet_nodes(const std::string &raw, const std::string &truth)
    {
        m_reco_jet_input_node = raw;
        m_truth_jet_input_node = truth;
    }
    
    void add_mult_rho_node(const std::string &name)
    {
        m_mult_rho_node = name;
    }

    void set_max_eta( const float eta ) { m_max_eta = eta; }
    float get_max_eta() const { return m_max_eta; }

    void add_truth_jet_eta_cut(double abs_eta)
    {
        m_truth_jet_eta_cut = abs_eta;
    }
    
    void add_truth_jet_min_pT(double min_pT)
    {
        m_truth_jet_min_pT = min_pT;
    }

    void is_pythia8(bool is_pythia8)
    {
        m_is_pythia8 = is_pythia8;
    }


    int Init(PHCompositeNode *topNode) override;
    int process_event(PHCompositeNode *topNode) override;
    int ResetEvent(PHCompositeNode */*topNode*/) override;
    int End(PHCompositeNode * /*topNode*/) override;


private:

    // output file name
    std::string m_output_filename {"JetVnTree.root"};
    std::string m_reco_jet_input_node{""};
    std::string m_truth_jet_input_node{""};
    std::string m_mult_rho_node{""};

    // truth jet cuts
    float m_max_eta {1.1};
    double m_truth_jet_eta_cut {1.1};
    double m_truth_jet_min_pT {5.0};

    bool m_is_pythia8 {false};

    TTree * m_mc_tree { nullptr };
    unsigned int m_N_truth_jets {0};
    unsigned int m_N_accepted_event {0};
    unsigned int m_N_processed_event {0};
    double m_Sum_Of_Weight {0.0};
    double m_Integrated_Lumi {0.0};

    TTree * m_tree { nullptr };
    int m_event_id { -1 };
    float m_mult_rho{0.0};

    std::vector< float > m_ref_tower_phi {};
    std::vector< float > m_ref_tower_e {};
    std::vector< float > m_ref_tower_eta {};

    std::vector< float > m_truth_jet_pt {};
    std::vector< float > m_truth_jet_phi {};
    std::vector< float > m_truth_jet_eta {};

    std::vector<float> m_reco_jet_pt {};
    std::vector<float> m_reco_jet_phi {};
    std::vector<float> m_reco_jet_eta {};
    std::vector< int > m_reco_jet_ntowers {};

    std::vector<unsigned int> m_vkey {};
    CDBTTree *cdbttree{nullptr};

    void GetStats(PHCompositeNode *topNode);
    
};

#endif // JETFLOW_JETVNTREE_H
