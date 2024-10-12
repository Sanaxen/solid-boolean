/*
Double the precision of floating point operations using QD.
Many of the ideas and basic code were heavily influenced by Greg Santucci and Danilo Balby in 2008 and 2009.
*/
#include "ML_Vector.h"

#include "ML_Maths.h"


extern const mlVector3D mlVector3DX(1.0, 0.0, 0.0);
extern const mlVector3D mlVector3DY(0.0, 1.0, 0.0);
extern const mlVector3D mlVector3DZ(0.0, 0.0, 1.0);
extern const mlVector3D mlVector3DZero(0.0, 0.0, 0.0);


mlVector3D mlVectorScale(const mlVector3D &a, const mlVector3D &b)
{
	return mlVector3D(
		a.x * b.x,
		a.y * b.y,
		a.z * b.z);
}

mlVector3D	mlInterpolate(mlFloat t, const mlVector3D &a, const mlVector3D &b)
{
	return a * (1.0 - t) + b * t;
}

mlFloat mlDistanceBetween(const mlVector3D & a, const mlVector3D & b)
{
	mlVector3D difference = a - b;
	return difference.Magnitude();
}

mlVector3D mlVectorRotate(mlVector3D p, const mlVector3D &p1, const mlVector3D &p2, mlFloat theta)
{
	mlVector3D q,r;
	mlFloat costheta,sintheta;

	r.x = p2.x - p1.x;
	r.y = p2.y - p1.y;
	r.z = p2.z - p1.z;
	p.x -= p1.x;
	p.y -= p1.y;
	p.z -= p1.z;
	r.Normalise();

	costheta = cos(theta);
	sintheta = sin(theta);

	q.x += (costheta + (1.0 - costheta) * r.x * r.x) * p.x;
	q.x += ((1.0 - costheta) * r.x * r.y - r.z * sintheta) * p.y;
	q.x += ((1.0 - costheta) * r.x * r.z + r.y * sintheta) * p.z;

	q.y += ((1.0 - costheta) * r.x * r.y + r.z * sintheta) * p.x;
	q.y += (costheta + (1.0 - costheta) * r.y * r.y) * p.y;
	q.y += ((1.0 - costheta) * r.y * r.z - r.x * sintheta) * p.z;

	q.z += ((1.0 - costheta) * r.x * r.z - r.y * sintheta) * p.x;
	q.z += ((1.0 - costheta) * r.y * r.z + r.x * sintheta) * p.y;
	q.z += (costheta + (1.0 - costheta) * r.z * r.z) * p.z;

	q.x += p1.x;
	q.y += p1.y;
	q.z += p1.z;

	return q;
}

extern const mlVector4D mlVector4DZero(0.0f, 0.0f, 0.0f, 0.0f);

mlVector4D	mlVector4D::operator -  (const mlVector4D &v) const	{ return mlVector4D(x - v.x, y - v.y, z - v.z, w - v.w); }
mlVector4D	mlVector4D::operator +  (const mlVector4D &v) const	{ return mlVector4D(x + v.x, y + v.y, z + v.z, w + v.w); }
void		mlVector4D::operator += (const mlVector4D &v)		{ x += v.x; y += v.y; z += v.z; w += v.w; }
void		mlVector4D::operator *= (mlFloat s)			{ x *= s; y *= s; z *= s; w *= s; }
void		mlVector4D::SetZero()					{ x = y = z = w = 0.0f; }

mlFloat		mlVector4D::Magnitude()	const				{ return mlSqrt(x * x + y * y + z * z + w * w); }
mlFloat		mlVector4D::MagnitudeSquared()	const			{ return x * x + y * y + z * z + w * w; }

mlVector4D	operator * (const mlVector4D &v, mlFloat s)		{ return mlVector4D(v.x * s, v.y * s, v.z * s, v.w * s); }
mlVector4D	operator * (mlFloat s, const mlVector4D &v)		{ return mlVector4D(v.x * s, v.y * s, v.z * s, v.w * s); }

mlFloat		operator * (const mlVector4D &a, const mlVector4D &b)
{
	return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

void mlVector4D::Normalise()
{
	mlFloat invMag = mlInvSqrt(x*x + y*y + z*z + w*w);
	x *= invMag;
	y *= invMag;
	z *= invMag;
	w *= invMag;
}

mlVector4D	mlInterpolate(mlFloat t, const mlVector4D &a, const mlVector4D &b)
{
	return a * (1.0 - t) + b * t;
}

