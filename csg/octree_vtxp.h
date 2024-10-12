#ifndef _OCTREE_PCELLP_H
#define _OCTREE_PCELLP_H

#include <vector>

#define OCTREE_CELL_TOL	(0.001f)


#define NODE_MAX_DEPTH 7
template <class T, class U>
class octreeVertexP
{
	// Octree code
	struct Node {
		Node* subnodes[8];

		T boxcenter[3];
		T boxhalfsize[3];

		std::vector<U> vertexList;
		int level;
		Node()
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


	void buildoctree(Node *root, T halfwidth, T centerpt[3], std::vector<U>& vertexList, T MAXNUMTRIS = 5, T MINHALFWIDTH = 0.001)
	{
		T *boxcenter   = root->boxcenter;
		T *boxhalfsize = root->boxhalfsize;
		boxcenter[0]   = centerpt[0];
		boxcenter[1]   = centerpt[1];
		boxcenter[2]   = centerpt[2];
		boxhalfsize[0] = boxhalfsize[1] = boxhalfsize[2] = halfwidth;

		if (vertexList.size() < MAXNUMTRIS || halfwidth < MINHALFWIDTH || root->level == 1)
		{
			std::vector<U>::iterator it = vertexList.begin();
			std::vector<U>::iterator iend = vertexList.end();

			std::vector<U>* root_vrt = &(root->vertexList);
			for ( ; it != iend; ++it)
			{
				root_vrt->push_back((*it));
			}
			return; 
		}
    
		T halfwidth_half     = halfwidth*0.5f;
		T halfwidth_half_tol = halfwidth_half+OCTREE_CELL_TOL;
		T boxhalfsize_half[3]     = {halfwidth_half_tol, halfwidth_half_tol, halfwidth_half_tol};

		for (int nodectr = 0; nodectr < 8; nodectr++)
		{
			std::vector<U> tempvrt;

			T plusorminus[3] = {-1.0f, -1.0f, -1.0f};

			if (nodectr & 1) plusorminus[0] = 1.0f;
			if (nodectr & 2) plusorminus[1] = 1.0f;
			if (nodectr & 4) plusorminus[2] = 1.0f;

			T boxcenter[3] =
			{
				centerpt[0] + plusorminus[0]*halfwidth_half,
				centerpt[1] + plusorminus[1]*halfwidth_half,
				centerpt[2] + plusorminus[2]*halfwidth_half
			};

			T minA[3], maxA[3];

			maxA[0] = boxcenter[0]+boxhalfsize_half[0]+OCTREE_CELL_TOL;
			minA[0] = boxcenter[0]-boxhalfsize_half[0]-OCTREE_CELL_TOL;

			maxA[1] = boxcenter[1]+boxhalfsize_half[1]+OCTREE_CELL_TOL;
			minA[1] = boxcenter[1]-boxhalfsize_half[1]-OCTREE_CELL_TOL;

			maxA[2] = boxcenter[2]+boxhalfsize_half[2]+OCTREE_CELL_TOL;
			minA[2] = boxcenter[2]-boxhalfsize_half[2]-OCTREE_CELL_TOL;

			for (int ctr = 0; ctr < vertexList.size(); ctr++)
			{
				U vrt = vertexList[ctr];

				if (( vrt->x-OCTREE_CELL_TOL > maxA[0] ) ||
					( vrt->x+OCTREE_CELL_TOL < minA[0] ) ||
					( vrt->y-OCTREE_CELL_TOL > maxA[1] ) ||
					( vrt->y+OCTREE_CELL_TOL < minA[1] ) ||
					( vrt->z-OCTREE_CELL_TOL > maxA[2] ) ||
					( vrt->z+OCTREE_CELL_TOL < minA[2] ) ) continue;
			
				tempvrt.push_back(vrt);
			}
			root->subnodes[nodectr] = new Node;
			root->subnodes[nodectr]->level = root->level/2;

			buildoctree(root->subnodes[nodectr], halfwidth_half, boxcenter, tempvrt, MAXNUMTRIS, MINHALFWIDTH);
		}

	}


  // draw faces that intersect an AABB
  // this is only approximate - for it to be completely accurate,
  // if there's only a partial intersection with a leaf node, we 
  // should check each triangle with the AABB
	void getpnts(Node *root, U& pnt, std::vector<U>& vrtlist)
	{

		if (	pnt->x < root->boxcenter[0]-root->boxhalfsize[0]-OCTREE_CELL_TOL ||
				pnt->y < root->boxcenter[1]-root->boxhalfsize[1]-OCTREE_CELL_TOL ||
				pnt->z < root->boxcenter[2]-root->boxhalfsize[2]-OCTREE_CELL_TOL ||
				pnt->x > root->boxcenter[0]+root->boxhalfsize[0]+OCTREE_CELL_TOL ||
				pnt->y > root->boxcenter[1]+root->boxhalfsize[1]+OCTREE_CELL_TOL ||
				pnt->z > root->boxcenter[2]+root->boxhalfsize[2]+OCTREE_CELL_TOL )
		{
			return;
		}

		// leaf
		if (!root->subnodes[0])
		{
			std::vector<U>::iterator it = root->vertexList.begin();
			std::vector<U>::iterator iend = root->vertexList.end();

			for ( ; it != iend; ++it)
			{
				vrtlist.push_back((*it));
			}
			return;                      
		}
             
		for (int nodectr = 0; nodectr < 8; nodectr++)
			getpnts(root->subnodes[nodectr], pnt,  vrtlist);
	}    

	inline void deleteoctree(Node *root)
	{
     
		// if there are leaves, delete them     
		if (root->subnodes[0])
		for (int nodectr = 0; nodectr < 8; nodectr++) deleteoctree(root->subnodes[nodectr]);

		delete root;
     
	}


 public:

	inline octreeVertexP()
	{
		octroot = new Node;
		octroot->level = (int)pow(2.0, NODE_MAX_DEPTH);
		for (int nodectr = 0; nodectr < 8; nodectr++) octroot->subnodes[nodectr] = NULL;
	}


	Node* octroot;

	inline ~octreeVertexP(void)
	{
		deleteoctree(octroot);
	}

	inline void buildoctree(std::vector<U>& object)
	{
		T halfwidth;
		T centerpt[3];
		T xMax, xMin;
		T yMax, yMin;
		T zMax, zMin;

		int sz = object.size();
		xMax = xMin = object[0]->x;
		yMax = yMin = object[0]->y;
		zMax = zMin = object[0]->z;
		for ( int i = 1; i < sz; i++ )
		{
			if ( xMax < object[i]->x ) xMax = object[i]->x;
			if ( yMax < object[i]->y ) yMax = object[i]->y;
			if ( zMax < object[i]->z ) zMax = object[i]->z;
			if ( xMin > object[i]->x ) xMin = object[i]->x;
			if ( yMin > object[i]->y ) yMin = object[i]->y;
			if ( zMin > object[i]->z ) zMin = object[i]->z;
		}
		centerpt[0] = (xMax + xMin)*0.5f;
		centerpt[1] = (yMax + yMin)*0.5f;
		centerpt[2] = (zMax + zMin)*0.5f;

		T len[3];
		len[0] = (xMax - xMin)*0.5f+OCTREE_CELL_TOL;
		len[1] = (yMax - yMin)*0.5f+OCTREE_CELL_TOL;
		len[2] = (zMax - zMin)*0.5f+OCTREE_CELL_TOL;

		halfwidth = len[0];
		if ( halfwidth < len[1] ) halfwidth = len[1];
		if ( halfwidth < len[2] ) halfwidth = len[2];
	  
		buildoctree(octroot, halfwidth, centerpt, object);
	}

	inline void getpnts( U& pnt, std::vector<U>& vrtlist)
	{
		getpnts( octroot, pnt, vrtlist);
	}
};

#endif
