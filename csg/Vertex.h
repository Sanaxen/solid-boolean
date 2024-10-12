/*
Double the precision of floating point operations using QD.
Many of the ideas and basic code were heavily influenced by Greg Santucci and Danilo Balby in 2008 and 2009.
*/
#ifndef VERTEX_H
#define VERTEX_H

#include "ML_Vector.h"
#include "VertexSet.h"
#include "ColorSet.h"
#include <stdio.h>
#include "tol.h"

class VertexPointerSet;

#define VERTEX_REF_COUNT	1
#define VERTEX_REF_COUNT_INIT_VALUE	1
#define VERTEX_LINK_USE

#define Vertex_UNKNOWN  1
#define Vertex_INSIDE   2
#define Vertex_OUTSIDE  3
#define Vertex_BOUNDARY 4


class Vertex
{
	color_attr_type color;
	int ref_count;
	int status;
	int* statusPtr;
public:
	int id;
	mlVector3D xyz;

	inline Vertex():status(Vertex_UNKNOWN),ref_count(VERTEX_REF_COUNT_INIT_VALUE),statusPtr(0),id(-1)
	{
		xyz.x = xyz.y = xyz.z = mlFloat(0.0);
	}

	inline Vertex(const Vertex & v):xyz(v.xyz),status(v.status),color(v.color),ref_count(VERTEX_REF_COUNT_INIT_VALUE),statusPtr(0),id(-1)
	{
#ifndef VERTEX_LINK_USE		
		adjacentVertices.Add(v.adjacentVertices);
#endif
	}

	inline Vertex(const Vertex* v):xyz(v->xyz),status(v->status),color(v->color),ref_count(VERTEX_REF_COUNT_INIT_VALUE),statusPtr(0),id(-1)
	{
#ifndef VERTEX_LINK_USE
		adjacentVertices.Add(v->adjacentVertices);
#endif
	}

	inline Vertex(const mlVector3D & position, color_attr_type* icolor):xyz(position),status(Vertex_UNKNOWN),color(*icolor),ref_count(VERTEX_REF_COUNT_INIT_VALUE),statusPtr(0),id(-1)
	{
	}

	inline Vertex(const mlVector3D & position, const color_attr_type icolor, const int stat):xyz(position),status(stat),color(icolor),ref_count(VERTEX_REF_COUNT_INIT_VALUE),statusPtr(0),id(-1)
	{
	}

	inline Vertex::~Vertex()
	{
	}	
		
	inline float* toFloat(float* v)
	{
#ifdef USE_QDOUBLE
		v[0] = to_double(xyz.x);
		v[1] = to_double(xyz.y);
		v[2] = to_double(xyz.z);
#else
		v[0] = xyz.x;
		v[1] = xyz.y;
		v[2] = xyz.z;
#endif
		return v;
	}

	inline Vertex* __fastcall AddRef()
	{
		ref_count++;
		return this;
	}

	inline void __fastcall Release()
	{
		ref_count--;
		if ( ref_count < 0 )
		{
			printf("internal error[Have a misuse] Vertex->Release\n");
		}
#if VERTEX_REF_COUNT
		if ( ref_count == 0 )
		{
			delete this;
		}
#else
			delete this;
#endif
	}


	inline bool __fastcall equals(const Vertex * vertex) const
	{
		const mlVector3D& w = vertex->xyz;
		const mlFloat tol = Vertex_TOL;
		return	( color.color == vertex->color.color && Absolute(xyz.x-w.x)<tol && Absolute(xyz.y-w.y)<tol && Absolute(xyz.z-w.z)<tol );
	}
	inline bool __fastcall equals(const Vertex * vertex, const mlFloat& tol) const
	{
		const mlVector3D& w = vertex->xyz;
		return	(color.color == vertex->color.color && Absolute(xyz.x-w.x)<tol && Absolute(xyz.y-w.y)<tol && Absolute(xyz.z-w.z)<tol );
	}
		
	inline bool __fastcall equals_geometry(const Vertex * vertex) const
	{
		const mlVector3D& w = vertex->xyz;
		const mlFloat tol = Vertex_TOL;
		return	(Absolute(xyz.x-w.x)<tol && Absolute(xyz.y-w.y)<tol && Absolute(xyz.z-w.z)<tol );
	}
	
	inline bool __fastcall equals_geometry(const Vertex * vertex, const mlFloat& tol) const
	{
		const mlVector3D& w = vertex->xyz;
		return	(Absolute(xyz.x-w.x)<tol && Absolute(xyz.y-w.y)<tol && Absolute(xyz.z-w.z)<tol );
	}

	inline void __fastcall setStatus(const int stat)
	{
		if(status>=Vertex_UNKNOWN && status<=Vertex_BOUNDARY)
		{
			if ( statusPtr != NULL ) *statusPtr = stat;
			this->status = stat;
		}
	}
	inline const color_attr_type& __fastcall getColor() const
	{
		return color;
	}
	inline const void __fastcall setColor(const color_attr_type& icolor) 
	{
		 color = icolor;
	}

	inline const mlVector3D& __fastcall getPosition() const
	{
		return xyz;
	}


	inline int __fastcall getStatus() const
	{
		if ( statusPtr != NULL ) return *statusPtr;
		return status;
	}

	inline void __fastcall addAdjacentVertex(Vertex * adjacentVertex)
	{
#ifdef VERTEX_LINK_USE
		if ( statusPtr != NULL )
		{
			adjacentVertex->statusPtr = statusPtr;
		}else
		{
			adjacentVertex->statusPtr = &status;
		}
#else
		// Call add on adjacentVertices
		if(!adjacentVertices.FindVertexPtr(adjacentVertex))
		{
			adjacentVertices.AddVertexPointer(adjacentVertex);
		}
#endif
	}

	inline void __fastcall delete_mark()
	{
		statusPtr = NULL;
	}
#ifdef VERTEX_LINK_USE
	inline void __fastcall mark(const int eStatus)
	{
		if ( statusPtr == NULL )
		{
			status = eStatus;
		}else
		{
			*statusPtr = eStatus;
		}
	}
#else
	void __fastcall mark(int eStatus);
#endif

private:
	//inline Vertex & operator=(const Vertex & v)
	//{
	//	adjacentVertices.Add(v.adjacentVertices);
	//	status = v.status;
	//	color = v.color;
	//	xyz = v.xyz;

	//	return *this;
	//}	
#ifndef VERTEX_LINK_USE
	VertexPointerSet adjacentVertices;
#endif
};

#ifdef VERTEX_LINK_USE
#define Vertex_mark_metod_call_stack_init()	/* */
#else
void Vertex_mark_metod_call_stack_init();
#endif

#endif // VERTEX_H
