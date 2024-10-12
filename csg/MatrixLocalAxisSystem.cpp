/*	点と座標軸より座標変換行列を作成する			*/

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
 * エントリ
 */
int	MatrixLocalAxisSystem (mlFloat point[3], mlFloat nvec[3], mlFloat base_x[3], mlFloat matrix[4][4], mlFloat matinv[4][4])
{

	/* 内部変数宣言 */
	int		status;		/* 関数戻り値		*/
	int		stat;		/* 関数戻り値		*/
	int		i;		/* ループ変数		*/
	int		j;		/* ループ変数		*/
	mlFloat		lbase_x;	/* Ｘ軸ベクトル長	*/
	mlFloat		lnvec;		/* Ｚ軸ベクトル長	*/
	mlFloat		ubase_x[3];	/* Ｘ軸単位ベクトル	*/
	mlFloat		ubase_y[3];	/* Ｙ軸単位ベクトル	*/
	mlFloat		unvec[3];	/* Ｚ軸単位ベクトル	*/


	/*
	 * 初期処理
	 */
	status	= 0;
	matrix[0][0] = matrix[0][1] = matrix[0][2] = matrix[0][3] = 0.0;
	matrix[1][0] = matrix[1][1] = matrix[1][2] = matrix[1][3] = 0.0;
	matrix[2][0] = matrix[2][1] = matrix[2][2] = matrix[2][3] = 0.0;
	matrix[3][0] = matrix[3][1] = matrix[3][2] = matrix[3][3] = 0.0;

	/*
	 * 入力パラメータチェック
	 */
		/* nvec のチェック	*/
	if ( VectorNormalize(nvec, &lnvec, unvec ) != 0 )
	{
		status = -1;
		return( status );
	}

		/* base_x のチェック	*/
	if ( VectorNormalize( base_x, &lbase_x, ubase_x ) != 0 )
	{
		status = -1;
		return( status );
	}

	/*
	 * Ｙ軸を求める
	 */
	VectorOuterProduct( unvec, ubase_x, ubase_y );

	/*
	 * 変換行列を作成する．
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
	 * 逆行列を作成する
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