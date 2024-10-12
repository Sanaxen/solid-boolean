/*
Double the precision of floating point operations using QD.
Many of the ideas and basic code were heavily influenced by Greg Santucci and Danilo Balby in 2008 and 2009.
*/
#ifndef SEGMENT_H
#define SEGMENT_H

#include "ML_Vector.h"

#include "Line.h"
#include "Face.h"
#include "Vertex.h"

/** define as vertex one of the segment ends */
/** define as face one of the segment ends */
/** define as edge one of the segment ends */
#define Segment_VERTEX  1
#define Segment_FACE  2
#define Segment_EDGE  3


class Segment
{
public:

	inline Segment(){};
	void Create(const Line & line, Face* face, int sign[3], Vertex** vertex = NULL);

	/** line resulting from the two planes intersection */
	Line line;
	/** shows how many ends were already defined */
	int index;
	
	/** distance from the segment starting point to the point defining the plane */
	mlFloat startDist;
	/** distance from the segment ending point to the point defining the plane */
	mlFloat endDist;
	
	/** starting point status relative to the face */
	int startType;
	/** intermediate status relative to the face */
	int middleType;
	/** ending point status relative to the face */
	int endType;
	
	/** nearest vertex from the starting point */
	Vertex * startVertex;
	/** nearest vertex from the ending point */
	Vertex * endVertex; 
	
	/** start of the intersection point */
	mlVector3D startPos;
	/** end of the intersection point */
	mlVector3D endPos;

	
	/**
	 * Gets the start vertex
	 * 
	 * @return start vertex
	 */
	inline Vertex * __fastcall getStartVertex() const
	{
		return startVertex;
	}	
	/**
	 * Gets the end vertex
	 * 
	 * @return end vertex
	 */
	inline Vertex * __fastcall getEndVertex() const
	{
		return endVertex;
	}

	inline void __fastcall getStartEnd(Vertex** s, Vertex** e) const
	{
		*s = startVertex;
		*e = endVertex;
	}
#if 0
	/**
	 * Gets the distance from the origin until the starting point
	 * 
	 * @return distance from the origin until the starting point
	 */
	inline mlFloat __fastcall getStartDistance() const
	{
		return startDist;
	}
	/**
	 * Gets the distance from the origin until ending point
	 * 
	 * @return distance from the origin until the ending point
	 */
	inline mlFloat __fastcall getEndDistance() const
	{
		return endDist;
	}

	/**
	 * Gets the type of the starting point
	 * 
	 * @return type of the starting point
	 */
	inline int __fastcall getStartType() const
	{
		return startType;
	}

	/**
	 * Gets the type of the segment between the starting and ending points
	 * 
	 * @return type of the segment between the starting and ending points
	 */
	inline int __fastcall getIntermediateType() const
	{
		return middleType;
	}

	/**
	 * Gets the type of the ending point
	 * 
	 * @return type of the ending point
	 */
	inline int __fastcall getEndType() const
	{
		return endType;
	}
#endif
	/**
	 * Gets the number of ends already set
	 *
	 * @return number of ends already set
	 */
	inline int __fastcall getNumEndsSet() const
	{
		return index;
	}
#if 0
	/**
	 * Gets the starting position
	 * 
	 * @return start position
	 */
	inline mlVector3D __fastcall getStartPosition() const
	{
		return startPos;
	}

	/**
	 * Gets the ending position
	 * 
	 * @return ending position
	 */
	inline mlVector3D __fastcall getEndPosition() const
	{
		return endPos;
	}

#endif
	/**
	 * Checks if two segments intersect
	 * 
	 * @param segment the other segment to check the intesection
	 * @return true if the segments intersect, false otherwise
	 */
	inline bool __fastcall intersect(const Segment & segment) const
	{
		if(endDist<segment.startDist+Segment_TOL ||segment.endDist<startDist+Segment_TOL)
		{
			return false;
		}
		else
		{
			return true;
		}
	}

private:

	bool __fastcall setVertex(Vertex * vertex);
	bool __fastcall setEdge(Vertex * vertex1, Vertex * vertex2);

	/** Swaps the starting point and the ending point */	
	inline void __fastcall swapEnds()
	{
		mlFloat distTemp = startDist;
		startDist = endDist;
		endDist = distTemp;
	
		int typeTemp = startType;
		startType = endType;
		endType = typeTemp;
	
		Vertex * vertexTemp = startVertex;
		startVertex = endVertex;
		endVertex = vertexTemp;
	
		mlVector3D posTemp = startPos;
		startPos = endPos;
		endPos = posTemp;		
	}
};

#endif // SEGMENT_H
