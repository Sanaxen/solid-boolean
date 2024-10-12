#ifndef _MESH_SIMPLFY_HPP

#define _MESH_SIMPLFY_HPP

#include "HalfEdge.hpp"
#include "EdgeCollapse.hpp"
#include "Smoothing.hpp"

MESHSIMPLIFICATION_DLLEXPORTS void dump_mesh_obj_faceColor(char* filename, MeshV& mesh, int* faceColor);

MESHSIMPLIFICATION_DLLEXPORTS int create_mesh_obj(const char *filename, MeshV& mesh);
MESHSIMPLIFICATION_DLLEXPORTS void save_mesh_obj(char* filename, MeshV& mesh);


MESHSIMPLIFICATION_DLLEXPORTS void* create_mesh_obj_p( char *filename);
MESHSIMPLIFICATION_DLLEXPORTS void save_mesh_obj_p(char* filename, void* mesh);


#endif