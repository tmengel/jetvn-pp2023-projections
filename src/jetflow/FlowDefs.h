#ifndef FLOWDEFS_H
#define FLOWDEFS_H

#include <cstdint>
#include <climits>
#include <vector>
#include <utility>

namespace FlowDefs 
{
    typedef uint16_t flow_key;
    static const FlowDefs::flow_key FLOW_VOID_KEY __attribute__((unused)) = UINT16_MAX;
    static const unsigned int kBitShiftFlowKey __attribute__((unused)) = 8;

    FlowDefs::flow_key gen_flow_key(const uint8_t harmonic, const uint8_t particles);
    uint8_t get_harmonic(const FlowDefs::flow_key key);
    uint8_t get_particles(const FlowDefs::flow_key key);

    typedef std::pair<double, double> flow_vector;
    
    double get_mag(const flow_vector& v);
    double get_real(const flow_vector& v);
    double get_imag(const flow_vector& v);

    flow_vector get_conj(const flow_vector& v);
    
    flow_vector calc_Qvec(std::vector<double> &phi, uint8_t harmonic);
}

#endif // FLOWDEFS_H