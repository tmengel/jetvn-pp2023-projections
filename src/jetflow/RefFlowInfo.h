#ifndef REFFLOWINFO_H
#define REFFLOWINFO_H

#include "FlowDefs.h"

#include <phool/PHObject.h>

#include <utility>
#include <vector>
#include <iostream>

class RefFlowInfo : public PHObject
{
 public:
    
    ~RefFlowInfo() override {}

    void identify(std::ostream& os = std::cout) const override {
        os << "RefFlowInfo base class" << std::endl;
    }

    PHObject* CloneMe() const override { return nullptr; }

    virtual void set_ref_key(  unsigned int /*nharm*/, unsigned int /*npart*/) { return; }
    virtual void set_ref_key(FlowDefs::flow_key /*key*/) { return; }
    FlowDefs::flow_key get_ref_key() const { return FlowDefs::FLOW_VOID_KEY; }

    virtual void set_qvector(FlowDefs::flow_vector /*qvec*/) { return; }
    virtual FlowDefs::flow_vector get_qvector() const { return std::make_pair(NAN, NAN); }

    virtual void set_mult(const unsigned int /*mult*/) { return; }
    virtual unsigned int get_mult() const { return 0; }

 protected:
    RefFlowInfo() {}

 private:
    ClassDefOverride(RefFlowInfo, 1);
};

#endif