#ifndef _MESH_EDITOR_H
#define _MESH_EDITOR_H

#include "mesh_symplify.hpp"
#include "HalfEdge.hpp"
#include "Face.h"
#include "FaceList.h"
#include "octreecell.h"

#include "kdtree.hpp"

#include <stdlib.h>
#include <string>
#include <iostream>


class NGFace
{
public:
	MeshV* mesh;
	int faceid;
	bool connect[3];
	unsigned int edge[3][2];
	bool skipp;

	NGFace()
	{
		connect[0] = false;
		connect[1] = false;
		connect[2] = false;
		calcbox = false;
		mesh = NULL;
		skipp = false;
	}
	inline void Mesh(MeshV* meshPtr)
	{
		mesh = meshPtr;
	}

	bool calcbox;
	boundaryBox box;

	void getBox()
	{
		if ( calcbox )
		{
			return;
		}
		getFaceBox( mesh, faceid, &box);
		calcbox = true;
	}

	bool isOverlap(NGFace& face)
	{
		getBox();
		face.getBox();

		return isBoxOverlap(&box, &face.box);
	}

};

class mesh_editting_info
{
public:
	int merge_num1;
	int merge_num2;
	int split_num;

	mesh_editting_info()
	{
		merge_num1 = merge_num2 = split_num = 0;
	}
};


class T_JunctionSeam
{
	void mesh_min_max();
	void SeamFace(octreecell& octcell, std::vector<NGFace>& ngFaceList, int id, int edge_no, int pos, double tol, mesh_editting_info& edit_info);
	void SeamNGFace(octreecell& octcell, std::vector<NGFace>& ngFaceList, int id, double tol, mesh_editting_info& edit_info );
	bool findConnectFace(const unsigned int edge[2], std::vector<FaceS>& NeighborhoodFaces, int curid, std::vector<NGFace>& ngFaceList);
	void calcNormal(int id, double n[3]);

public:
	MeshV* mesh_;
	boundaryBox bbox;

	T_JunctionSeam(MeshV& mesh)
	{
		mesh_ = &mesh;
		mesh_min_max();
	}

	void AddVertex(double* pnt, double* nor, int color, int attr);
	void AddFace();
	void Seam(double connect_tol);
};

#endif