#ifndef _Smoothing_HPP
#define _Smoothing_HPP

#include "HalfEdge.hpp"

MESHSIMPLIFICATION_DLLEXPORTS int LaplacianSmoothing(Solid& solid, double dt, double minarea, double costh, double probability);
MESHSIMPLIFICATION_DLLEXPORTS int Simple_LaplacianSmoothing(Solid& solid, double dt, double alp, int iter, double th, double probability);

MESHSIMPLIFICATION_DLLEXPORTS int LaplacianSmoothing_p(void* solid, double dt, double minarea, double costh, double probability);
MESHSIMPLIFICATION_DLLEXPORTS int Simple_LaplacianSmoothing_p(void* solid, double dt, double alp, int iter, double th, double probability);

#endif