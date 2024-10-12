/* ベクトルの単位ベクトルを計算する	*/
#include        <math.h>
#include "ML_Types.h"

#define		ZDEPS	(1.0e-18)	/* 実変数の0ﾁｪｯｸ用微小値 */


int VectorNormalize(mlFloat invt[], mlFloat *length_p, mlFloat unitvt[])
{
	mlFloat	sum;
	mlFloat	length;

	sum = invt[0] * invt[0] + invt[1] * invt[1] + invt[2] * invt[2];	
	if( sum < ZDEPS ){
		*length_p = 0.0;
		unitvt[0] = invt[0];
		unitvt[1] = invt[1];
		unitvt[2] = invt[2];
		return -2;
	}
	else{
		length = sqrt(sum);
		mlFloat w = 1.0/length;
		unitvt[0] = invt[0] * w;
		unitvt[1] = invt[1] * w;
		unitvt[2] = invt[2] * w;
		*length_p = length;
		return  0;
	}
}
