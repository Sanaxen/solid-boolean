#ifndef _BITUTIL_H
#define _BITUTIL_H

#include <stdio.h>
#include <limits.h>

//ビット操作マクロ
#define BIT_VAL(n)  ((1<<n))
#define BIT_ON( a, n)  ( (a) |= (1<<n))
#define BIT_OFF( a, n) ( (a) &= ~(1<<n))
#define BIT_GET(a, n)  ( (a) & (1 << n))

#define BIT_00          (0x1)
#define BIT_01          (0x2)
#define BIT_02          (0x4)
#define BIT_03          (0x8)
#define BIT_04          (0x10)
#define BIT_05          (0x20)
#define BIT_06          (0x40)
#define BIT_07          (0x80)
#define BIT_08          (0x100)
#define BIT_09          (0x200)
#define BIT_10          (0x400)
#define BIT_11          (0x800)
#define BIT_12          (0x1000)
#define BIT_13          (0x2000)
#define BIT_14          (0x4000)
#define BIT_15          (0x8000)
#define BIT_16          (0x10000)
#define BIT_17          (0x20000)
#define BIT_18          (0x40000)
#define BIT_19          (0x80000)
#define BIT_20          (0x100000)
#define BIT_21          (0x200000)
#define BIT_22          (0x400000)
#define BIT_23          (0x800000)
#define BIT_24          (0x1000000)
#define BIT_25          (0x2000000)
#define BIT_26          (0x4000000)
#define BIT_27          (0x8000000)
#define BIT_28          (0x10000000)
#define BIT_29          (0x20000000)
#define BIT_30          (0x40000000)
#define BIT_31          (0x80000000)


/*** ビット表示関数 ***/
inline void bit_disp( unsigned int dt )
{
        int i, len;

        /* unsigned int のビット数をlenにセット (注)*/
        len = sizeof(dt) * CHAR_BIT;
        
        printf( "%#x ---> ", dt );
        for ( i = len - 1; i >= 0; i-- ){
                /* ビットの表示 */
                printf( "%u", ( dt >> i ) & 0x0001 );
        }
        printf( "\n" );
}


#endif