/*
Double the precision of floating point operations using QD.
Many of the ideas and basic code were heavily influenced by Greg Santucci and Danilo Balby in 2008 and 2009.
*/
#include <stdlib.h>

#include "Line.h"

#include "Face.h"
#include "Vertex.h"

#include <math.h>
#include "def.h"

#if _DEBUG
#ifndef _CRTDBG_MAP_ALLOC
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#endif

const mlFloat Line::TOL = Line_TOL;
const mlFloat Line::TOL2 = Line_TOL* Line_TOL;
//mlFloat Line::TOL = 1e-10f;
//mlFloat Line::TOL = 1e-5f;
//mlFloat Line::TOL = 1e-3f;


mlVector3D __fastcall Line::computePlaneIntersection(const mlVector3D & normal, const mlVector3D & planePoint, bool & bResult) const
{
	bResult = true;

	//Ax + By + Cz + D = 0
	//x = x0 + t(x1 ÅEx0)
	//y = y0 + t(y1 ÅEy0)
	//z = z0 + t(z1 ÅEz0)
	//(x1 - x0) = dx, (y1 - y0) = dy, (z1 - z0) = dz
	//t = -(A*x0 + B*y0 + C*z0 )/(A*dx + B*dy + C*dz)

	
	//const mlFloat D = -(normal.x*planePoint.x + normal.y*planePoint.y + normal.z*planePoint.z);
	//const mlFloat numerator = VECTOR3D_DOT(normal, point) + D;
	const mlFloat numerator = normal.x*(point.x - planePoint.x)
							+ normal.y*(point.y - planePoint.y)
							+ normal.z*(point.z - planePoint.z);

	const mlFloat denominator = VECTOR3D_DOT(normal, direction);	
	const mlFloat tol = TOL;

	//if line is paralel to the plane...
	if(Absolute(denominator)<tol)
	{
		//if line is contained in the plane...
		if(Absolute(numerator)<tol)
		{
			return point;
		}
		else
		{
			bResult = false;
			return mlVector3DZero;
			//return null;
		}
	}
	//if line intercepts the plane...
	else
	{
		const mlFloat t = -numerator/denominator;
		const mlVector3D resultPoint = point + direction * t;
		
		return resultPoint;
	}
}




