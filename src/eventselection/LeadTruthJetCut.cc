#include "LeadTruthJetCut.h"

// phool includes
#include <phool/PHCompositeNode.h>
#include <phool/getClass.h>

// jet reco
#include <jetbase/Jetv1.h>
#include <jetbase/Jetv2.h>
#include <jetbase/JetContainerv1.h>

#include <cstdlib>

LeadTruthJetCut::LeadTruthJetCut(float high, float low) : EventCut("LeadTruthJetCut")
{
    SetRange(high, low);
    SetNodeName("AntiKt_Truth_r04"); // default node name
}

void LeadTruthJetCut::identify(std::ostream &os) const
{
    os << Name() + "::identify: " << std::endl;
    os << "  Node name: " << GetNodeName() << std::endl;
    os << "  pT range: " << GetRange().first << " < pT < " << GetRange().second << " GeV" << std::endl;
    return;
}

bool LeadTruthJetCut::operator()(PHCompositeNode *topNode)
{

    // get truth jet nodes
    auto * jets = findNode::getClass<JetContainerv1>(topNode, GetNodeName());
    if( !jets )
    {
        std::cerr << Name() + "::operator(PHCompositeNode *topNode) Could not find " + GetNodeName() << std::endl;
        exit(-1); // this is a fatal error
    }

    // get leading truth jet pT
    float max_pt = -1;
    for(auto jet: *jets){
        if(jet->get_pt() > max_pt){
            max_pt = jet->get_pt(); 
        }
    }
    
    Passed( (max_pt >= GetRange().first) && (max_pt <= GetRange().second) );
    if(!Passed() && Verbosity()){
       std::cout << Name() + "::operator(PHCompositeNode *topNode) Event failed" + Name() + ". Lead jet pT in " + GetNodeName() + ": " + std::to_string(max_pt) << std::endl;
    }

    return Passed();
}
