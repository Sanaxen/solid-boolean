#ifndef _KDTREE_HPP
#define _KDTREE_HPP

#if _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif

#include "kdtree.h"
#include <vector>

template <class T>
class KdTree
{
	struct kdtree *ptree;
public:
	KdTree()
	{
	  /* create a k-d tree for 3-dimensional points */
	  ptree = kd_create( 3 );
	}

	~KdTree()
	{
		kd_free( ptree );
		ptree = NULL;
	}

	inline void insert( int id, T* attr)
	{
		if ( kd_insert( ptree, id, attr ) != 0 )
		{
			throw "no memory.";
		}
	}

	inline int nearest_find(double x, double y, double z, const double radius, std::vector<int>& nearest_list)
	{
		double pt[3] = {x, y, z};

		return nearest_find(pt, radius, nearest_list);
	}
	inline int nearest_find(float x, float y, float z, const float radius, std::vector<int>& nearest_list)
	{
		double pt[3] = {x, y, z};

		return nearest_find(pt, radius, nearest_list);
	}

	inline int nearest_find(const double pt[3], const double radius, std::vector<int>& nearest_list)
	{
		/* find points closest to the origin and within distance radius */
		struct kdres *presults = kd_nearest_range( (struct kdtree *)ptree, pt, radius );

		int sz = kd_res_size(presults);
		if ( sz == 0 )
		{
			kd_res_free( presults );
			return 0;
		}
		int pos = 0;

		while( !kd_res_end( presults ) ) {
			/* get the data and position of the current result item */
			T* attr = (T*)kd_res_item( presults, &pos );

			nearest_list.push_back(pos);

			/* go to the next entry */
			kd_res_next( presults );
		}
		kd_res_free( presults );
		return sz;
	}

};

#endif
