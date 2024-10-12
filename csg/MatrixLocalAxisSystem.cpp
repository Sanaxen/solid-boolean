/*	�_�ƍ��W�������W�ϊ��s����쐬����			*/

#include	<math.h>
#include "ML_Types.h"
#include "ML_Vector.h"

void VectorOuterProduct(mlFloat invt1[3], mlFloat invt2[3], mlFloat outer[3]);
int	MatrixInverse(int dij, mlFloat *inmx_p, mlFloat *invmx_p);
int VectorNormalize( mlFloat invt[], mlFloat *length_p, mlFloat unitvt[]);

#define	X		0
#define	Y		1
#define	Z		2

/*
 * �G���g��
 */
int	MatrixLocalAxisSystem (mlFloat point[3], mlFloat nvec[3], mlFloat base_x[3], mlFloat matrix[4][4], mlFloat matinv[4][4])
{

	/* �����ϐ��錾 */
	int		status;		/* �֐��߂�l		*/
	int		stat;		/* �֐��߂�l		*/
	int		i;		/* ���[�v�ϐ�		*/
	int		j;		/* ���[�v�ϐ�		*/
	mlFloat		lbase_x;	/* �w���x�N�g����	*/
	mlFloat		lnvec;		/* �y���x�N�g����	*/
	mlFloat		ubase_x[3];	/* �w���P�ʃx�N�g��	*/
	mlFloat		ubase_y[3];	/* �x���P�ʃx�N�g��	*/
	mlFloat		unvec[3];	/* �y���P�ʃx�N�g��	*/


	/*
	 * ��������
	 */
	status	= 0;
	matrix[0][0] = matrix[0][1] = matrix[0][2] = matrix[0][3] = 0.0;
	matrix[1][0] = matrix[1][1] = matrix[1][2] = matrix[1][3] = 0.0;
	matrix[2][0] = matrix[2][1] = matrix[2][2] = matrix[2][3] = 0.0;
	matrix[3][0] = matrix[3][1] = matrix[3][2] = matrix[3][3] = 0.0;

	/*
	 * ���̓p�����[�^�`�F�b�N
	 */
		/* nvec �̃`�F�b�N	*/
	if ( VectorNormalize(nvec, &lnvec, unvec ) != 0 )
	{
		status = -1;
		return( status );
	}

		/* base_x �̃`�F�b�N	*/
	if ( VectorNormalize( base_x, &lbase_x, ubase_x ) != 0 )
	{
		status = -1;
		return( status );
	}

	/*
	 * �x�������߂�
	 */
	VectorOuterProduct( unvec, ubase_x, ubase_y );

	/*
	 * �ϊ��s����쐬����D
	 */
	matrix[0][0]	= ubase_x[0];
	matrix[1][0]	= ubase_y[0];
	matrix[2][0]	= unvec  [0];

	matrix[0][1]	= ubase_x[1];
	matrix[1][1]	= ubase_y[1];
	matrix[2][1]	= unvec  [1];

	matrix[0][2]	= ubase_x[2];
	matrix[1][2]	= ubase_y[2];
	matrix[2][2]	= unvec  [2];

	matrix[0][3]	= -( ubase_x[0]*point[X] +ubase_x[1]*point[Y] +ubase_x[2]*point[Z] );
	matrix[1][3]	= -( ubase_y[0]*point[X] +ubase_y[1]*point[Y] +ubase_y[2]*point[Z] );
	matrix[2][3]	= -( unvec  [0]*point[X] +unvec  [1]*point[Y] +unvec  [2]*point[Z] );
	matrix[3][3]	= 1.0;

	/*
	 * �t�s����쐬����
	 */
	stat	= MatrixInverse( 4, (mlFloat *)&matrix[0][0], (mlFloat *)&matinv[0][0] );
	if ( stat < 0 ){
		status = -1;
		return( status );
	}
	return( status );
}

int	MatrixLocalAxisSystem2 ( mlFloat nvec[3],  mlFloat matrix[4][4], int invmx_calc, mlFloat matinv[4][4])
{
	mlFloat point[3];
	mlFloat base_x[3];

	point[0] = 0.0;
	point[1] = 0.0;
	point[2] = 0.0;

	base_x[0] = 0.0;
	base_x[1] = 1.0;
	base_x[2] = 0.0;

	mlVector3D n(nvec[0],nvec[1],nvec[2]);
	mlVector3D x(base_x[0],base_x[1],base_x[2]);

	mlVector3D y = mlVectorCross(n, x);
	y.Normalise();
	if ( Absolute(y.MagnitudeSquared() - 1.0) > 0.01 )
	{
		x.x = 1.0;
		x.y = 0.0;
		x.z = 0.0;
		y = mlVectorCross(n, x);
		y.Normalise();
		if ( Absolute(y.MagnitudeSquared() - 1.0) > 0.01 )
		{
			x.x = 1.0;
			x.y = 0.0;
			x.z = 0.0;
			y = mlVectorCross(n, x);
			y.Normalise();
			if ( Absolute(y.MagnitudeSquared() - 1.0) > 0.01 )
			{
				return -1;
			}
		}
	}
	base_x[0] = y.x;
	base_x[1] = y.y;
	base_x[2] = y.z;

	return MatrixLocalAxisSystem (point, nvec, base_x, matrix, matinv);
}