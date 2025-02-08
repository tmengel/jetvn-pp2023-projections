#include "TowerChi2Cut.h"

#include <phool/PHCompositeNode.h>
#include <phool/getClass.h>

// tower info
#include <calobase/TowerInfo.h>
#include <calobase/TowerInfoContainer.h>

#include <cstdlib>

TowerChi2Cut::TowerChi2Cut() : EventCut("TowerChi2Cut") 
{ 
    SetNodeNames({ "TOWERINFO_CALIB_CEMC", "TOWERINFO_CALIB_HCALIN", "TOWERINFO_CALIB_HCALOUT" }); // Set the default node names
}


void TowerChi2Cut::identify(std::ostream &os) const
{
    os << Name() + "::identify: " << std::endl;
    for (const auto &node_name : GetNodeNames())
    {
        os << "  Node name: " << node_name << std::endl;
    }
    return;
}

bool TowerChi2Cut::operator()(PHCompositeNode *topNode)
{

    for (const auto &node_name : GetNodeNames())
    {
        auto * towers = findNode::getClass<TowerInfoContainer>(topNode, node_name);
        if (!towers) {
            std::cerr << Name() + "::operator(PHCompositeNode *topNode) Could not find " << node_name << " node" << std::endl;
            exit(-1); // this is a fatal error
        }

        unsigned int nChannels = towers->size();
        for(unsigned int channel = 0; channel < nChannels; channel++){
            auto * tower = towers->get_tower_at_channel(channel);
            Passed(!tower->get_isBadChi2() || tower->get_isHot() || tower->get_isNoCalib());
            if (!Passed()){
                if(Verbosity()){
                    std::cout << Name() + "::operator(PHCompositeNode *topNode) Node " << node_name << " failed, channel " << channel << std::endl;
                }
                return Passed();
            }
        } 
    }

    return Passed();
    
}
