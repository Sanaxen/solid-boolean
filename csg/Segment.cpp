/*
Double the precision of floating point operations using QD.
Many of the ideas and basic code were heavily influenced by Greg Santucci and Danilo Balby in 2008 and 2009.
*/
#if _DEBUG
#ifndef _CRTDBG_MAP_ALLOC
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif
#endif

#include "Segment.h"
#include "tol.h"


// Keep both chunks.
void Segment::Create(const Line & line, Face* face, int sign[3], Vertex** vertex)
{
	//TOL = 1e-10f;

	this->line = line;
	index = 0;

	if ( vertex == NULL )
	{
		vertex = face->getVertex();
	}

	
	//VERTEX is an end
	if(sign[0] == 0)
	{
		setVertex(vertex[0]);
		//other vertices on the same side - VERTEX-VERTEX VERTEX
		if(sign[1] == sign[2])
		{
			setVertex(vertex[0]);
		}
	}
	
	//VERTEX is an end
	if(sign[1] == 0)
	{
		setVertex(vertex[1]);
		//other vertices on the same side - VERTEX-VERTEX VERTEX
		if(sign[0] == sign[2])
		{
			setVertex(vertex[1]);
		}
	}
	
	//VERTEX is an end
	if(sign[2] == 0)
	{
		setVertex(vertex[2]);
		//other vertices on the same side - VERTEX-VERTEX VERTEX
		if(sign[0] == sign[1])
		{
			setVertex(vertex[2]);
		}
	}
	
	//There are undefined ends - one or more edges cut the planes intersection line
	if(getNumEndsSet() != 2)
	{
		//EDGE is an end
		if((sign[0]==1 && sign[1]==-1)||(sign[0]==-1 && sign[1]==1))
		{
			setEdge(vertex[0], vertex[1]);
		}
		//EDGE is an end
		if((sign[1]==1 && sign[2]==-1)||(sign[1]==-1 && sign[2]==1))
		{
			setEdge(vertex[1], vertex[2]);
		}
		//EDGE is an end
		if((sign[2]==1 && sign[0]==-1)||(sign[2]==-1 && sign[0]==1))
		{
			setEdge(vertex[2], vertex[0]);
		}
	}
}

//-------------------------------------GETS-------------------------------------//








//------------------------------------OTHERS------------------------------------//


//---------------------------------PRIVATES-------------------------------------//

/**
 * Sets an end as vertex (starting point if none end were defined, ending point otherwise)
 * 
 * @param vertex the vertex that is an segment end 
 * @return false if all the ends were already defined, true otherwise
 */
bool __fastcall Segment::setVertex(Vertex * vertex)
{
	//none end were defined - define starting point as VERTEX
	if(index == 0)
	{
		startVertex = vertex;
	 	startType = Segment_VERTEX;
	 	line.computePointToPointDistance(vertex->xyz, startDist);
	 	startPos = startVertex->xyz;
	 	index++;
	 	return true;
	}
	//starting point were defined - define ending point as VERTEX
	if(index == 1)
	{
		endVertex = vertex;
		endType = Segment_VERTEX;
		line.computePointToPointDistance(vertex->xyz, endDist);
		endPos = endVertex->xyz;
		index++;
		
		//defining middle based on the starting point
		//VERTEX-VERTEX-VERTEX
		//if(startVertex.equals(endVertex)) // Need close-enough-to...
		if(true)
		{
			middleType = Segment_VERTEX;
		}
		//VERTEX-EDGE-VERTEX
		else if(startType==Segment_VERTEX)
		{
			middleType = Segment_EDGE;
		}
		
		//the ending point distance should be smaller than  starting point distance 
		if(startDist>endDist)
		{
			swapEnds();
		}
		
		return true;
	}
	else
	{
		return false;
	}
}

/**
 * Sets an end as edge (starting point if none end were defined, ending point otherwise)
 * 
 * @param vertex1 one of the vertices of the intercepted edge 
 * @param vertex2 one of the vertices of the intercepted edge
 * @return false if all ends were already defined, true otherwise
 */
bool __fastcall Segment::setEdge(Vertex * vertex1, Vertex * vertex2)
{
#if 0
	mlVector3D point1( vertex1->xyz );
	mlVector3D point2( vertex2->xyz );

	mlVector3D edgeDirection(point2.x - point1.x, point2.y - point1.y, point2.z - point1.z);
	Line edgeLine(edgeDirection, point1);
#else
	//const mlVector3D edgeDirection(vertex2->xyz.x - vertex1->xyz.x, vertex2->xyz.y - vertex1->xyz.y, vertex2->xyz.z - vertex1->xyz.z);
	//Line edgeLine(edgeDirection, vertex1->xyz);
	Line edgeLine(mlVector3D(vertex2->xyz.x - vertex1->xyz.x, vertex2->xyz.y - vertex1->xyz.y, vertex2->xyz.z - vertex1->xyz.z), vertex1->xyz);
#endif

	if(index==0)
	{
		startVertex = vertex1;
		startType = Segment_EDGE;
		line.computeLineIntersection(edgeLine, startPos);
		line.computePointToPointDistance(startPos, startDist);
		middleType = Segment_FACE;
		index++;
		return true;		
	}
	else if(index==1)
	{
		endVertex = vertex1;
		endType = Segment_EDGE;
		line.computeLineIntersection(edgeLine, endPos);
		line.computePointToPointDistance(endPos, endDist);
		middleType = Segment_FACE;
		index++;
		
		//the ending point distance should be smaller than  starting point distance 
		if(startDist>endDist)
		{
		  	swapEnds();
		}
		
		return true;
	}
	else
	{
		return false;
	}
}

