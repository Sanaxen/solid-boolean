/*
Double the precision of floating point operations using QD.
Many of the ideas and basic code were heavily influenced by Greg Santucci and Danilo Balby in 2008 and 2009.
*/
#ifndef BOUND_H
#define BOUND_H

#include "sse_func.h"
#include "ML_Vector.h"

class VectorSet;
#include "VectorSet.h"
#include "tol.h"

#  pragma warning ( disable : 4244 ) /* possible loss in conversion */
#  pragma warning ( disable : 4800 ) /* 'BOOL' : ブール値を 'true' または 'false' に強制的に設定します */

class Bound
{
	bool setbound;
public:

	ALIGN16 float xyz_Max[4];
	ALIGN16 float xyz_Min[4];
	
	inline Bound():setbound(false)
	{
	}
	inline Bound(const mlVector3D & p1, const mlVector3D & p2, const mlVector3D & p3)
	{
		set(p1, p2, p3);
	}

	inline bool __fastcall isBound() const
	{
		return setbound;
	}

	inline Bound(VectorSet * vertices)
	{
		set(vertices);
	}
	inline  ~Bound(){};
	
	inline void __fastcall set(const mlVector3D & p1, const mlVector3D & p2, const mlVector3D & p3)
	{
		xyz_Max[0] = xyz_Min[0] = to_double(p1.x);
		xyz_Max[1] = xyz_Min[1] = to_double(p1.y);
		xyz_Max[2] = xyz_Min[2] = to_double(p1.z);
		
		checkVertex(p2);
		checkVertex(p3);
		setbound = true;
	}
	inline void __fastcall set(const mlFloat& x1, const mlFloat& y1, const mlFloat& z1,
					const mlFloat& x2, const mlFloat& y2, const mlFloat& z2,
					const mlFloat& x3, const mlFloat& y3, const mlFloat& z3)
	{
		xyz_Max[0] = xyz_Min[0] = to_double(x1);
		xyz_Max[1] = xyz_Min[1] = to_double(y1);
		xyz_Max[2] = xyz_Min[2] = to_double(z1);
		
		checkVertex(x2, y2, z2);
		checkVertex(x3, y3, z3);
		setbound = true;
	}

	inline void __fastcall set(VectorSet * vertices)
	{
		const mlVector3D& vtx = vertices->GetVector(0);
		xyz_Max[0] = xyz_Min[0] = to_double(vtx.x);
		xyz_Max[1] = xyz_Min[1] = to_double(vtx.y);
		xyz_Max[2] = xyz_Min[2] = to_double(vtx.z);
		
		const int sz = vertices->GetSize();
		bool omp_thread = false;

#ifdef USE_OPENMP
		if ( sz > 1000 )
		{
			omp_thread = true;
		}
#endif

		if ( omp_thread )
		{
			#ifdef USE_OPENMP
			#pragma omp parallel for
			#endif
			for(int i=1;i<sz;++i)
			{
				const mlVector3D& vertex = vertices->GetVector(i);
				const float x = to_double(vertex.x);
				const float y = to_double(vertex.y);
				const float z = to_double(vertex.z);

				if(x>xyz_Max[0])
				{
					#ifdef USE_OPENMP
					#pragma omp critical
					#endif
					{
						if(x>xyz_Max[0]) xyz_Max[0] = x;
					}
				}
				if(x<xyz_Min[0])
				{
					#ifdef USE_OPENMP
					#pragma omp critical
					#endif
					{
						if(x<xyz_Min[0]) xyz_Min[0] = x;
					}
				}
		
				if(y>xyz_Max[1])
				{
					#ifdef USE_OPENMP
					#pragma omp critical
					#endif
					{
						if(y>xyz_Max[1]) xyz_Max[1] = y;
					}
				}
				if(y<xyz_Min[1])
				{
					#ifdef USE_OPENMP
					#pragma omp critical
					#endif
					{
						if(y<xyz_Min[1]) xyz_Min[1] = y;
					}
				}
		
				if(z>xyz_Max[2])
				{
					#ifdef USE_OPENMP
					#pragma omp critical
					#endif
					{
						if(z>xyz_Max[2]) xyz_Max[2] = z;
					}
				}
				if(z<xyz_Min[2])
				{
					#ifdef USE_OPENMP
					#pragma omp critical
					#endif
					{
						if(z<xyz_Min[2]) xyz_Min[2] = z;
					}
				}
			}
		}
		else
		{
			for(int i=1;i<sz;++i)
			{
				checkVertex(vertices->GetVector(i));
			}
		}
		setbound = true;
	}

	inline bool __fastcall overlap(const Bound* bound) const
	{
		if ( !setbound ) return true;

		if(	(xyz_Min[0]>bound->xyz_Max[0]+Bound_TOL)||
			(xyz_Min[1]>bound->xyz_Max[1]+Bound_TOL)||
			(xyz_Min[2]>bound->xyz_Max[2]+Bound_TOL)||
			(xyz_Max[0]<bound->xyz_Min[0]-Bound_TOL)||
			(xyz_Max[1]<bound->xyz_Min[1]-Bound_TOL)||
			(xyz_Max[2]<bound->xyz_Min[2]-Bound_TOL)	)
		{
			return false;
		}
		return true;
	}
	inline bool __fastcall overlap(Bound & bound) const
	{
		if ( !setbound ) return true;

		if(	(xyz_Min[0]>bound.xyz_Max[0]+Bound_TOL)||
			(xyz_Min[1]>bound.xyz_Max[1]+Bound_TOL)||
			(xyz_Min[2]>bound.xyz_Max[2]+Bound_TOL)||
			(xyz_Max[0]<bound.xyz_Min[0]-Bound_TOL)||
			(xyz_Max[1]<bound.xyz_Min[1]-Bound_TOL)||
			(xyz_Max[2]<bound.xyz_Min[2]-Bound_TOL)	)
		{
			return false;
		}

		return true;
	}

private:

	inline void __fastcall checkVertex(const mlVector3D & vertex)
	{
		const float x = to_double(vertex.x);
		const float y = to_double(vertex.y);
		const float z = to_double(vertex.z);

		if(x>xyz_Max[0])
		{
			xyz_Max[0] = x;
		}
		else if(x<xyz_Min[0])
		{
			xyz_Min[0] = x;
		}
		
		if(y>xyz_Max[1])
		{
			xyz_Max[1] = y;
		}
		else if(y<xyz_Min[1])
		{
			xyz_Min[1] = y;
		}
		
		if(z>xyz_Max[2])
		{
			xyz_Max[2] = z;
		}
		else if(z<xyz_Min[2])
		{
			xyz_Min[2] = z;
		}
	}
	inline void __fastcall checkVertex(const mlFloat& x, const mlFloat& y, const mlFloat& z)
	{
		const float xx = to_double(x);
		const float yy = to_double(y);
		const float zz = to_double(z);
		xyz_Max[3] = xyz_Min[3] = 0.0f;

		if(xx>xyz_Max[0])
		{
			xyz_Max[0] = xx;
		}
		else if(xx<xyz_Min[0])
		{
			xyz_Min[0] = xx;
		}
		
		if(yy>xyz_Max[1])
		{
			xyz_Max[1] = yy;
		}
		else if(yy<xyz_Min[1])
		{
			xyz_Min[1] = yy;
		}
		
		if(zz>xyz_Max[2])
		{
			xyz_Max[2] = zz;
		}
		else if(zz<xyz_Min[2])
		{
			xyz_Min[2] = zz;
		}
	}
};

#endif // BOUND_H
