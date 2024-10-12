/*
Double the precision of floating point operations using QD.
Many of the ideas and basic code were heavily influenced by Greg Santucci and Danilo Balby in 2008 and 2009.
*/
#ifndef LINE_H
#define LINE_H


#include "ML_Vector.h"
#include "Face.h"
#include <random>
#include <time.h>

class Line
{
	mlVector3D normalFace1_;
	mlVector3D normalFace2_;
	Vertex* vrt1_;
	Vertex* vrt2_;
	mlFloat d1_;
	mlFloat d2_;

	std::mt19937 mt;
	std::uniform_real_distribution<double> d_rand;

public:

	mlVector3D point;
	mlVector3D direction;
	
	const static mlFloat TOL;
	const static mlFloat TOL2;

	inline Line(): point(mlVector3DZero), direction(mlVector3D(1,0,0))
	{
		mt.seed(time(NULL));
		d_rand = std::uniform_real_distribution<double>(-1.0, 1.0);
		//point = mlVector3DZero;
		//direction = mlVector3D(1,0,0);
	}

	inline Line(Face * face1, Face * face2)
	{
		mt.seed(time(NULL));
		d_rand = std::uniform_real_distribution<double>(-1.0, 1.0);
		normalFace1_ = face1->getNormal();
		normalFace2_ = face2->getNormal();
		vrt1_ = face1->getVertex1();
		vrt2_ = face2->getVertex1();
		d1_ = -(normalFace1_.x*vrt1_->xyz.x + normalFace1_.y*vrt1_->xyz.y + normalFace1_.z*vrt1_->xyz.z);
		d2_ = -(normalFace2_.x*vrt2_->xyz.x + normalFace2_.y*vrt2_->xyz.y + normalFace2_.z*vrt2_->xyz.z);

		creaetCrossLine();
	}

	inline Line(const mlVector3D & directioni, const mlVector3D & pointi): point(pointi), direction(directioni)
	{
		mt.seed(time(NULL));
		d_rand = std::uniform_real_distribution<double>(-1.0, 1.0);
		//direction = directioni;
		//point = pointi;
		direction.Normalise();
	}

	inline Line(Face * face1)
	{
		mt.seed(time(NULL));
		d_rand = std::uniform_real_distribution<double>(-1.0, 1.0);
		normalFace1_ = face1->getNormal();
		vrt1_        = face1->getVertex1();
		d1_ = -(normalFace1_.x*vrt1_->xyz.x + normalFace1_.y*vrt1_->xyz.y + normalFace1_.z*vrt1_->xyz.z);
	}

	inline void __fastcall creaetCrossLine(Face* face2)
	{
		normalFace2_ = face2->getNormal();
		vrt2_        = face2->getVertex1();
		d2_ = -(normalFace2_.x*vrt2_->xyz.x + normalFace2_.y*vrt2_->xyz.y + normalFace2_.z*vrt2_->xyz.z);
		creaetCrossLine();
	}

	inline void __fastcall creaetCrossLine(void)
	{
		//direction: cross product of the faces normals
		direction = mlVectorCross(normalFace1_, normalFace2_);
		mlVector3D org_direction = direction;

		mlFloat mag;
		direction.Normalise(mag);

		//mlFloat TOL = 0.00001f;
			
		//if direction lenght is not zero (the planes aren't parallel )...
		if (!(mag < TOL))
		{
			//getting a line point, zero is set to a coordinate whose direction 
			//component isn't zero (line intersecting its origin plan)
#if 10
			int c = 0;
			mlFloat val0 = Absolute(org_direction.x);
			mlFloat val1 = Absolute(org_direction.y);
			mlFloat val2 = Absolute(org_direction.z);
			mlFloat dd = org_direction.x;

			if(val0 < val1)
			{
				c = 1;
				val0 = val1;
				dd = org_direction.y;
			}
			if(val0 < val2)
			{
				c = 2;
				val0 = val2;
				dd = org_direction.z;
			}
			switch(c)
			{
				case 0:
					point.x = mlFloat(0.0);
					point.y = (d2_*normalFace1_.z - d1_*normalFace2_.z)/dd;
					point.z = (d1_*normalFace2_.y - d2_*normalFace1_.y)/dd;
					break;
				case 1:
					point.x = (d1_*normalFace2_.z - d2_*normalFace1_.z)/dd;
					point.y = mlFloat(0.0);
					point.z = (d2_*normalFace1_.x - d1_*normalFace2_.x)/dd;
					break;
				default:
					point.x = (d2_*normalFace1_.y - d1_*normalFace2_.y)/dd;
					point.y = (d1_*normalFace2_.x - d2_*normalFace1_.x)/dd;
					point.z = mlFloat(0.0);
					break;
			}
#else
			if(fabs(org_direction.x)>TOL)
			{
				point.x = mlFloat(0.0);
				point.y = (d2*normalFace1.z - d1*normalFace2.z)/org_direction.x;
				point.z = (d1*normalFace2.y - d2*normalFace1.y)/org_direction.x;
			}
			else if(fabs(org_direction.y)>TOL)
			{
				point.x = (d1*normalFace2.z - d2*normalFace1.z)/org_direction.y;
				point.y = mlFloat(0.0);
				point.z = (d2*normalFace1.x - d1*normalFace2.x)/org_direction.y;
			}
			else
			{
				point.x = (d2*normalFace1.y - d1*normalFace2.y)/org_direction.z;
				point.y = (d1*normalFace2.x - d2*normalFace1.x)/org_direction.z;
				point.z = mlFloat(0.0);
			}
#endif
		}else{
			printf("ファセットが平行接触\n");
		}
	}


	
	inline void __fastcall computePointToPointDistance(const mlVector3D & otherPoint, mlFloat& distance) const
	{
#if 0
		//mlVector3D vec = otherPoint - point;
		mlVector3D vec = mlVector3D(otherPoint.x - point.x, otherPoint.y - point.y, otherPoint.z - point.z);
		mlFloat distance;
		vec.Normalise(distance);
		if (VECTOR3D_DOT(vec, direction)<0)
		{
			return -distance;			
		}
		else
		{
			return distance;
		}
#else
		mlFloat x = otherPoint.x - point.x;
		mlFloat y = otherPoint.y - point.y;
		mlFloat z = otherPoint.z - point.z;

		distance = sqrt(x*x + y*y + z*z);
		//if ( distance != 0.0 )
		//{
		//	const mlFloat w = 1.0/distance;
		//	x *= w;
		//	y *= w;
		//	z *= w;
		//}
		if (( x*direction.x + y*direction.y + z*direction.z ) < 0.0 )
		{
			distance = -distance;			
		}
#endif
	}

	
	inline void __fastcall computeLineIntersection(Line& otherLine, mlVector3D& vResult) const
	{
		//x = x1 + a1*t = x2 + b1*s
		//y = y1 + a2*t = y2 + b2*s
		//z = z1 + a3*t = z2 + b3*s
	
		//const mlVector3D linePoint = otherLine.getPoint(); 
		//const mlVector3D lineDirection = otherLine.getDirection();
		//const mlVector3D linePoint     = otherLine.point; 
		//const mlVector3D lineDirection = otherLine.direction;

		//mlFloat TOL = 0.00001;
			
		const mlFloat& x   = direction.x;
		const mlFloat& y   = direction.y;
		const mlFloat& z   = direction.z;
		const mlFloat& px  = point.x;
		const mlFloat& py  = point.y;
		const mlFloat& pz  = point.z;
		const mlFloat& lx  = otherLine.direction.x;
		const mlFloat& ly  = otherLine.direction.y;
		const mlFloat& lz  = otherLine.direction.z;
		const mlFloat& lpx = otherLine.point.x;
		const mlFloat& lpy = otherLine.point.y;
		const mlFloat& lpz = otherLine.point.z;

		const mlFloat w[3] = { y*lx-x*ly, z*lx-x*lz, y*lz-z*ly};

#if 10
		int ii = 0;
		const mlFloat fabs_w[3] = { Absolute(w[0]), Absolute(w[1]), Absolute(w[2])};
		if ( fabs_w[ii] < fabs_w[1]) ii = 1;
		if ( fabs_w[ii] < fabs_w[2]) ii = 2;

		mlFloat t;
		switch(ii)
		{
		case 0:
			//t = (-py*lx+lpy*lx+ly*px-ly*lpx)/w[0];
			t = (lx*(lpy-py)+ly*(px-lpx))/w[0];
			break;
		case 1:
			//t=-(-lz*px+lz*lpx+lx*pz-lx*lpz)/w[1];
			t = (lz*(px-lpx)+lx*(lpz-pz))/w[1];
			break;
		case 2:
			//t = (pz*ly-lpz*ly-lz*py+lz*lpy)/w[2];
			t = (ly*(pz-lpz)+lz*(lpy-py))/w[2];
			break;
		default:
			if(fabs_w[0] > TOL)
			{
				t = (lx*(-py+lpy)+ly*(px-lpx))/w[0];
			}
			else if (fabs_w[1]>TOL)
			{
				t=-(lz*(-px+lpx)+lx*(pz-lpz))/w[1];
			}
			else if (fabs_w[2]>TOL)
			{
				t = (ly*(pz-lpz)+lz*(-py+lpy))/w[2];
			}
			else
			{
				vResult = mlVector3DZero;
			}
		}
#else
		if(fabs(w[0])>TOL)
		{
			t = (-point.y*lineDirection.x+linePoint.y*lineDirection.x+lineDirection.y*point.x-lineDirection.y*linePoint.x)/w[0];
		}
		else if (fabs(w[1])>TOL)
		{
			t=-(-lineDirection.z*point.x+lineDirection.z*linePoint.x+lineDirection.x*point.z-lineDirection.x*linePoint.z)/w[1];
		}
		else if (fabs(w[2])>TOL)
		{
			t = (point.z*lineDirection.y-linePoint.z*lineDirection.y-lineDirection.z*point.y+lineDirection.z*linePoint.y)/w[2];
		}
		else
		{
			return mlVector3DZero;
			//return null;
		}
#endif
		vResult = point + direction * t;
	}

	mlVector3D __fastcall computePlaneIntersection(const mlVector3D & normal, const mlVector3D & planePoint, bool & bResult) const;

	inline double __fastcall LineRandomNum()
	{
		return d_rand(mt);
	///*
		//int nRand = rand() % 10000;
		//double fRand = (double)nRand;
		//fRand *= 0.0001;
		//fRand *= 2.0;
		//fRand -= 1.0;
		//return fRand;
	//*/
	//	return (mlFloat)rand( ) / RAND_MAX;
	}

	inline mlVector3D __fastcall perturbDirection()
	{
		//direction.x += LineRandomNum();			
		//direction.y += LineRandomNum();			
		//direction.z += LineRandomNum();

		direction.x += LineRandomNum() * 0.0001;			
		direction.y += LineRandomNum() * 0.0001;			
		direction.z += LineRandomNum() * 0.0001;

		//direction.Normalise();
		return direction;
	}

	//Uniform random number on a sphere
	inline void __fastcall perturbDirection2(mlVector3D& dir)
	{
		mlFloat dot;
		do {
			double u = LineRandomNum();
			double v = LineRandomNum();
			double z = -2.0 * u + 1.0;
			double zz = sqrt(fabs(1.0 - z * z));

			direction.x = zz * cos(mlTwoPi * v);
			direction.y = zz * sin(mlTwoPi * v);
			direction.z = z;

			direction.Normalise();

			dot = VECTOR3D_DOT(dir, direction);
		} while (dot < 0.01);

		dir = direction;
	}

	inline void __fastcall perturbDirection(mlVector3D & dir)
	{
		//direction.x += LineRandomNum();			
		//direction.y += LineRandomNum();			
		//direction.z += LineRandomNum();

		mlFloat dot;
		do {
			direction.x += LineRandomNum() * 0.01;
			direction.y += LineRandomNum() * 0.01;
			direction.z += LineRandomNum() * 0.01;

			direction.Normalise();

			dot = VECTOR3D_DOT(dir, direction);
		} while (dot < 0.01);

		dir = direction;
	}
};

#endif // LINE_H
