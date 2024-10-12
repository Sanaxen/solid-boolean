/*
Double the precision of floating point operations using QD.
Many of the ideas and basic code were heavily influenced by Greg Santucci and Danilo Balby in 2008 and 2009.
*/
#ifndef ML_VECTOR_H
#define ML_VECTOR_H


#include "ML_Types.h"
#include "ML_Maths.h"

class mlVector3D
{
public:

	mlFloat		x;
	mlFloat 	y;
	mlFloat		z;

	inline mlVector3D():x(0.0),y(0.0),z(0.0)
	{}
	inline mlVector3D(const mlFloat& xv, const mlFloat& yv, const mlFloat& zv) : x(xv), y(yv), z(zv)
	{}

	inline void	__fastcall SetZero()					{ x = y = z = 0.0; }
	inline void	__fastcall Set(const mlFloat& xv, const mlFloat& yv, const mlFloat& zv)	{ x = xv; y = yv; z = zv; }
	
	inline float* toFloat(float* v)
	{
#ifdef USE_QDOUBLE
		v[0] = to_double(x);
		v[1] = to_double(y);
		v[2] = to_double(z);
#else
		v[0] = x;
		v[1] = y;
		v[2] = z;
#endif
		return v;
	}

	inline void	__fastcall Normalise()
	{
		const mlFloat mag = Magnitude();
	
		if(mag == 0.0)
		{
	//		printf("Normalise error.\n");
			return;
		}

		const mlFloat invMag = 1.0 / mag;
		x *= invMag;
		y *= invMag;
		z *= invMag;
	}

	inline void __fastcall Normalise(mlFloat& mag)
	{
		mag = Magnitude();
	
		if(mag == 0.0)
		{
	//		printf("Normalise error.\n");
			return;
		}

		const mlFloat invMag = 1.0 / mag;
		x *= invMag;
		y *= invMag;
		z *= invMag;
	}
	
	// lengは二乗の値がセットされて返る
	inline bool __fastcall NormaliseChk(const mlFloat& tol, mlFloat& leng)
	{
		leng = Magnitude();
	
		if(leng <= tol)
		{
	//		printf("Normalise error.\n");
			return false;
		}
		return true;
	}
	

	inline mlVector3D __fastcall Normalised() const
	{
		mlVector3D normalisedVector(x,y,z);
	
		//mlFloat maxValue = 0.0f;
		//
		//if(mlFabs(normalisedVector.x) > maxValue) maxValue = mlFabs(normalisedVector.x);
		//if(mlFabs(normalisedVector.y) > maxValue) maxValue = mlFabs(normalisedVector.y);
		//if(mlFabs(normalisedVector.z) > maxValue) maxValue = mlFabs(normalisedVector.z);
		//
		//if(maxValue == 0.0f)
		//	return mlVector3D(x,y,z);
		//	
		//normalisedVector.x = normalisedVector.x / maxValue;
		//normalisedVector.y = normalisedVector.y / maxValue;
		//normalisedVector.z = normalisedVector.z / maxValue;
	
		const mlFloat mag = normalisedVector.Magnitude();
	
		if(mag == 0.0)
			return mlVector3D(x,y,z);
	
		const mlFloat invMag = 1.0 / mag;
		normalisedVector.x *= invMag;
		normalisedVector.y *= invMag;
		normalisedVector.z *= invMag;
	
		return normalisedVector;
	}


	inline mlFloat		__fastcall Magnitude()	const			{ return sqrt(x * x + y * y + z * z); }
	inline mlFloat		__fastcall MagnitudeSquared()	const	{ return x * x + y * y + z * z; }

	inline mlVector3D	__fastcall operator +  (const mlVector3D &v) const	{ return mlVector3D(x + v.x, y + v.y, z + v.z); }
	inline mlVector3D	__fastcall operator -  (const mlVector3D &v) const	{ return mlVector3D(x - v.x, y - v.y, z - v.z); }
	inline void			__fastcall operator += (const mlVector3D &v)		{ x += v.x; y += v.y; z += v.z; }
	inline void			__fastcall operator -= (const mlVector3D &v)		{ x -= v.x; y -= v.y; z -= v.z; }
	inline void			__fastcall operator *= (mlFloat s)			{ x *= s; y *= s; z *= s; }
	inline mlVector3D	__fastcall operator -	() const			{ return mlVector3D(-x, -y, -z);}


	inline bool	__fastcall operator==(const mlVector3D &other) const
	{
		return (x == other.x) && (y == other.y) && (z == other.z);
	}
	inline bool	__fastcall operator!=(const mlVector3D &other) const
	{
		return (x != other.x) || (y != other.y) || (z != other.z);
	}
	inline mlFloat&	__fastcall operator[](const int index)
	{
		return reinterpret_cast<mlFloat*>(this)[index];
	}

	inline const mlFloat&	__fastcall operator[](const int index) const
	{
		return reinterpret_cast<const mlFloat*>(this)[index];
	}
};



#define VECTOR3D_SQR(a)    (a.x * a.x + a.y * a.y + a.z * a.z)
#define VECTOR3D_DOT(a, b) (a.x * b.x + a.y * b.y + a.z * b.z)
inline mlFloat	__fastcall operator * (const mlVector3D &a, const mlVector3D &b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}
inline mlFloat __fastcall mlVectorDot(const mlVector3D &a, const mlVector3D &b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

#define VECTOR3D_CROSS(a, b) (mlVector3D(a.y * b.z - a.z * b.y,	a.z * b.x - a.x * b.z,a.x * b.y - a.y * b.x))
inline mlVector3D __fastcall mlVectorCross(const mlVector3D &a, const mlVector3D &b)
{
	return mlVector3D(
				a.y * b.z - a.z * b.y,
				a.z * b.x - a.x * b.z,
				a.x * b.y - a.y * b.x
			);
}

inline mlVector3D __fastcall operator * (const mlVector3D &v, const mlFloat& s){ return mlVector3D(v.x * s, v.y * s, v.z * s); }
inline mlVector3D __fastcall operator * (const mlFloat& s, const mlVector3D &v){ return mlVector3D(v.x * s, v.y * s, v.z * s); }


mlVector3D mlVectorScale(const mlVector3D &a, const mlVector3D &b);
mlVector3D mlVectorRotate(mlVector3D p, const mlVector3D &p1, const mlVector3D &p2, mlFloat theta);




mlVector3D	mlInterpolate(mlFloat t, const mlVector3D &a, const mlVector3D &b);

mlFloat		mlDistanceBetween(const mlVector3D & a, const mlVector3D & b);

extern const mlVector3D mlVector3DX;
extern const mlVector3D mlVector3DY;
extern const mlVector3D mlVector3DZ;

extern const mlVector3D mlVector3DZero;


class mlVector4D
{
public:
	mlFloat	x;
	mlFloat y;
	mlFloat	z;
	mlFloat	w;
	
	mlVector4D	() { x = 0.0f; y = 0.0f; z = 0.0f; w = 0.0f; }
	mlVector4D	(mlFloat xv, mlFloat yv, mlFloat zv, mlFloat wv);
	mlVector4D	(const mlVector3D & v, mlFloat w);
	
	void			SetZero();
	void			Normalise();
	
	mlFloat			Magnitude()	const;
	mlFloat			MagnitudeSquared() const;
	
	void			operator *= (mlFloat s);
	mlVector4D		operator -  (const mlVector4D &v) const;
	mlVector4D		operator +  (const mlVector4D &v) const;
	void			operator += (const mlVector4D &v);
	
	mlFloat &		operator[](int index);
	const mlFloat &	operator[](int index) const;

};

inline mlVector4D::mlVector4D(mlFloat xv, mlFloat yv, mlFloat zv, mlFloat wv) : x(xv), y(yv), z(zv), w(wv)	{}
inline mlVector4D::mlVector4D(const mlVector3D & v, mlFloat w) : x(v.x), y(v.y), z(v.z), w(w) 			{}

inline mlFloat&	mlVector4D::operator[](int index)
{
	return reinterpret_cast<mlFloat*>(this)[index];
}

inline const mlFloat&	mlVector4D::operator[](int index) const
{
	return reinterpret_cast<const mlFloat*>(this)[index];
}

mlFloat		operator * (const mlVector4D &a, const mlVector4D &b);
mlVector4D	operator * (const mlVector4D &v, mlFloat s);
mlVector4D	operator * (mlFloat s, const mlVector4D &v);

mlVector4D	mlInterpolate(mlFloat t, const mlVector4D &a, const mlVector4D &b);

extern const mlVector4D mlVector4DZero;

inline bool							mlEquivalent(const mlVector3D & a, const mlVector3D & b, mlFloat tolerance)
{
	mlFloat deltax = a.x - b.x;
	mlFloat deltay = a.y - b.y;
	mlFloat deltaz = a.z - b.z;

	return	mlFabs(deltax) < tolerance &&
			mlFabs(deltay) < tolerance &&
			mlFabs(deltaz) < tolerance;
}

inline bool							mlEquivalent(const mlVector4D & a, const mlVector4D & b, mlFloat tolerance)
{
	mlFloat deltax = a.x - b.x;
	mlFloat deltay = a.y - b.y;
	mlFloat deltaz = a.z - b.z;
	mlFloat deltaw = a.w - b.w;

	return	mlFabs(deltax) < tolerance &&
			mlFabs(deltay) < tolerance &&
			mlFabs(deltaz) < tolerance &&
			mlFabs(deltaw) < tolerance;
}

inline bool mlIsValid(const mlVector3D & v)
{
	return	mlIsValid(v.x) &&
			mlIsValid(v.y) &&
			mlIsValid(v.z);
}

inline bool mlIsValid(const mlVector4D & v)
{
	return	mlIsValid(v.x) &&
			mlIsValid(v.y) &&
			mlIsValid(v.z) &&
			mlIsValid(v.w);
}


#endif // ML_VECTOR_H
