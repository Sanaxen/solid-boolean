/* 
Fast Ray-Box Intersection
by Andrew Woo
from "Graphics Gems", Academic Press, 1990
*/
#include <math.h>

#define EPS		0.01f
#define RIGHT	0
#define LEFT	1
#define MIDDLE	2

int RayBoxIntersection(float minB[3], float maxB[3], float origin[3], float dir[3])
//float minB[3], maxB[3];		/*box */
//float origin[3], dir[3];		/*ray */
//float coord[3];				/* hit point */
{
	char inside = 1;
	char quadrant[3];
	int whichPlane;
	float maxT[3];
	float candidatePlane[3];
	float coord[3];

	/* Find candidate planes; this loop can be avoided if
   	rays cast all from the eye(assume perpsective view) */
	if(origin[0] < minB[0]) {
		quadrant[0] = LEFT;
		candidatePlane[0] = minB[0];
		inside = 0;
	}else if (origin[0] > maxB[0]) {
		quadrant[0] = RIGHT;
		candidatePlane[0] = maxB[0];
		inside = 0;
	}else	{
		quadrant[0] = MIDDLE;
	}

	if(origin[1] < minB[1]) {
		quadrant[1] = LEFT;
		candidatePlane[1] = minB[1];
		inside = 0;
	}else if (origin[1] > maxB[1]) {
		quadrant[1] = RIGHT;
		candidatePlane[1] = maxB[1];
		inside = 0;
	}else	{
		quadrant[1] = MIDDLE;
	}

	if(origin[2] < minB[2]) {
		quadrant[2] = LEFT;
		candidatePlane[2] = minB[2];
		inside = 0;
	}else if (origin[2] > maxB[2]) {
		quadrant[2] = RIGHT;
		candidatePlane[2] = maxB[2];
		inside = 0;
	}else	{
		quadrant[2] = MIDDLE;
	}

	/* Ray origin inside bounding box */
	if(inside)	{
		return (1);
	}


	/* Calculate T distances to candidate planes */
	if (quadrant[0] != MIDDLE && fabs(dir[0]) > EPS)
		maxT[0] = (candidatePlane[0]-origin[0]) / dir[0];
	else
		maxT[0] = -1.;

	if (quadrant[1] != MIDDLE && fabs(dir[1]) > EPS)
		maxT[1] = (candidatePlane[1]-origin[1]) / dir[1];
	else
		maxT[1] = -1.;

	if (quadrant[2] != MIDDLE && fabs(dir[2]) > EPS)
		maxT[2] = (candidatePlane[2]-origin[2]) / dir[2];
	else
		maxT[2] = -1.;

	/* Get largest of the maxT's for final choice of intersection */
	whichPlane = 0;
	if (maxT[whichPlane] < maxT[1])
		whichPlane = 1;
	if (maxT[whichPlane] < maxT[2])
		whichPlane = 2;

	/* Check final candidate actually inside box */
	if (maxT[whichPlane] < 0.) return (0);

	if (whichPlane != 0) {
		coord[0] = origin[0] + maxT[whichPlane] *dir[0];
		if (coord[0] < minB[0] || coord[0] > maxB[0])
			return (0);
	}

	if (whichPlane != 1) {
		coord[1] = origin[1] + maxT[whichPlane] *dir[1];
		if (coord[1] < minB[1] || coord[1] > maxB[1])
			return (0);
	}

	if (whichPlane != 2) {
		coord[2] = origin[2] + maxT[whichPlane] *dir[2];
		if (coord[2] < minB[2] || coord[2] > maxB[2])
			return (0);
	} 

	return (1);				/* ray hits box */
}	

