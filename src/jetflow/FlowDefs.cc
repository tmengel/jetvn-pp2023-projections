#include "FlowDefs.h"

#include <cmath>

FlowDefs::flow_key FlowDefs::gen_flow_key(const uint8_t harmonic, const uint8_t particles)
{
    return ((uint16_t) harmonic << FlowDefs::kBitShiftFlowKey) | ((uint16_t) particles & 0xFF);
}

uint8_t FlowDefs::get_harmonic(const FlowDefs::flow_key key)
{
    return (uint8_t) (key >> FlowDefs::kBitShiftFlowKey);
}

uint8_t FlowDefs::get_particles(const FlowDefs::flow_key key)
{
    return (uint8_t) key & 0xFF;
}


double FlowDefs::get_mag(const flow_vector& v)
{
    return sqrt(v.first * v.first + v.second * v.second);
}

double FlowDefs::get_real(const flow_vector& v)
{
    return v.first;
}

double FlowDefs::get_imag(const flow_vector& v)
{
    return v.second;
}

FlowDefs::flow_vector FlowDefs::get_conj(const flow_vector& v)
{
    return std::make_pair(v.first, -v.second);
}

FlowDefs::flow_vector FlowDefs::calc_Qvec(std::vector<double> &phi, uint8_t harmonic)
{
    double cos_sum = 0;
    double sin_sum = 0;
    double harm_double = (double) harmonic;
    for (unsigned int i = 0; i < phi.size(); i++) {
        cos_sum += std::cos(harm_double * phi[i]);
        sin_sum += std::sin(harm_double * phi[i]);
    }

    return std::make_pair(cos_sum, sin_sum);
}

