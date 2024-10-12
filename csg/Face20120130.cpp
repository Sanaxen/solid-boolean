
// Author: Greg Santucci, 2008 and 2009
// This is part of a port of the CSG project
// originally written in java by Danilo Balby
// Email: thecodewitch@gmail.com
// Web: http://createuniverses.blogspot.com/

#include "Face.h"

#include "Vertex.h"
#include "Bound.h"

#include "ML_Vector.h"

#include "Line.h"
#include "octreecell.h"

#include <math.h>
#include <stdio.h>

#include "def.h"

#include "openmp_config.h"

#if _DEBUG
#ifndef _CRTDBG_MAP_ALLOC
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#endif

const int Face::UNKNOWN     = 1;
const int Face::INSIDE      = 2;
const int Face::OUTSIDE     = 3;
const int Face::SAME        = 4;
const int Face::OPPOSITE    = 5;

const int Face::UP          = 6;
const int Face::DOWN        = 7;
const int Face::ON          = 8;
const int Face::NONE        = 9;

const mlFloat Face::TOL = Face_TOL;
//mlFloat Face::TOL = 1e-14;
//mlFloat Face::TOL = 1e-5f;
//mlFloat Face::TOL = 1e-3f;




mlVector3D Face::calsNormal()
{
#if 0
	mlVector3D p1 = v1->xyz;
	mlVector3D p2 = v2->xyz;
	mlVector3D p3 = v3->xyz;
	
	mlVector3D xy = p2 - p1;
	mlVector3D xz = p3 - p1;

	//xy.Normalise();
	//xz.Normalise();
	
	mlVector3D normal = mlVectorCross(xy, xz);
	normal.Normalise();
	
	return normal;

#else
	;
	;

	mlVector3D normal = mlVectorCross(
		mlVector3D(v2->xyz.x - v1->xyz.x,  v2->xyz.y - v1->xyz.y, v2->xyz.z - v1->xyz.z), 
		mlVector3D(v3->xyz.x - v1->xyz.x,  v3->xyz.y - v1->xyz.y, v3->xyz.z - v1->xyz.z));
	normal.Normalise();
	
	return normal;
	
#endif
}



mlFloat Face::getArea()
{
#if 0
	//area = (a * c * sen(B))/2

	mlVector3D p1 = v1->xyz;
	mlVector3D p2 = v2->xyz;
	mlVector3D p3 = v3->xyz;
	mlVector3D xy = p2 - p1;
	mlVector3D xz = p3 - p1;
	
	mlFloat a = xy.Magnitude();
	mlFloat c = xz.Magnitude();
	//mlFloat B = xy.angle(xz);
	mlFloat B = 0.0f;
	{
		mlVector3D xyNorm = xy.Normalised();
		mlVector3D xzNorm = xz.Normalised();
		mlFloat fDot = xyNorm * xzNorm;
		mlFloat fAngle = acos(fDot);
		B = fAngle;
	}
	
	return (a * c * sin(B))/2.0f;
#else
	mlVector3D normal = mlVectorCross(
		mlVector3D(v2->xyz.x - v1->xyz.x,  v2->xyz.y - v1->xyz.y, v2->xyz.z - v1->xyz.z), 
		mlVector3D(v3->xyz.x - v1->xyz.x,  v3->xyz.y - v1->xyz.y, v3->xyz.z - v1->xyz.z));

	mlFloat a = sqrt(normal*normal)*0.5;
	return a;
#endif
}



bool Face::simpleClassify()
{
	const int status1 = v1->getStatus();
	const int status2 = v2->getStatus();
	const int status3 = v3->getStatus();
	
#if 10
	if(status1==Vertex::INSIDE || status2==Vertex::INSIDE || status3==Vertex::INSIDE)
	{
		status = INSIDE;
		return true; 
	}
	if(status1==Vertex::OUTSIDE || status2==Vertex::OUTSIDE || status3==Vertex::OUTSIDE)
	{
		status = OUTSIDE;
		return true; 
	}
	return false;
#else
	if(status1==Vertex::INSIDE)
	{
		status = INSIDE;
		return true; 
	}

	if(status1==Vertex::OUTSIDE)
	{
		status = OUTSIDE;
		return true; 
	}

	if(status2==Vertex::INSIDE)
	{
		status = INSIDE;
		return true;
	}

	if(status2==Vertex::OUTSIDE)
	{
		status = OUTSIDE;
		return true;
	}

	if(status3==Vertex::INSIDE)
	{
		status = INSIDE;
		return true;
	}

	if(status3==Vertex::OUTSIDE)
	{
		status = OUTSIDE;
		return true;
	}
	return false;
#endif

}

int Face::rayTraceClassify(Object3D & object)
{
	//printf("rayTraceClassify\n");
	if ( !object.getBound()->overlap(getBound()))
	{
		status = OUTSIDE;
		return 0;
	}

	//creating a ray starting starting at the face baricenter going to the normal direction
#if 10
	mlVector3D p0(	(v1->xyz.x + v2->xyz.x + v3->xyz.x)/3.0,
					(v1->xyz.y + v2->xyz.y + v3->xyz.y)/3.0,
					(v1->xyz.z + v2->xyz.z + v3->xyz.z)/3.0 );
	Line ray(getNormal(),p0);
#else
	mlVector3D p0;
	p0.x = (v1->xyz.x + v2->xyz.x + v3->xyz.x)/3.0;
	p0.y = (v1->xyz.y + v2->xyz.y + v3->xyz.y)/3.0;
	p0.z = (v1->xyz.z + v2->xyz.z + v3->xyz.z)/3.0;
	Line ray(getNormal(),p0);
#endif

	volatile bool success;
	Face * closestFace = 0;
	mlFloat closestDistance;


	//mlFloat TOL = 0.0001f;
	
	int stat = 0;
	int n = 0;
	int loopmax = 100;
	const mlFloat maxdist = 99999.9;
	const mlFloat zero = mlFloat(0.0);
	const mlFloat tol = Face::TOL;


	mlVector3D rayDir;
	do
	{
		rayDir = ray.getDirection();
		if ( n > loopmax )
		{
			printf("rayTrace error.\n");
			stat = -1;
			break;
		}

		//リトライ処理（レイを少しだけ傾けてみる
		if ( n >= 1 )
		{
			ray.perturbDirection(rayDir);
		}

		n++;
		success = true;
		closestDistance = maxdist;

		//for each face from the other solid...
		const int sz = object.getNumFaces();

		for(int i=0;i<sz;i++)
		{
			mlFloat dotProduct, distance; 
			mlFloat fabs_dotProduct, fabs_distance; 
			mlVector3D intersectionPoint;

			Face* face = object.getFace(i);

			//ラフチェックで使用
			if (!face->RayIntersection(p0, rayDir))
			{
				continue;
			}
			//ラフチェックで使用(これは若干問題があるかも）
			//集合演算（引き算）でソリッドの一部が綺麗に抜け落ちたらこれを疑う！！
			//if (face->intersection(p0, rayDir) == 0 )
			//{
			//	continue;
			//}

			mlVector3D& face_normal = face->getNormal();

//			dotProduct = face_normal * ray.getDirection(); // dot product
			dotProduct = VECTOR3D_DOT(face_normal, rayDir); // dot product
			fabs_dotProduct = fabs(dotProduct);

			bool bIntersectResult = false;
			intersectionPoint = ray.computePlaneIntersection(face_normal, face->v1->xyz, bIntersectResult);
			// Need to return whether was successful.
				
			//if ray intersects the plane...  
			//if(intersectionPoint!=0)
			if(bIntersectResult)
			{
				distance = ray.computePointToPointDistance(intersectionPoint);
				fabs_distance = fabs(distance);

				//if ray lies in plane...
				if(fabs_distance<tol && fabs_dotProduct<tol)
				{
					//disturb the ray in order to not lie into another plane 
					//ray.perturbDirection();
					//rayDir = ray.getDirection();
					success = false;
					break;
				}

				
				//if ray starts in plane...
				if(fabs_distance<tol && fabs_dotProduct>tol)
				{
					//if ray intersects the face...
					if(face->hasPoint(intersectionPoint))
					{
						//faces coincide
						closestFace = face;
						closestDistance = zero;
						break;
					}
				}
				
				//if ray intersects plane... 
				else if(fabs_dotProduct>tol && distance>tol)
				{
					if(distance<closestDistance)
					{
						//if ray intersects the face;
						if(face->hasPoint(intersectionPoint))
						{
							//this face is the closest face untill now
							closestDistance = distance;
							closestFace = face;
						}
					}
				}
			}
		}
	}while(success==false);
	
	status = SAME;
	if ( stat < 0 )
	{
		return stat;
	}

	//none face found: outside face
	if(closestFace==0)
	{
		status = OUTSIDE;
	}
	//face found: test dot product
	else
	{
		const mlVector3D closestFaceNormal = closestFace->getNormal();
		const mlFloat dotProduct = VECTOR3D_DOT(closestFaceNormal, rayDir);
		
		//distance = 0: coplanar faces
		if(fabs(closestDistance)<tol)
		{
			if(dotProduct>TOL)
			{
				status = SAME;
			}
			else if(dotProduct<-tol)
			{
				status = OPPOSITE;
			}
		}
		
		//dot product > 0 (same direction): inside face
		else if(dotProduct>tol)
		{
			status = INSIDE;
		}
		
		//dot product < 0 (opposite direction): outside face
		else if(dotProduct<-tol)
		{
			status = OUTSIDE;
		}
	}
	return stat;
}

//------------------------------------PRIVATES----------------------------------//

/**
 * Checks if the the face contains a point
 * 
 * @param point to be tested
 * @param true if the face contains the point, false otherwise 
 */	
bool Face::hasPoint(const mlVector3D &  point)
{
	int result1, result2, result3;
	const mlVector3D& normal = getNormal(); 

	mlVector3D pos[3];

	getPosition(pos);

	int axis = 0;
	mlFloat maxval = fabs(normal.x);

	if ( fabs(normal.y) > maxval )
	{
		axis = 1;
		maxval = fabs(normal.y);
	}
	if ( fabs(normal.z) > maxval )
	{
		axis = 2;
		maxval = fabs(normal.z);
	}

	switch(axis)
	{
	case 0:
		result1 = linePositionInX(point, pos[0], pos[1]);
		result2 = linePositionInX(point, pos[1], pos[2]);
		result3 = linePositionInX(point, pos[2], pos[0]);
		break;
	case 1:
		result1 = linePositionInY(point, pos[0], pos[1]);
		result2 = linePositionInY(point, pos[1], pos[2]);
		result3 = linePositionInY(point, pos[2], pos[0]);
		break;
	case 2:
		result1 = linePositionInZ(point, pos[0], pos[1]);
		result2 = linePositionInZ(point, pos[1], pos[2]);
		result3 = linePositionInZ(point, pos[2], pos[0]);
	}

	//if the point is up and down two lines...		
	if(((result1==UP)||(result2==UP)||(result3==UP))&&((result1==DOWN)||(result2==DOWN)||(result3==DOWN)))
	{
		return true;
	}
	//if the point is on of the lines...
	else if ((result1==ON)||(result2==ON)||(result3==ON))
	{
		return true;
	}
	else
	{
		return false;
	}
}

/** 
 * Gets the position of a point relative to a line in the x plane
 * 
 * @param point point to be tested
 * @param pointLine1 one of the line ends
 * @param pointLine2 one of the line ends
 * @return position of the point relative to the line - UP, DOWN, ON, NONE 
 */
int Face::linePositionInX(const mlVector3D & point, const mlVector3D & pointLine1, const mlVector3D & pointLine2)
{
	mlFloat a, b, z;
	if((fabs(pointLine1.y-pointLine2.y)>TOL)&&(((point.y>=pointLine1.y)&&(point.y<=pointLine2.y))||((point.y<=pointLine1.y)&&(point.y>=pointLine2.y))))
	{
		a = (pointLine2.z-pointLine1.z)/(pointLine2.y-pointLine1.y);
		b = pointLine1.z - a*pointLine1.y;
		z = a*point.y + b;
		if(z>point.z+TOL)
		{
			return UP;			
		}
		else if(z<point.z-TOL)
		{
			return DOWN;
		}
		else
		{
			return ON;
		}
	}
	else
	{
		return NONE;
	}
}

/** 
 * Gets the position of a point relative to a line in the y plane
 * 
 * @param point point to be tested
 * @param pointLine1 one of the line ends
 * @param pointLine2 one of the line ends
 * @return position of the point relative to the line - UP, DOWN, ON, NONE 
 */

int Face::linePositionInY(const mlVector3D & point, const mlVector3D & pointLine1, const mlVector3D & pointLine2)
{
	mlFloat a, b, z;
	if((fabs(pointLine1.x-pointLine2.x)>TOL)&&(((point.x>=pointLine1.x)&&(point.x<=pointLine2.x))||((point.x<=pointLine1.x)&&(point.x>=pointLine2.x))))
	{
		a = (pointLine2.z-pointLine1.z)/(pointLine2.x-pointLine1.x);
		b = pointLine1.z - a*pointLine1.x;
		z = a*point.x + b;
		if(z>point.z+TOL)
		{
			return UP;			
		}
		else if(z<point.z-TOL)
		{
			return DOWN;
		}
		else
		{
			return ON;
		}
	}
	else
	{
		return NONE;
	}
}

/** 
 * Gets the position of a point relative to a line in the z plane
 * 
 * @param point point to be tested
 * @param pointLine1 one of the line ends
 * @param pointLine2 one of the line ends
 * @return position of the point relative to the line - UP, DOWN, ON, NONE 
 */

int Face::linePositionInZ(const mlVector3D & point, const mlVector3D & pointLine1, const mlVector3D & pointLine2)
{
	mlFloat a, b, y;
	if((fabs(pointLine1.x-pointLine2.x)>TOL)&&(((point.x>=pointLine1.x)&&(point.x<=pointLine2.x))||((point.x<=pointLine1.x)&&(point.x>=pointLine2.x))))
	{
		a = (pointLine2.y-pointLine1.y)/(pointLine2.x-pointLine1.x);
		b = pointLine1.y - a*pointLine1.x;
		y = a*point.x + b;
		if(y>point.y+TOL)
		{
			return UP;			
		}
		else if(y<point.y-TOL)
		{
			return DOWN;
		}
		else
		{
			return ON;
		}
	}
	else
	{
		return NONE;
	}
}
