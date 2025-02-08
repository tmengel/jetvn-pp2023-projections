#ifndef REFFLOWMAP_H
#define REFFLOWMAP_H

#include "FlowDefs.h"

#include <phool/PHObject.h>
#include <iostream>
#include <vector>   

class RefFlowInfo;

class RefFlowMap : public PHObject
{
 public:

    typedef std::vector<RefFlowInfo*> RefFlowVec;

    ~RefFlowMap() override {}

    void identify(std::ostream& os = std::cout) const override { os << "RefFlowMap base class" << std::endl; }
    virtual bool empty() const {return true;}
    virtual void clear() {}

    virtual RefFlowInfo* get(FlowDefs::flow_key /*key*/) const { return nullptr; }

 protected:
    RefFlowMap() {}

 private:
    ClassDefOverride(RefFlowMap, 1);
};

#endif  // REFFLOWMAP_H