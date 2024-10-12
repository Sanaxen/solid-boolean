#ifndef _FACE_CELL_H
#define _FACE_CELL_H

#include "Face.h"
#include <concurrent_vector.h>

class faceCell
{
public:
	//std::vector<Face*> faceList;
	Concurrency::concurrent_vector<Face*> faceList;
	Bound box;
};


#define FACE_GRID_NUM 8
class faceCellGrid
{
public:
	faceCellGrid(){}
	Bound box;
	faceCell cellList[FACE_GRID_NUM][FACE_GRID_NUM][FACE_GRID_NUM];
	faceCellGrid(Bound* bound)
	{
		box = *bound;
		float dx = (box.xyz_Max[0] - box.xyz_Min[0])/FACE_GRID_NUM;
		float dy = (box.xyz_Max[1] - box.xyz_Min[1])/FACE_GRID_NUM;
		float dz = (box.xyz_Max[2] - box.xyz_Min[2])/FACE_GRID_NUM;

		for ( int i = 0; i < FACE_GRID_NUM; i++ )
		{
			float ddx = box.xyz_Min[0] + i*dx;
			for ( int j = 0; j < FACE_GRID_NUM; j++ )
			{
				float ddy = box.xyz_Min[1] + j*dy;
				for ( int k = 0; k < FACE_GRID_NUM; k++ )
				{
					Bound* cellBox = &(cellList[i][j][k].box);
					cellBox->xyz_Min[0] = ddx;
					cellBox->xyz_Max[0] = cellBox->xyz_Min[0] + dx;

					cellBox->xyz_Min[1] = ddy
					cellBox->xyz_Max[1] = cellBox->xyz_Min[1] + dy;

					cellBox->xyz_Min[2] = box.xyz_Min[2] + k*dz;
					cellBox->xyz_Max[2] = cellBox->xyz_Min[2] + dz;
					cellBox->setbound = true;
				}
			}
		}
	}

	inline void Add(Face* face)
	{
		Bound* bound = face->getBound();
		for ( int i = 0; i < FACE_GRID_NUM; i++ )
		{
			if ( bound->xyz_Min[0]-Bound::TOL > cellList[i][0][0].box.xyz_Max[0] || bound->xyz_Max[0]+Bound::TOL < cellList[i][0][0].box.xyz_Min[0])
			{
				continue;
			}
			for ( int j = 0; j < FACE_GRID_NUM; j++ )
			{
				if ( bound->xyz_Min[1]-Bound::TOL > cellList[i][j][0].box.xyz_Max[1] || bound->xyz_Max[1]+Bound::TOL < cellList[i][j][0].box.xyz_Min[1])
				{
					continue;
				}
				for ( int k = 0; k < FACE_GRID_NUM; k++ )
				{
					if ( bound->xyz_Min[2]-Bound::TOL > cellList[i][j][k].box.xyz_Max[2] || bound->xyz_Max[2]+Bound::TOL < cellList[i][j][k].box.xyz_Min[2])
					{
						continue;
					}
					cellList[i][j][k].faceList.push_back(face);
				}
			}
		}
	}

	inline void GetItfList(Face* face, std::vector<Face*>& intersect_face)
	{
		Bound* bound = face->getBound();
		for ( int i = 0; i < FACE_GRID_NUM; i++ )
		{
			if ( bound->xyz_Min[0]-Bound::TOL > cellList[i][0][0].box.xyz_Max[0] || bound->xyz_Max[0]+Bound::TOL < cellList[i][0][0].box.xyz_Min[0])
			{
				continue;
			}
			for ( int j = 0; j < FACE_GRID_NUM; j++ )
			{
				if ( bound->xyz_Min[1]-Bound::TOL > cellList[i][j][0].box.xyz_Max[1] || bound->xyz_Max[1]+Bound::TOL < cellList[i][j][0].box.xyz_Min[1])
				{
					continue;
				}
				for ( int k = 0; k < FACE_GRID_NUM; k++ )
				{
					if ( bound->xyz_Min[2]-Bound::TOL > cellList[i][j][k].box.xyz_Max[2] || bound->xyz_Max[2]+Bound::TOL < cellList[i][j][k].box.xyz_Min[2])
					{
						continue;
					}
					const int sz = cellList[i][j][k].faceList.size();
					for ( int kk = 0; kk < sz; kk++ )
					{
						const int sz2 = intersect_face.size();
						int dup = 0;
						for ( int jj = 0; jj < sz2; jj++ )
						{
							if ( intersect_face[jj] == cellList[i][j][k].faceList[kk] )
							{
								dup = 1;
								break;
							}
						}
						if (!dup) intersect_face.push_back(cellList[i][j][k].faceList[kk]);
					}
				}
			}
		}
	}
};

#endif
