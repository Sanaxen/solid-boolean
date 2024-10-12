#ifndef _SOLID_GEOMETRY_H
#define _SOLID_GEOMETRY_H

#if _DEBUG
#ifndef _CRTDBG_MAP_ALLOC
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#define new new(_NORMAL_BLOCK, __FILE__, __LINE__)
#endif
#endif

Solid* SolidSphere( mlFloat radius, int slices, int stacks );
Solid* SolidCone( mlFloat base, mlFloat height, int slices, int stacks );
Solid* SolidCube( mlFloat width );
Solid* SolidCube( mlFloat org[3], mlFloat width );
Solid* SolidCube2( mlFloat minA[3], mlFloat maxA[3], int n );
Solid* SolidCylinder2( mlFloat base, mlFloat base2, mlFloat height, int slices, int stacks );
Solid* SolidCylinder( mlFloat base, mlFloat height, int slices, int stacks );
Solid* SolidTorus( mlFloat dInnerRadius, mlFloat dOuterRadius, int nSides, int nRings );

#endif