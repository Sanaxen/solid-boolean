#ifndef _OCTREE_CELL_H
#define _OCTREE_CELL_H

#include <vector>
#include <Windows.h>

#define DEBUG_PRINT(s)	printf s

#define OCTREE_CELL_TOL	(0.0001)


#define NODE_MAX_DEPTH 4

#ifdef _linux // gcc
#define ALIGN(n)	__attribute__((aligned(n)))
#define ALIGN16 ALIGN(16)
#else // cl
#define ALIGN(n)	_declspec(align(n))
#define ALIGN16 ALIGN(16)
#endif


class octreecell
{
	// Octree code
	struct Node {
		Node* subnodes[8];

		ALIGN16 float boxcenter[3];
		ALIGN16 float boxhalfsize[3];

		std::vector<FaceS> triangles;
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


	inline void __fastcall buildoctree(int flag, Node *root, const float halfwidth, const float centerpt[3], std::vector<FaceS>& triangles, float MAXNUMTRIS = 10, float MINHALFWIDTH = 0.001)
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
			
			std::vector<FaceS>::iterator it = triangles.begin();
			std::vector<FaceS>::iterator iend = triangles.end();

			std::vector<FaceS>* root_tri = &(root->triangles);
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
			std::vector<FaceS> temptriangles;

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
				FaceS& face = triangles[ctr];
				face.getBox();
				const boundaryBox* bound = &face.box;
				if (( bound->min[0]-OCTREE_CELL_TOL > maxA[0] ) ||
					( bound->max[0]+OCTREE_CELL_TOL < minA[0] ) ||
					( bound->min[1]-OCTREE_CELL_TOL > maxA[1] ) ||
					( bound->max[1]+OCTREE_CELL_TOL < minA[1] ) ||
					( bound->min[2]-OCTREE_CELL_TOL > maxA[2] ) ||
					( bound->max[2]+OCTREE_CELL_TOL < minA[2] ) ) continue;
				temptriangles.push_back(face);
			}
			root->subnodes[nodectr] = new Node;
			root->subnodes[nodectr]->level = root->level/2;

			buildoctree(flag, root->subnodes[nodectr], halfwidth_half, boxcenter, temptriangles, MAXNUMTRIS, MINHALFWIDTH);
		}

	}

	// test if an AABB and another AABB intersect
	inline int __fastcall aabbaabb(const float boxcenter[3], const float boxhalfsize[3], const float boxcenter2[3], float boxhalfsize2[3])
	{
		if ((fabs(boxcenter2[0] - boxcenter[0]) > boxhalfsize2[0] + boxhalfsize[0]) ||
			(fabs(boxcenter2[1] - boxcenter[1]) > boxhalfsize2[1] + boxhalfsize[1]) ||
			(fabs(boxcenter2[2] - boxcenter[2]) > boxhalfsize2[2] + boxhalfsize[2]) ) return 0;
		return 1;
	}

  // draw faces that intersect an AABB
  // this is only approximate - for it to be completely accurate,
  // if there's only a partial intersection with a leaf node, we 
  // should check each triangle with the AABB
	__forceinline void __fastcall getfaces(Node *root, const float boxcenter[3], const float boxhalfsize[3], std::vector<FaceS>& facelist, int threadID)
	{

		int isect = aabbaabb(boxcenter, boxhalfsize, root->boxcenter, root->boxhalfsize);
		if (!isect) return;

		// leaf
		if (!root->subnodes[0])
		{
			std::vector<FaceS>::iterator it = root->triangles.begin();
			std::vector<FaceS>::iterator iend = root->triangles.end();

			for ( ; it != iend; ++it)
			{
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

	__forceinline void __fastcall delfaces(Node *root, const float boxcenter[3], const float boxhalfsize[3], FaceS face)
	{

		int isect = aabbaabb(boxcenter, boxhalfsize, root->boxcenter, root->boxhalfsize);
		if (!isect) return;

		// leaf
		if (!root->subnodes[0])
		{
			std::vector<FaceS>::iterator it = root->triangles.begin();

			for ( ; it != root->triangles.end();)
			{
				if ( (*it).id == face.id )
				{
					it = root->triangles.erase(it);
				}else
				{
					++it;
				}
			}
			return;                      
		}
             
		delfaces(root->subnodes[0], boxcenter, boxhalfsize, face);
		delfaces(root->subnodes[1], boxcenter, boxhalfsize, face);
		delfaces(root->subnodes[2], boxcenter, boxhalfsize, face);
		delfaces(root->subnodes[3], boxcenter, boxhalfsize, face);
		delfaces(root->subnodes[4], boxcenter, boxhalfsize, face);
		delfaces(root->subnodes[5], boxcenter, boxhalfsize, face);
		delfaces(root->subnodes[6], boxcenter, boxhalfsize, face);
		delfaces(root->subnodes[7], boxcenter, boxhalfsize, face);
	}    

	__forceinline void __fastcall addfaces(Node *root, const float boxcenter[3], const float boxhalfsize[3], FaceS face)
	{

		int isect = aabbaabb(boxcenter, boxhalfsize, root->boxcenter, root->boxhalfsize);
		if (!isect) return;

		// leaf
		if (!root->subnodes[0])
		{
			root->triangles.push_back( face );
			return;                      
		}
             
		addfaces(root->subnodes[0], boxcenter, boxhalfsize, face);
		addfaces(root->subnodes[1], boxcenter, boxhalfsize, face);
		addfaces(root->subnodes[2], boxcenter, boxhalfsize, face);
		addfaces(root->subnodes[3], boxcenter, boxhalfsize, face);
		addfaces(root->subnodes[4], boxcenter, boxhalfsize, face);
		addfaces(root->subnodes[5], boxcenter, boxhalfsize, face);
		addfaces(root->subnodes[6], boxcenter, boxhalfsize, face);
		addfaces(root->subnodes[7], boxcenter, boxhalfsize, face);
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

	inline void __fastcall buildoctree(FaceList* facelist, boundaryBox* bbox, int flag)
	{
		int ts = GetTickCount();

		float halfwidth;
		const ALIGN16 float centerpt[3]={
			(bbox->max[0] + bbox->min[0])*0.5f,
			(bbox->max[1] + bbox->min[1])*0.5f,
			(bbox->max[2] + bbox->min[2])*0.5f
		};

		const ALIGN16 float len[3]={
			(bbox->max[0] - bbox->min[0])*0.5+OCTREE_CELL_TOL,
			(bbox->max[1] - bbox->min[1])*0.5+OCTREE_CELL_TOL,
			(bbox->max[2] - bbox->min[2])*0.5+OCTREE_CELL_TOL
		};

		halfwidth = len[0];
		if ( halfwidth < len[1] ) halfwidth = len[1];
		if ( halfwidth < len[2] ) halfwidth = len[2];
	  
		buildoctree(flag, octroot, halfwidth, centerpt, facelist->face);

		DEBUG_PRINT(("buildo ctree time %d ms\n", (GetTickCount() - ts)));

	}

	__forceinline void __fastcall getfaces( boundaryBox* bound, std::vector<FaceS>& facelist, int threadID)
	{
		getfaces( octroot, bound , facelist, threadID);
	}


	inline void __fastcall getfaces( Node* root, boundaryBox* bound, std::vector<FaceS>& facelist, int threadID)
	{
		const ALIGN16 float boxhalfsize[3]={
			((bound->max[0]) - (bound->min[0]))*0.5f+OCTREE_CELL_TOL,
			((bound->max[1]) - (bound->min[1]))*0.5f+OCTREE_CELL_TOL,
			((bound->max[2]) - (bound->min[2]))*0.5f+OCTREE_CELL_TOL
		};

		const ALIGN16 float boxcenter[3]={
			((bound->max[0]) + (bound->min[0]))*0.5f,
			((bound->max[1]) + (bound->min[1]))*0.5f,
			((bound->max[2]) + (bound->min[2]))*0.5f
		};

		getfaces(root, boxcenter, boxhalfsize, facelist, threadID);
	}




	__forceinline void __fastcall delfaces( FaceS& face)
	{
		delfaces( octroot, face);
	}


	inline void __fastcall delfaces( Node* root, FaceS& face)
	{
		boundaryBox* bound = &face.box;
		const ALIGN16 float boxhalfsize[3]={
			((bound->max[0]) - (bound->min[0]))*0.5f+OCTREE_CELL_TOL,
			((bound->max[1]) - (bound->min[1]))*0.5f+OCTREE_CELL_TOL,
			((bound->max[2]) - (bound->min[2]))*0.5f+OCTREE_CELL_TOL
		};

		const ALIGN16 float boxcenter[3]={
			((bound->max[0]) + (bound->min[0]))*0.5f,
			((bound->max[1]) + (bound->min[1]))*0.5f,
			((bound->max[2]) + (bound->min[2]))*0.5f
		};

		delfaces(root, boxcenter, boxhalfsize, face);
	}
	__forceinline void __fastcall addfaces( FaceS& face)
	{
		addfaces( octroot, face);
	}


	inline void __fastcall addfaces( Node* root, FaceS& face)
	{
		boundaryBox* bound = &face.box;
		const ALIGN16 float boxhalfsize[3]={
			((bound->max[0]) - (bound->min[0]))*0.5f+OCTREE_CELL_TOL,
			((bound->max[1]) - (bound->min[1]))*0.5f+OCTREE_CELL_TOL,
			((bound->max[2]) - (bound->min[2]))*0.5f+OCTREE_CELL_TOL
		};

		const ALIGN16 float boxcenter[3]={
			((bound->max[0]) + (bound->min[0]))*0.5f,
			((bound->max[1]) + (bound->min[1]))*0.5f,
			((bound->max[2]) + (bound->min[2]))*0.5f
		};

		addfaces(root, boxcenter, boxhalfsize, face);
	}
};

#endif
