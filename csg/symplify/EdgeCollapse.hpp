#ifndef _EdgeCollapse_HPP
#define _EdgeCollapse_HPP

#include "HalfEdge.hpp"
#include "qem.hpp"

MESHSIMPLIFICATION_DLLEXPORTS int Simple_EdgeCollapse(int type, Solid& solid, double minarea, double costh, double pdf);
MESHSIMPLIFICATION_DLLEXPORTS int Simple_EdgeCollapseMain(Solid& solid, int type, double alp, int iter, double th, double pdf);

MESHSIMPLIFICATION_DLLEXPORTS int Simple_EdgeCollapse_p(int type, void* solid, double minarea, double costh, double pdf);
MESHSIMPLIFICATION_DLLEXPORTS int Simple_EdgeCollapseMain_p(void* solid, int type, double alp, int iter, double th, double pdf);

MESHSIMPLIFICATION_DLLEXPORTS double getFaceAreaMin(void* solid);
MESHSIMPLIFICATION_DLLEXPORTS double getFaceAreaMax(void* solid);
#endif
