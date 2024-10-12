/*
Double the precision of floating point operations using QD.
Many of the ideas and basic code were heavily influenced by Greg Santucci and Danilo Balby in 2008 and 2009.
*/
#ifndef ML_MATHS_H
#define ML_MATHS_H

#include <math.h>
#include <float.h>

#include "ML_Types.h"

const mlFloat		mlPi			= 3.14159265358979323846;
const mlFloat		mlTwoPi			= mlPi * 2.0;
const mlFloat		mlHalfPi		= mlPi * 0.5;
const mlFloat		mlInversePi		= 1.0f / mlPi;
const mlFloat 		mlHalfInversePi = mlInversePi * 0.5;

const mlFloat 		mlSqrtTwo		= 1.414213562;
const mlFloat 		mlHalfSqrtTwo	= 0.707106781;

const mlAngle		mlDegrees10 = mlPi / 18.0f;
const mlAngle		mlDegrees15 = mlPi / 12.0f;
const mlAngle		mlDegrees20 = mlPi / 9.0f;
const mlAngle		mlDegrees30 = mlPi / 6.0f;
const mlAngle		mlDegrees45 = mlPi / 4.0f;
const mlAngle		mlDegrees60 = mlPi / 3.0f;
const mlAngle		mlDegrees90 = mlPi / 2.0f;
const mlAngle		mlDegrees180 = mlPi;
const mlAngle		mlDegrees360 = mlPi * 2.0f;

const mlFloat		mlDegreesPerRadian = 180.0f / mlPi;
const mlFloat		mlRadiansPerDegree = mlPi / 180.0f;

inline mlAngle	mlDegreesToRadians(mlFloat degrees) { return degrees * mlRadiansPerDegree; }
inline mlAngle	mlRadiansToDegrees(mlFloat radians) { return radians * mlDegreesPerRadian; }

inline mlAngle	mlDegrees(mlFloat degrees)		{ return mlDegreesToRadians(degrees); }
inline mlAngle	mlRadians(mlFloat radians)		{ return radians; }

inline mlFloat	mlMin(mlFloat a, mlFloat b)		{ return (a <= b) ? a : b; }
inline mlFloat	mlMax(mlFloat a, mlFloat b)		{ return (a >= b) ? a : b; }

inline Int32	mlMin(Int32 a, Int32 b)		{ return (a <= b) ? a : b; }
inline Int32	mlMax(Int32 a, Int32 b)		{ return (a >= b) ? a : b; }

mlAngle 		mlAngleNormalise(mlAngle x);

inline bool 	mlAngleIsNormalised(mlAngle x)		{ return (x >= -mlPi) && (x <= mlPi); }

inline mlFloat	mlFabs(mlFloat x)			{ return ::fabs(x); }
inline mlFloat	mlSign(mlFloat fValue)
{
	if(fValue > 0.0f)
	{
		return 1.0f;
	}

	if(fValue < 0.0f)
	{
		return -1.0f;
	}

	return 0.0f;
}

#ifdef USE_1DOUBLE
inline mlFloat	mlSqrt(mlFloat x)			{ return sqrt(x); }
#else
inline mlFloat	mlSqrt(mlFloat& x)			{ return sqrt(x); }
#endif
inline mlFloat	mlInvSqrt(mlFloat x)		{ assert(x != 0.0); return 1.0 / sqrt(x); }
inline mlFloat	mlSin(mlAngle x)			{ return sin(x); }
inline mlFloat	mlCos(mlAngle x)			{ return cos(x); }
inline mlFloat	mlTan(mlAngle x)			{ return tan(x); }

void mlSinCos(mlAngle angle, mlFloat * sinResult, mlFloat * cosResult);

inline mlFloat	mlFloor(mlFloat x)				{ return floor(x); }
inline mlFloat	mlCeil(mlFloat x)				{ return ceil(x); }

inline mlAngle	mlArcSin(mlFloat x)				{ return asin(x); }
inline mlAngle	mlArcCos(mlFloat x)				{ return acos(x); }
inline mlAngle	mlArcTan(mlFloat x)				{ return atan(x); }
inline mlAngle	mlArcTan2(mlFloat y, mlFloat x) { return atan2(y, x); }

inline mlFloat	mlLog(mlFloat x)				{ return log(x); }
inline mlFloat	mlPow(mlFloat x, mlFloat y)		{ return pow(x, y); }
inline mlFloat	mlFmod(mlFloat x, mlFloat y)	{ return fmod(to_double(x), to_double(y)); }
inline mlFloat	mlExp(mlFloat x)				{ return exp(x); }

inline mlFloat mlClamp(mlFloat value, mlFloat min_, mlFloat max_)
{
	mlFloat	result;
	
	if (value < min_)
		result = min_;	
	else if (max_ < value)
		result = max_;
	else
		result = value;
		
	return result;
}

inline mlFloat mlFmodUniform(mlFloat x, mlFloat y)
{
	mlFloat modulo = mlFmod(x, y);

	if(modulo < 0.0f)
	{
		return modulo + y;
	}
	else
	{
		return modulo;
	}
}

inline bool mlEquivalent(mlFloat a, mlFloat b, mlFloat tolerance)
{
	mlFloat delta = a - b;
	return mlFabs(delta) < tolerance;
}

inline mlFloat mlInterpolate(mlFloat alpha, mlFloat start, mlFloat end)
{
	return (end - start) * alpha + start;
}

template <typename InType, typename OutType>
OutType mlLinearMapping(InType input, InType inLower, InType inUpper, OutType outLower, OutType outUpper)
{
	const InType inDelta = inUpper - inLower;
	const OutType outDelta = outUpper - outLower;
	mlFloat alpha = static_cast<mlFloat>(input - inLower) * (1.0f / inDelta);
	OutType outResult = (alpha * outDelta) + outLower;
	return outResult;
}

bool mlIsValid(mlFloat x);

#endif // ML_MATHS_H
