#ifndef _RAYVECTOR_H
#define _RAYVECTOR_H

#if 10
#define double_ex	float
#define to_double__(x)	to_double(x)	
#else
#define double_ex	mlFloat
#define to_double__(x)	x	
#endif

class mlVector3D;

class rayVector
{
public:
	double_ex orig[3];
	double_ex dir[3];
	inline rayVector(mlVector3D& org, mlVector3D& direction)
	{
		orig[0] = to_double__(org.x);
		orig[1] = to_double__(org.y);
		orig[2] = to_double__(org.z);
		dir[0] = to_double__(direction.x);
		dir[1] = to_double__(direction.y);
		dir[2] = to_double__(direction.z);

	}
};

#endif
