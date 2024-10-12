#ifndef _qem_HPP
#define _qem_HPP

#include "HalfEdge.hpp"

int invers3x3(double mat[3][3], double inv[3][3]);
void calcQEM(Vertex* v, Face* f, double qem[3][3], double& dd);
int calcQEM(Solid* solid, Vertex* v, std::set<Face*>& faces, double qem[3][3], double normal[3]);
int calcVertexQEM(Solid* solid, std::vector<Face*>& faceList, Vertex* v1, Vertex* v2, double new_coord[3]);

#endif
