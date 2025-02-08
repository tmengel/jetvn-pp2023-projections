/*!
 * \file TowerChi2Cut.h
 * \brief Cuts events based on the chi2 of the towers
 * \author Tanner Mengel <tmengel@bnl.gov>
 * \version $Verison: 1.0.1 $
 * \date $Date: 09/10/2024 $
 */

#ifndef EVENTSELECTION_TOWERCHI2CUT_H
#define EVENTSELECTION_TOWERCHI2CUT_H

#include "EventCut.h"

class PHCompositeNode;

class TowerChi2Cut  : public EventCut
{
  public:

    TowerChi2Cut();
    ~TowerChi2Cut() override {}

    void identify(std::ostream &os = std::cout) const override;
    bool operator()(PHCompositeNode* topNode) override;

};

#endif // EVENTSELECTION_TOWERCHI2CUT_H