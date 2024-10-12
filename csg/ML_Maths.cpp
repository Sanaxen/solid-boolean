/*
Double the precision of floating point operations using QD.
Many of the ideas and basic code were heavily influenced by Greg Santucci and Danilo Balby in 2008 and 2009.
*/
#include "ML_Maths.h"

void mlSinCos(mlAngle angle, mlFloat * sinResult, mlFloat * cosResult)
{
	*sinResult = mlSin(angle);
	*cosResult = mlCos(angle);
}

mlAngle mlAngleNormalise(mlAngle x)
{
	mlFloat t = x * mlHalfInversePi + 100.5;

	mlFloat f = t - mlFloat(to_int(t));
	mlFloat result = (f - 0.5) * mlTwoPi;
	return result;
}

static UInt32 mlReinterpretFloatAsInteger(mlFloat x)
{
	return reinterpret_cast<UInt32 &>(x);
}

static bool mlInternal_IsNan(mlFloat32 x)
{
	assert(sizeof(x) == sizeof(mlFloat32));
	
	UInt32 i = mlReinterpretFloatAsInteger(x);
	
	return	((i & 0x7f800000L) == 0x7f800000L) &&
		 	((i & 0x007fffffL) != 0000000000L);
}

static bool mlInternal_IsInfinity(mlFloat32 x)
{
	assert(sizeof(x) == sizeof(mlFloat32));

	UInt32 i = mlReinterpretFloatAsInteger(x);
	
	return 	((i & 0x7f800000L) == 0x7f800000L) &&
		 	((i & 0x007fffffL) == 0000000000L);
}

bool mlIsValid(mlFloat x)
{
	return !mlInternal_IsNan(x) && !mlInternal_IsInfinity(x);
}
