/*
Double the precision of floating point operations using QD.
Many of the ideas and basic code were heavily influenced by Greg Santucci and Danilo Balby in 2008 and 2009.
*/

#ifndef BOOLEANMODELLER_H
#define BOOLEANMODELLER_H

#include "Object3D.h"
#include "Solid.h"
#include "Face.h"

class Object3D;
class Solid;

class BooleanModeller
{
public:

	bool create_status;
	bool split_status;
	bool time_out;

	bool default_color1;
	bool default_color2;
	Object3D * m_pObject1;
	Object3D * m_pObject2;
	
	BooleanModeller(Solid * solid1, Solid * solid2);
	virtual ~BooleanModeller();

	Solid * get1();
	Solid * get2();
	Solid * getUnion();
	Solid * getIntersection();
	Solid * getDifference();

private:
	void Object_Object_Interference();

	Solid * composeSolid(int faceStatus1, int faceStatus2, int faceStatus3, int op);
	
	void groupObjectComponents(Object3D* pObject, VertexSet & vertices, IntSet & indices, ColorSet& colors, int faceStatus1, int faceStatus2, int op);
};

#endif // BOOLEANMODELLER_H
