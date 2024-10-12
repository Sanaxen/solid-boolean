#ifndef _OCTREE_PCELL_H
#define _OCTREE_PCELL_H

#include <vector>

#define OCTREE_CELL_TOL	(0.001f)


#define NODE_MAX_DEPTH 7
template <class T, class U>
class octreeVertex
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


	void __fastcall buildoctree(Node *root, T halfwidth, T centerpt[3], std::vector<U>& vertexList, T MAXNUMTRIS = 5, T MINHALFWIDTH = 0.001)
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

			const int sz = vertexList.size();
			for (int ctr = 0; ctr < sz; ctr++)
			{
				const U vrt = vertexList[ctr];
				const T p[3] = {to_double(vrt.pos->x), to_double(vrt.pos->y), to_double(vrt.pos->z)};

				if (( p[0]-OCTREE_CELL_TOL > maxA[0] ) ||
					( p[0]+OCTREE_CELL_TOL < minA[0] ) ||
					( p[1]-OCTREE_CELL_TOL > maxA[1] ) ||
					( p[1]+OCTREE_CELL_TOL < minA[1] ) ||
					( p[2]-OCTREE_CELL_TOL > maxA[2] ) ||
					( p[2]+OCTREE_CELL_TOL < minA[2] ) ) continue;
			
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
	__forceinline void __fastcall getpnts(Node *root, U& pnt, std::vector<U>& vrtlist)
	{
		const T b[3] = {root->boxcenter[0], root->boxcenter[1], root->boxcenter[2]};
		const T s[3] = {root->boxhalfsize[0], root->boxhalfsize[1], root->boxhalfsize[2]};
		const T p[3] = {to_double(pnt.pos->x), to_double(pnt.pos->y), to_double(pnt.pos->z)};
		if (	p[0] < b[0]-s[0]-OCTREE_CELL_TOL ||
				p[1] < b[1]-s[1]-OCTREE_CELL_TOL ||
				p[2] < b[2]-s[2]-OCTREE_CELL_TOL ||
				p[0] > b[0]+s[0]+OCTREE_CELL_TOL ||
				p[1] > b[1]+s[1]+OCTREE_CELL_TOL ||
				p[2] > b[2]+s[2]+OCTREE_CELL_TOL )
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
             
		//for (int nodectr = 0; nodectr < 8; nodectr++)
		//	getpnts(root->subnodes[nodectr], pnt,  vrtlist);
		getpnts(root->subnodes[0], pnt,  vrtlist);
		getpnts(root->subnodes[1], pnt,  vrtlist);
		getpnts(root->subnodes[2], pnt,  vrtlist);
		getpnts(root->subnodes[3], pnt,  vrtlist);
		getpnts(root->subnodes[4], pnt,  vrtlist);
		getpnts(root->subnodes[5], pnt,  vrtlist);
		getpnts(root->subnodes[6], pnt,  vrtlist);
		getpnts(root->subnodes[7], pnt,  vrtlist);
	}    

	inline void __fastcall deleteoctree(Node *root)
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

	inline octreeVertex()
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

	inline ~octreeVertex(void)
	{
		deleteoctree(octroot);
	}
	inline void __fastcall clearVertex(void)
	{
		clearVertex(octroot);
	}
	inline void __fastcall clearVertex(Node* node)
	{
		if ( node == NULL ) return;
		node->vertexList.clear();
		clearVertex(node->subnodes[0]);
		clearVertex(node->subnodes[1]);
		clearVertex(node->subnodes[2]);
		clearVertex(node->subnodes[3]);
		clearVertex(node->subnodes[4]);
		clearVertex(node->subnodes[5]);
		clearVertex(node->subnodes[6]);
		clearVertex(node->subnodes[7]);
	}

	__forceinline void __fastcall buildoctree(std::vector<U>& object)
	{
		T halfwidth;
		T centerpt[3];
		T xyz_Max[3];
		T xyz_Min[3];

		const int sz = object.size();
		xyz_Max[0] = xyz_Min[0] = to_double(object[0].pos->x);
		xyz_Max[1] = xyz_Min[1] = to_double(object[0].pos->y);
		xyz_Max[2] = xyz_Min[2] = to_double(object[0].pos->z);
		for ( int i = 1; i < sz; i++ )
		{
			const T d[3]={ to_double(object[i].pos->x),  to_double(object[i].pos->y),  to_double(object[i].pos->z)};
			if ( xyz_Max[0] < d[0] ) xyz_Max[0] = d[0];
			if ( xyz_Min[0] > d[0] ) xyz_Min[0] = d[0];
			if ( xyz_Max[1] < d[1] ) xyz_Max[1] = d[1];
			if ( xyz_Min[1] > d[1] ) xyz_Min[1] = d[1];
			if ( xyz_Max[2] < d[2] ) xyz_Max[2] = d[2];
			if ( xyz_Min[2] > d[2] ) xyz_Min[2] = d[2];
		}
		centerpt[0] = (xyz_Max[0] + xyz_Min[0])*0.5f;
		centerpt[1] = (xyz_Max[1] + xyz_Min[1])*0.5f;
		centerpt[2] = (xyz_Max[2] + xyz_Min[2])*0.5f;

		T len[3];
		len[0] = (xyz_Max[0] - xyz_Min[0])*0.5f+OCTREE_CELL_TOL;
		len[1] = (xyz_Max[1] - xyz_Min[1])*0.5f+OCTREE_CELL_TOL;
		len[2] = (xyz_Max[2] - xyz_Min[2])*0.5f+OCTREE_CELL_TOL;

		halfwidth = len[0];
		if ( halfwidth < len[1] ) halfwidth = len[1];
		if ( halfwidth < len[2] ) halfwidth = len[2];
	  
		buildoctree(octroot, halfwidth, centerpt, object);
	}

	inline void __fastcall getpnts( U& pnt, std::vector<U>& vrtlist)
	{
		getpnts( octroot, pnt, vrtlist);
	}
	inline void __fastcall addpnts( U& pnt)
	{
		addpnts( octroot, pnt);
	}
	inline void __fastcall addpnts( Node* node, U& pnt)
	{
		const T b[3] = {node->boxcenter[0], node->boxcenter[1], node->boxcenter[2]};
		const T s[3] = {node->boxhalfsize[0], node->boxhalfsize[1], node->boxhalfsize[2]};
		const T p[3] = {to_double(pnt.pos->x), to_double(pnt.pos->y), to_double(pnt.pos->z)};
		if (	p[0] < b[0]-s[0]-OCTREE_CELL_TOL ||
				p[1] < b[1]-s[1]-OCTREE_CELL_TOL ||
				p[2] < b[2]-s[2]-OCTREE_CELL_TOL ||
				p[0] > b[0]+s[0]+OCTREE_CELL_TOL ||
				p[1] > b[1]+s[1]+OCTREE_CELL_TOL ||
				p[2] > b[2]+s[2]+OCTREE_CELL_TOL )
		{
			return;
		}

		// leaf
		if (!node->subnodes[0])
		{
			node->vertexList.push_back(pnt);
			return;
		}
             
		addpnts(node->subnodes[0], pnt);
		addpnts(node->subnodes[1], pnt);
		addpnts(node->subnodes[2], pnt);
		addpnts(node->subnodes[3], pnt);
		addpnts(node->subnodes[4], pnt);
		addpnts(node->subnodes[5], pnt);
		addpnts(node->subnodes[6], pnt);
		addpnts(node->subnodes[7], pnt);
	}

};

#endif
