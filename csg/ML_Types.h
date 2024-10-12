/*
Double the precision of floating point operations using QD.
Many of the ideas and basic code were heavily influenced by Greg Santucci and Danilo Balby in 2008 and 2009.
*/
#ifndef ML_TYPES_H
#define ML_TYPES_H

#include <stddef.h>

#include <assert.h>
#pragma warning ( disable : 4800 ) /* 'BOOL' : ブール値を 'true' または 'false' に強制的に設定します */

//#define USE_1DOUBLE
//#define USE_QDOUBLE
#define USE_DDOUBLE

#ifdef USE_QDOUBLE
#include "qd/qd_real.h"
#endif

#ifdef USE_DDOUBLE
#include "qd/dd_real.h"
#endif



#ifdef USE_QDOUBLE
typedef	qd_real			mlFloat32;
typedef qd_real			mlFloat64;
#define	Absolute(x)		abs((x))
#define ToFloat(x)		static_cast<float>(to_double(x))
#define ToString(x)		(x).to_string()
#define _CVTBUFFFER		/* */
#endif

#ifdef USE_DDOUBLE
typedef	dd_real			mlFloat32;
typedef dd_real			mlFloat64;
#define	Absolute(x)		abs((x))
#define ToFloat(x)		static_cast<float>(to_double(x))
#define _CVTBUFFFER		/* */
#define ToString(x)		(x).to_string()
#endif

#ifdef USE_1DOUBLE
typedef	double			mlFloat32;
typedef double			mlFloat64;
#define	Absolute(x)		fabs((x))
#define ToFloat(x)		static_cast<float>(x)
#define to_double(x)	(x)
#define to_int(x)	(x)
#define _CVTBUFFFER		char buffer[_CVTBUFSIZE]
#define ToString(x)		std::string(_gcvt( (x), 12, buffer ))
#define fpu_fix_start(x)	/* */
#define fpu_fix_end(x)		/* */
#endif

typedef mlFloat32		mlFloat;
typedef mlFloat64		mlDouble;

typedef mlFloat			mlAngle;

const mlFloat			mlFloatMax = 3.402823466e+38f * 0.1f;
const mlFloat			mlFloatMin = 1.175494351e-38f * 10.0f;

typedef unsigned char		UInt8,	U08;
typedef signed char		Int8,	S08;

typedef unsigned short		UInt16,	U16;
typedef signed short		Int16,	S16;

typedef unsigned int		UInt32,	U32;
typedef signed int		Int32,	S32;

enum mlAxis
{
	mlAxisX,
	mlAxisY,
	mlAxisZ
};

#endif // ML_TYPES_H
