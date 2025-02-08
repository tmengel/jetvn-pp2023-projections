/*!
 * \file LeadTruthJetCut.h
 * \brief LeadTruthJetCut: Cuts on the leading truth jet pT. This only is from MC truth information.
 * \author Tanner Mengel <tmengel@bnl.gov>
 * \version $Verison: 1.0.1 $
 * \date $Date: 09/10/2024 $
 */

#ifndef EVENTSELECTION_LEADTRUTHJETCUT_H
#define EVENTSELECTION_LEADTRUTHJETCUT_H

#include "EventCut.h"

class PHCompositeNode;

class LeadTruthJetCut  : public EventCut
{
  public:

    LeadTruthJetCut(float high, float low);
    ~LeadTruthJetCut() override {}

    void identify(std::ostream &os = std::cout) const override;
    bool operator()(PHCompositeNode* topNode) override;
    
};

#endif // EVENTSELECTION_LEADTRUTHJETCUT_H