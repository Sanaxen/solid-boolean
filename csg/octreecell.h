#ifndef _OCTREE_CELL_H
#define _OCTREE_CELL_H

#include <vector>
#include "bitmacro.h"
#include "RayBox_def.h"
#include "openmp_config.h"
#include "sse_func.h"

#include <Windows.h>
#include "debug_print.h"

#define OCTREE_CELL_TOL	(0.001f)
extern float OCTREE_CELL_TOL_value;

int triBoxOverlap(float boxcenter[3],float boxhalfsize[3],float triverts[3][3]);

#define FLOAT_DATA_T	float
#define FLOAT_CAST(x)	static_cast<FLOAT_DATA_T>(x)

#define EPSILON 0.000001

#define CROSS(dest,v1,v2) \
          dest[0]=v1[1]*v2[2]-v1[2]*v2[1]; \
          dest[1]=v1[2]*v2[0]-v1[0]*v2[2]; \
          dest[2]=v1[0]*v2[1]-v1[1]*v2[0];
#define DOT(v1,v2) (v1[0]*v2[0]+v1[1]*v2[1]+v1[2]*v2[2])
#define SUB(dest,v1,v2) \
          dest[0]=v1[0]-v2[0]; \
          dest[1]=v1[1]-v2[1]; \
          dest[2]=v1[2]-v2[2]; 

/* the original jgt code */
inline int intersect_triangle_(FLOAT_DATA_T orig[3], FLOAT_DATA_T dir[3],
	FLOAT_DATA_T vert0[3], FLOAT_DATA_T vert1[3], FLOAT_DATA_T vert2[3],
	FLOAT_DATA_T *t, FLOAT_DATA_T *u, FLOAT_DATA_T *v)
{
	FLOAT_DATA_T edge1[3], edge2[3], tVector3d[3], pVector3d[3], qVector3d[3];
	FLOAT_DATA_T det, inv_det;

	/* find Vector3dtors for two edges sharing vert0 */
	SUB(edge1, vert1, vert0);
	SUB(edge2, vert2, vert0);

	/* begin calculating determinant - also used to calculate U parameter */
	CROSS(pVector3d, dir, edge2);

	/* if determinant is near zero, ray lies in plane of triangle */
	det = DOT(edge1, pVector3d);

	if (det > -EPSILON && det < EPSILON)
		return 0;

	inv_det = FLOAT_CAST(1.0) / det;
	/* calculate distance from vert0 to ray origin */
	SUB(tVector3d, orig, vert0);

	/* calculate U parameter and test bounds */
	*u = DOT(tVector3d, pVector3d) * inv_det;
	if (*u < FLOAT_CAST(0.0) || *u > FLOAT_CAST(1.0))
		return 0;

	/* prepare to test V parameter */
	CROSS(qVector3d, tVector3d, edge1);

	/* calculate V parameter and test bounds */
	*v = DOT(dir, qVector3d) * inv_det;
	if (*v < 0.0 || *u + *v > 1.0)
		return 0;

	/* calculate t, ray intersects triangle */
	*t = DOT(edge2, qVector3d) * inv_det;

	return 1;
}


/* code rewritten to do tests on the sign of the determinant */
/* the division is at the end in the code                    */
inline int intersect_triangle1_(FLOAT_DATA_T orig[3], FLOAT_DATA_T dir[3],
	FLOAT_DATA_T vert0[3], FLOAT_DATA_T vert1[3], FLOAT_DATA_T vert2[3],
	FLOAT_DATA_T *t, FLOAT_DATA_T *u, FLOAT_DATA_T *v)
{
	FLOAT_DATA_T edge1[3], edge2[3], tVector3d[3], pVector3d[3], qVector3d[3];
	FLOAT_DATA_T det, inv_det;

	/* find Vector3dtors for two edges sharing vert0 */
	SUB(edge1, vert1, vert0);
	SUB(edge2, vert2, vert0);

	/* begin calculating determinant - also used to calculate U parameter */
	CROSS(pVector3d, dir, edge2);

	/* if determinant is near zero, ray lies in plane of triangle */
	det = DOT(edge1, pVector3d);

	if (det > EPSILON)
	{
		/* calculate distance from vert0 to ray origin */
		SUB(tVector3d, orig, vert0);

		/* calculate U parameter and test bounds */
		*u = DOT(tVector3d, pVector3d);
		if (*u < FLOAT_CAST(0.0) || *u > det)
			return 0;

		/* prepare to test V parameter */
		CROSS(qVector3d, tVector3d, edge1);

		/* calculate V parameter and test bounds */
		*v = DOT(dir, qVector3d);
		if (*v < FLOAT_CAST(0.0) || *u + *v > det)
			return 0;

	}
	else if (det < -EPSILON)
	{
		/* calculate distance from vert0 to ray origin */
		SUB(tVector3d, orig, vert0);

		/* calculate U parameter and test bounds */
		*u = DOT(tVector3d, pVector3d);
		/*      printf("*u=%f\n",(float)*u); */
		/*      printf("det=%f\n",det); */
		if (*u > FLOAT_CAST(0.0) || *u < det)
			return 0;

		/* prepare to test V parameter */
		CROSS(qVector3d, tVector3d, edge1);

		/* calculate V parameter and test bounds */
		*v = DOT(dir, qVector3d);
		if (*v > FLOAT_CAST(0.0) || *u + *v < det)
			return 0;
	}
	else return 0;  /* ray is parallell to the plane of the triangle */


	inv_det = FLOAT_CAST(1.0) / det;

	/* calculate t, ray intersects triangle */
	*t = DOT(edge2, qVector3d) * inv_det;
	(*u) *= inv_det;
	(*v) *= inv_det;

	return 1;
}

/* code rewritten to do tests on the sign of the determinant */
/* the division is before the test of the sign of the det    */
inline int intersect_triangle2_(FLOAT_DATA_T orig[3], FLOAT_DATA_T dir[3],
	FLOAT_DATA_T vert0[3], FLOAT_DATA_T vert1[3], FLOAT_DATA_T vert2[3],
	FLOAT_DATA_T *t, FLOAT_DATA_T *u, FLOAT_DATA_T *v)
{
	FLOAT_DATA_T edge1[3], edge2[3], tVector3d[3], pVector3d[3], qVector3d[3];
	FLOAT_DATA_T det, inv_det;

	/* find Vector3dtors for two edges sharing vert0 */
	SUB(edge1, vert1, vert0);
	SUB(edge2, vert2, vert0);

	/* begin calculating determinant - also used to calculate U parameter */
	CROSS(pVector3d, dir, edge2);

	/* if determinant is near zero, ray lies in plane of triangle */
	det = DOT(edge1, pVector3d);

	/* calculate distance from vert0 to ray origin */
	SUB(tVector3d, orig, vert0);

	if (det > EPSILON)
	{
		/* calculate U parameter and test bounds */
		*u = DOT(tVector3d, pVector3d);
		if (*u < FLOAT_CAST(0.0) || *u > det)
			return 0;

		/* prepare to test V parameter */
		CROSS(qVector3d, tVector3d, edge1);

		/* calculate V parameter and test bounds */
		*v = DOT(dir, qVector3d);
		if (*v < FLOAT_CAST(0.0) || *u + *v > det)
			return 0;

	}
	else if (det < -EPSILON)
	{
		/* calculate U parameter and test bounds */
		*u = DOT(tVector3d, pVector3d);
		if (*u > FLOAT_CAST(0.0) || *u < det)
			return 0;

		/* prepare to test V parameter */
		CROSS(qVector3d, tVector3d, edge1);

		/* calculate V parameter and test bounds */
		*v = DOT(dir, qVector3d);
		if (*v > FLOAT_CAST(0.0) || *u + *v < det)
			return 0;
	}
	else return 0;  /* ray is parallell to the plane of the triangle */

	inv_det = FLOAT_CAST(1.0) / det;
	
	/* calculate t, ray intersects triangle */
	*t = DOT(edge2, qVector3d) * inv_det;
	(*u) *= inv_det;
	(*v) *= inv_det;

	return 1;
}

/* code rewritten to do tests on the sign of the determinant */
/* the division is before the test of the sign of the det    */
/* and one CROSS has been moved out from the if-else if-else */
inline int intersect_triangle3_(const FLOAT_DATA_T orig[3], const FLOAT_DATA_T dir[3],
	const FLOAT_DATA_T vert0[3], const FLOAT_DATA_T vert1[3], const FLOAT_DATA_T vert2[3],
	FLOAT_DATA_T *t, FLOAT_DATA_T *u, FLOAT_DATA_T *v)
{
	FLOAT_DATA_T edge1[3], edge2[3], tVector3d[3], pVector3d[3], qVector3d[3];
	FLOAT_DATA_T det, inv_det;

	/* find Vector3dtors for two edges sharing vert0 */
	SUB(edge1, vert1, vert0);
	SUB(edge2, vert2, vert0);

	/* begin calculating determinant - also used to calculate U parameter */
	CROSS(pVector3d, dir, edge2);

	/* if determinant is near zero, ray lies in plane of triangle */
	det = DOT(edge1, pVector3d);

	/* calculate distance from vert0 to ray origin */
	SUB(tVector3d, orig, vert0);
	inv_det = FLOAT_CAST(1.0) / det;

	CROSS(qVector3d, tVector3d, edge1);

	if (det > EPSILON)
	{
		*u = DOT(tVector3d, pVector3d);
		if (*u < FLOAT_CAST(0.0) || *u > det)
			return 0;

		/* calculate V parameter and test bounds */
		*v = DOT(dir, qVector3d);
		if (*v < FLOAT_CAST(0.0) || *u + *v > det)
			return 0;

	}
	else if (det < -EPSILON)
	{
		/* calculate U parameter and test bounds */
		*u = DOT(tVector3d, pVector3d);
		if (*u > FLOAT_CAST(0.0) || *u < det)
			return 0;

		/* calculate V parameter and test bounds */
		*v = DOT(dir, qVector3d);
		if (*v > FLOAT_CAST(0.0) || *u + *v < det)
			return 0;
	}
	else return 0;  /* ray is parallell to the plane of the triangle */

	*t = DOT(edge2, qVector3d) * inv_det;
	(*u) *= inv_det;
	(*v) *= inv_det;

	return 1;
}

inline int intersect_triangle3a_(const FLOAT_DATA_T orig[3], const FLOAT_DATA_T dir[3],
	const FLOAT_DATA_T vert0[3], const FLOAT_DATA_T vert1[3], const FLOAT_DATA_T vert2[3],
	FLOAT_DATA_T *t, FLOAT_DATA_T *u, FLOAT_DATA_T *v)
{
	FLOAT_DATA_T edge1[3], edge2[3], tVector3d[3], pVector3d[3], qVector3d[3];
	FLOAT_DATA_T det, inv_det;

	/* find Vector3dtors for two edges sharing vert0 */
	SUB(edge1, vert1, vert0);
	SUB(edge2, vert2, vert0);

	/* begin calculating determinant - also used to calculate U parameter */
	CROSS(pVector3d, dir, edge2);

	/* if determinant is near zero, ray lies in plane of triangle */
	det = DOT(edge1, pVector3d);
	//if (det > -EPSILON && det < EPSILON) return 0;  /* ray is parallell to the plane of the triangle */
	if (det > -1.0E-12 && det < 1.0E-12) return 0;  /* ray is parallell to the plane of the triangle */

	/* calculate distance from vert0 to ray origin */
	SUB(tVector3d, orig, vert0);
	inv_det = FLOAT_CAST(1.0) / det;

	CROSS(qVector3d, tVector3d, edge1);

	*u = DOT(tVector3d, pVector3d);
	*v = DOT(dir, qVector3d);


	*t = DOT(edge2, qVector3d) * inv_det;
	(*u) *= inv_det;
	(*v) *= inv_det;

	return 1;
}

class Face;
class FaceSet;
class Object3D;

#ifdef USE_OPENMP
#define NODE_MAX_DEPTH 7
#else
#define NODE_MAX_DEPTH 5
#endif

class octreecell
{
	// Octree code
	struct Node {
		Node* subnodes[8];

		ALIGN16 float boxcenter[3];
		ALIGN16 float boxhalfsize[3];

		std::vector<Face*> triangles;
		int level;
		inline Node()
		{
			subnodes[0] = NULL;
			subnodes[1] = NULL;
			subnodes[2] = NULL;
			subnodes[3] = NULL;
			subnodes[4] = NULL;
			subnodes[5] = NULL;
			subnodes[6] = NULL;
			subnodes[7] = NULL;
		}
	};


	inline void __fastcall buildoctree(int flag, Node *root, const float halfwidth, const float centerpt[3], std::vector<Face*>& triangles, float MAXNUMTRIS = 10, float MINHALFWIDTH = 0.001)
	{
		float *boxcenter   = root->boxcenter;
		float *boxhalfsize = root->boxhalfsize;
		boxcenter[0]   = centerpt[0];
		boxcenter[1]   = centerpt[1];
		boxcenter[2]   = centerpt[2];
		boxhalfsize[0] = boxhalfsize[1] = boxhalfsize[2] = halfwidth;

		if (triangles.size() < MAXNUMTRIS || halfwidth < MINHALFWIDTH || root->level == 1)
		{
			// build a leaf node
			//for (int ctr = 0; ctr < 8; ctr++) root->subnodes[ctr] = NULL;
			//root->triangles = triangles;
			
			std::vector<Face*>::iterator it = triangles.begin();
			std::vector<Face*>::iterator iend = triangles.end();

			std::vector<Face*>* root_tri = &(root->triangles);
			for ( ; it != iend; ++it)
			{
				root_tri->push_back((*it));
			}

			return; 
		}
    
		const ALIGN16 float halfwidth_half     = halfwidth*0.5f;
		const ALIGN16 float halfwidth_half_tol = halfwidth_half+OCTREE_CELL_TOL;
		const ALIGN16 float boxhalfsize_half[3]     = {halfwidth_half_tol, halfwidth_half_tol, halfwidth_half_tol};

		const ALIGN16 float dx[2] = {boxhalfsize_half[0]+OCTREE_CELL_TOL, boxhalfsize_half[0]-OCTREE_CELL_TOL};
		const ALIGN16 float dy[2] = {boxhalfsize_half[1]+OCTREE_CELL_TOL, boxhalfsize_half[1]-OCTREE_CELL_TOL};
		const ALIGN16 float dz[2] = {boxhalfsize_half[2]+OCTREE_CELL_TOL, boxhalfsize_half[2]-OCTREE_CELL_TOL};

		for (int nodectr = 0; nodectr < 8; nodectr++)
		{
			std::vector<Face*> temptriangles;

			ALIGN16 float plusorminus[3] = {-1.0f, -1.0f, -1.0f};

			if (nodectr & 1) plusorminus[0] = 1.0f;
			if (nodectr & 2) plusorminus[1] = 1.0f;
			if (nodectr & 4) plusorminus[2] = 1.0f;

			const ALIGN16 float boxcenter[3] =
			{
				centerpt[0] + plusorminus[0]*halfwidth_half,
				centerpt[1] + plusorminus[1]*halfwidth_half,
				centerpt[2] + plusorminus[2]*halfwidth_half
			};


#if 0
			float triverts[3][3];

			for (int ctr = 0; ctr < triangles.size(); ctr++)
			{
				Face* face = triangles[ctr];
				Vertex* vrt1 = face->getVertex1();
				Vertex* vrt2 = face->getVertex2();
				Vertex* vrt3 = face->getVertex3();

				triverts[0][0] = to_double(vrt1->x);
				triverts[0][1] = to_double(vrt1->y);
				triverts[0][2] = to_double(vrt1->z);

				triverts[1][0] = to_double(vrt2->x);
				triverts[1][1] = to_double(vrt2->y);
				triverts[1][2] = to_double(vrt2->z);

				triverts[2][0] = to_double(vrt3->x);
				triverts[2][1] = to_double(vrt3->y);
				triverts[2][2] = to_double(vrt3->z);

				int isect = triBoxOverlap(boxcenter, boxhalfsize_half, triverts);
				if (isect) temptriangles.push_back(face);
			}
#else
			const ALIGN16 float maxA[3]={
				boxcenter[0]+dx[0],
				boxcenter[1]+dy[0],
				boxcenter[2]+dz[0]
			};
			const ALIGN16 float minA[3]={
				boxcenter[0]-dx[1],
				boxcenter[1]-dy[1],
				boxcenter[2]-dz[1]
			};


			const int sz = triangles.size();
			for (int ctr = 0; ctr < sz; ctr++)
			{
				Face* face = triangles[ctr];

				if ( flag && IS_NO_OVERLAP_FACE(face)) continue;
				const Bound* bound = face->getBound();
				if (( bound->xyz_Min[0]-OCTREE_CELL_TOL > maxA[0] ) ||
					( bound->xyz_Max[0]+OCTREE_CELL_TOL < minA[0] ) ||
					( bound->xyz_Min[1]-OCTREE_CELL_TOL > maxA[1] ) ||
					( bound->xyz_Max[1]+OCTREE_CELL_TOL < minA[1] ) ||
					( bound->xyz_Min[2]-OCTREE_CELL_TOL > maxA[2] ) ||
					( bound->xyz_Max[2]+OCTREE_CELL_TOL < minA[2] ) ) continue;
			
				temptriangles.push_back(face);
			}
#endif
			root->subnodes[nodectr] = new Node;
			root->subnodes[nodectr]->level = root->level/2;

			buildoctree(flag, root->subnodes[nodectr], halfwidth_half, boxcenter, temptriangles, MAXNUMTRIS, MINHALFWIDTH);
		}

	}

	// test if an AABB and another AABB intersect
	inline int __fastcall aabbaabb(const float boxcenter[3], const float boxhalfsize[3], const float boxcenter2[3], float boxhalfsize2[3])
	{
     
#if 0
		float distx = fabs(boxcenter2[0] - boxcenter[0]);
		float radsumx = boxhalfsize2[0] + boxhalfsize[0];
		if (distx > radsumx) return 0;

		float disty = fabs(boxcenter2[1] - boxcenter[1]);
		float radsumy = boxhalfsize2[1] + boxhalfsize[1];
		if (disty > radsumy) return 0;

		float distz = fabs(boxcenter2[2] - boxcenter[2]);
		float radsumz = boxhalfsize2[2] + boxhalfsize[2];
		if (distz > radsumz) return 0;
#else
		if ((fabs(boxcenter2[0] - boxcenter[0]) > boxhalfsize2[0] + boxhalfsize[0]) ||
			(fabs(boxcenter2[1] - boxcenter[1]) > boxhalfsize2[1] + boxhalfsize[1]) ||
			(fabs(boxcenter2[2] - boxcenter[2]) > boxhalfsize2[2] + boxhalfsize[2]) ) return 0;
#endif
		return 1;
	}

  // draw faces that intersect an AABB
  // this is only approximate - for it to be completely accurate,
  // if there's only a partial intersection with a leaf node, we 
  // should check each triangle with the AABB
	__forceinline void __fastcall getfaces(Node *root, const float boxcenter[3], const float boxhalfsize[3], std::vector<Face*>& facelist, int threadID)
	{

		int isect = aabbaabb(boxcenter, boxhalfsize, root->boxcenter, root->boxhalfsize);
		if (!isect) return;

		// leaf
		if (!root->subnodes[0])
		{
			std::vector<Face*>::iterator it = root->triangles.begin();
			std::vector<Face*>::iterator iend = root->triangles.end();

			for ( ; it != iend; ++it)
			{
				if ( (*it)->tmp[threadID] /*|| IS_NO_OVERLAP_FACE((*it))*/ )
				{
					continue;
				}
				//if (IS_NO_OVERLAP_FACE((*it)) )
				//{
				//	printf("この条件は今は不要のはず\n");
				//	continue;
				//}

				(*it)->tmp[threadID] = 1;
				facelist.push_back((*it));
			}
			return;                      
		}
             
		//for (int nodectr = 0; nodectr < 8; nodectr++)
		//{
		//	getfaces(root->subnodes[nodectr], boxcenter, boxhalfsize, facelist);
		//}
		getfaces(root->subnodes[0], boxcenter, boxhalfsize, facelist, threadID);
		getfaces(root->subnodes[1], boxcenter, boxhalfsize, facelist, threadID);
		getfaces(root->subnodes[2], boxcenter, boxhalfsize, facelist, threadID);
		getfaces(root->subnodes[3], boxcenter, boxhalfsize, facelist, threadID);
		getfaces(root->subnodes[4], boxcenter, boxhalfsize, facelist, threadID);
		getfaces(root->subnodes[5], boxcenter, boxhalfsize, facelist, threadID);
		getfaces(root->subnodes[6], boxcenter, boxhalfsize, facelist, threadID);
		getfaces(root->subnodes[7], boxcenter, boxhalfsize, facelist, threadID);
	}    

	__forceinline void __fastcall getfaces_itfRay(Node *root, float org[3], float dir[3], std::vector<Face*>& facelist, int threadID)
	{
		const ALIGN16 float minB[3]={
			root->boxcenter[0] - root->boxhalfsize[0] - RAYBOX_BOX_EXT_TOL_value,
			root->boxcenter[1] - root->boxhalfsize[1] - RAYBOX_BOX_EXT_TOL_value,
			root->boxcenter[2] - root->boxhalfsize[2] - RAYBOX_BOX_EXT_TOL_value
		};
		const ALIGN16 float maxB[3]={
			root->boxcenter[0] + root->boxhalfsize[0] + RAYBOX_BOX_EXT_TOL_value,
			root->boxcenter[1] + root->boxhalfsize[1] + RAYBOX_BOX_EXT_TOL_value,
			root->boxcenter[2] + root->boxhalfsize[2] + RAYBOX_BOX_EXT_TOL_value
		};

		
		if ( ! RayBoxIntersection( minB,  maxB, org, dir) )
		{
			return;
		}

		// leaf
		if (!root->subnodes[0])
		{
			std::vector<Face*>::iterator it = root->triangles.begin();
			std::vector<Face*>::iterator iend = root->triangles.end();

#if 10
			const float eps = RAYBOX_BOX_EXT_TOL_value*0.8;
			for ( ; it != iend; ++it)
			{
#if 10
#if 0
				//Rayと三角形が十分平行では無いなら
				float n[3];
				(*it)->getNormal().toFloat(n);
				float dot = DOT(n, dir);
				//printf("%f %f\n", sqrt(DOT(n, n)), sqrt(DOT(dir, dir)));
				if (fabs(dot) < 0.99)
#endif
				{
					Bound b = (*it)->getBoundFast();

					//三角形の包含Ｂｏｘとチェックしてみる
					const ALIGN16 float minA[3] = {
						b.xyz_Min[0] - eps, 
						b.xyz_Min[1] - eps,
						b.xyz_Min[2] - eps
					};
					const ALIGN16 float maxA[3] = {
						b.xyz_Max[0] + eps,
						b.xyz_Max[1] + eps,
						b.xyz_Max[2] + eps
					};


					if (!RayBoxIntersection(minA, maxA, org, dir))
					{
						//printf("ray-tribox not intersection\n");
						//干渉しない
						continue;
					}
				}
#endif

				if ( (*it)->tmp[threadID] == 0 ) 
				{
					(*it)->tmp[threadID] = 1;
					facelist.push_back((*it));
				}
			}
#else
			for (; it != iend; ++it)
			{
				if ((*it)->tmp[threadID] == 0)
				{
					(*it)->tmp[threadID] = 1;


					
					float v1[3], v2[3], v3[3];
					(*it)->getVertex1()->toFloat(v1);
					(*it)->getVertex2()->toFloat(v2);
					(*it)->getVertex3()->toFloat(v3);

					float t, u, v;
					u = 0.0f;
					v = 0.0f;
					t = 0.0f;
					int stat = intersect_triangle3a_(org, dir, v1, v2, v3, &t, &u, &v);

					if (stat == 1)
					{
						stat = -1;
						if (u < 1.005 && u > -0.005 && u + v < 1.005 )
						{
							stat = 1;
						}
					}

					if (stat <= 0)
					{
						float n[3];
						(*it)->getNormal().toFloat(n);
						float dot = DOT(n, dir);
						//printf("%f %f\n", sqrt(DOT(n, n)), sqrt(DOT(dir, dir)));
						if (fabs(dot) > 0.9999)
						{
							stat = 1;
						}
						//printf("dot=%f\n", fabs(dot));

						if (stat == 0)
						{
							//printf("u:%f v:%f dot:%f\n", u, v, dot);
						}
					}

					if (stat == 1)
					{
						facelist.push_back((*it));
					}
				}
			}
#endif

			return;                      
		}
             
		getfaces_itfRay(root->subnodes[0], org, dir, facelist, threadID);
		getfaces_itfRay(root->subnodes[1], org, dir, facelist, threadID);
		getfaces_itfRay(root->subnodes[2], org, dir, facelist, threadID);
		getfaces_itfRay(root->subnodes[3], org, dir, facelist, threadID);
		getfaces_itfRay(root->subnodes[4], org, dir, facelist, threadID);
		getfaces_itfRay(root->subnodes[5], org, dir, facelist, threadID);
		getfaces_itfRay(root->subnodes[6], org, dir, facelist, threadID);
		getfaces_itfRay(root->subnodes[7], org, dir, facelist, threadID);
	}    

	__forceinline void __fastcall deleteoctree(Node *root)
	{
     
		// if there are leaves, delete them     
		if (root->subnodes[0])
		{
			//for (int nodectr = 0; nodectr < 8; nodectr++) deleteoctree(root->subnodes[nodectr]);
			deleteoctree(root->subnodes[0]);
			deleteoctree(root->subnodes[1]);
			deleteoctree(root->subnodes[2]);
			deleteoctree(root->subnodes[3]);
			deleteoctree(root->subnodes[4]);
			deleteoctree(root->subnodes[5]);
			deleteoctree(root->subnodes[6]);
			deleteoctree(root->subnodes[7]);
		}
		delete root;
     
	}


 public:

	inline octreecell()
	{
		octroot = new Node;
		octroot->level = (int)pow(2.0, NODE_MAX_DEPTH);
		//for (int nodectr = 0; nodectr < 8; nodectr++) octroot->subnodes[nodectr] = NULL;
		octroot->subnodes[0] = NULL;
		octroot->subnodes[1] = NULL;
		octroot->subnodes[2] = NULL;
		octroot->subnodes[3] = NULL;
		octroot->subnodes[4] = NULL;
		octroot->subnodes[5] = NULL;
		octroot->subnodes[6] = NULL;
		octroot->subnodes[7] = NULL;
	}


	Node* octroot;

	inline ~octreecell(void)
	{
		deleteoctree(octroot);
	}

	inline void __fastcall buildoctree(Object3D* object, int flag)
	{
		int ts = GetTickCount();

		float halfwidth;
		Bound* bound = object->bound;
		const ALIGN16 float centerpt[3]={
			(bound->xyz_Max[0] + bound->xyz_Min[0])*0.5f,
			(bound->xyz_Max[1] + bound->xyz_Min[1])*0.5f,
			(bound->xyz_Max[2] + bound->xyz_Min[2])*0.5f
		};

		const ALIGN16 float len[3]={
			(bound->xyz_Max[0] - bound->xyz_Min[0])*0.5+OCTREE_CELL_TOL,
			(bound->xyz_Max[1] - bound->xyz_Min[1])*0.5+OCTREE_CELL_TOL,
			(bound->xyz_Max[2] - bound->xyz_Min[2])*0.5+OCTREE_CELL_TOL
		};

		halfwidth = len[0];
		if ( halfwidth < len[1] ) halfwidth = len[1];
		if ( halfwidth < len[2] ) halfwidth = len[2];
	  
		buildoctree(flag, octroot, halfwidth, centerpt, object->faces->m_pFaces);

		DEBUG_PRINT(("buildo ctree time %d ms\n", (GetTickCount() - ts)));

	}

	__forceinline void __fastcall getfaces( Object3D* object, Bound* bound, std::vector<Face*>& facelist, int threadID)
	{
		//FaceSet* fset = object->faces;

		//const int sz = fset->GetSize();
		//for ( int i = 0; i < sz; i++ )
		//{
		//	fset->GetFace(i)->tmp[threadID] = 0;
		//}
		getfaces( octroot, bound , facelist, threadID);
	}


	inline void __fastcall getfaces( Node* root, Bound* bound, std::vector<Face*>& facelist, int threadID)
	{
		const ALIGN16 float boxhalfsize[3]={
			((bound->xyz_Max[0]) - (bound->xyz_Min[0]))*0.5f+OCTREE_CELL_TOL,
			((bound->xyz_Max[1]) - (bound->xyz_Min[1]))*0.5f+OCTREE_CELL_TOL,
			((bound->xyz_Max[2]) - (bound->xyz_Min[2]))*0.5f+OCTREE_CELL_TOL
		};

		const ALIGN16 float boxcenter[3]={
			((bound->xyz_Max[0]) + (bound->xyz_Min[0]))*0.5f,
			((bound->xyz_Max[1]) + (bound->xyz_Min[1]))*0.5f,
			((bound->xyz_Max[2]) + (bound->xyz_Min[2]))*0.5f
		};

		getfaces(root, boxcenter, boxhalfsize, facelist, threadID);
	}

	__forceinline void __fastcall getfaces_itfRay(float org[3], float dir[3], std::vector<Face*>& facelist, int threadID)
	{
		getfaces_itfRay(octroot, org, dir, facelist, threadID);
	}
};

#endif
