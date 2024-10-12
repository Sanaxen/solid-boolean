#ifndef _BITMACRO_H
#define _BITMACRO_H

#define BIT_01          (0x1)
#define BIT_02          (0x2)
#define BIT_03          (0x4)
#define BIT_04          (0x8)
#define BIT_05          (0x10)
#define BIT_06          (0x20)
#define BIT_07          (0x40)
#define BIT_08          (0x80)
#define BIT_09          (0x100)
#define BIT_10          (0x200)
#define BIT_11          (0x400)
#define BIT_12          (0x800)
#define BIT_13          (0x1000)
#define BIT_14          (0x2000)
#define BIT_15          (0x4000)
#define BIT_16          (0x8000)
#define BIT_17          (0x10000)
#define BIT_18          (0x20000)
#define BIT_19          (0x40000)
#define BIT_20          (0x80000)
#define BIT_21          (0x100000)
#define BIT_22          (0x200000)
#define BIT_23          (0x400000)
#define BIT_24          (0x800000)
#define BIT_25          (0x1000000)
#define BIT_26          (0x2000000)
#define BIT_27          (0x4000000)
#define BIT_28          (0x8000000)
#define BIT_29          (0x10000000)
#define BIT_30          (0x20000000)
#define BIT_31          (0x40000000)
#define BIT_32          (0x80000000)


//ビット操作マクロ
#define BIT_ON( a, n)  ( (a) |= (1<<n))
#define BIT_OFF( a, n) ( (a) &= ~(1<<n))
#define BIT_GET(a, n)  ( (a) & (1 << n))

inline void show_bit(int n)
{
  for (int i = 32 - 1; i >= 0; i--) {   /* ２進表示 */
    putchar( ((n >> i) & 0x1) ? '1' : '0' );
  }
}

//00000000000000000000000000000001
#define BIT_1_ON(a) ((a) |= 0x1)

//00000000000000000000000000000010
#define BIT_2_ON(a) ((a) |= 0x2)

//00000000000000000000000000000100
#define BIT_3_ON(a) ((a) |= 0x4)

//00000000000000000000000000001000
#define BIT_4_ON(a) ((a) |= 0x8)

//00000000000000000000000000010000
#define BIT_5_ON(a) ((a) |= 0x10)

//00000000000000000000000000100000
#define BIT_6_ON(a) ((a) |= 0x20)

//00000000000000000000000001000000
#define BIT_7_ON(a) ((a) |= 0x40)

//00000000000000000000000010000000
#define BIT_8_ON(a) ((a) |= 0x80)

//00000000000000000000000100000000
#define BIT_9_ON(a) ((a) |= 0x100)

//00000000000000000000001000000000
#define BIT_10_ON(a) ((a) |= 0x200)

//00000000000000000000010000000000
#define BIT_11_ON(a) ((a) |= 0x400)

//00000000000000000000100000000000
#define BIT_12_ON(a) ((a) |= 0x800)

//00000000000000000001000000000000
#define BIT_13_ON(a) ((a) |= 0x1000)

//00000000000000000010000000000000
#define BIT_14_ON(a) ((a) |= 0x2000)

//00000000000000000100000000000000
#define BIT_15_ON(a) ((a) |= 0x4000)

//00000000000000001000000000000000
#define BIT_16_ON(a) ((a) |= 0x8000)

//00000000000000010000000000000000
#define BIT_17_ON(a) ((a) |= 0x10000)

//00000000000000100000000000000000
#define BIT_18_ON(a) ((a) |= 0x20000)

//00000000000001000000000000000000
#define BIT_19_ON(a) ((a) |= 0x40000)

//00000000000010000000000000000000
#define BIT_20_ON(a) ((a) |= 0x80000)

//00000000000100000000000000000000
#define BIT_21_ON(a) ((a) |= 0x100000)

//00000000001000000000000000000000
#define BIT_22_ON(a) ((a) |= 0x200000)

//00000000010000000000000000000000
#define BIT_23_ON(a) ((a) |= 0x400000)

//00000000100000000000000000000000
#define BIT_24_ON(a) ((a) |= 0x800000)

//00000001000000000000000000000000
#define BIT_25_ON(a) ((a) |= 0x1000000)

//00000010000000000000000000000000
#define BIT_26_ON(a) ((a) |= 0x2000000)

//00000100000000000000000000000000
#define BIT_27_ON(a) ((a) |= 0x4000000)

//00001000000000000000000000000000
#define BIT_28_ON(a) ((a) |= 0x8000000)

//00010000000000000000000000000000
#define BIT_29_ON(a) ((a) |= 0x10000000)

//00100000000000000000000000000000
#define BIT_30_ON(a) ((a) |= 0x20000000)

//01000000000000000000000000000000
#define BIT_31_ON(a) ((a) |= 0x40000000)

//10000000000000000000000000000000
#define BIT_32_ON(a) ((a) |= 0x80000000)

//11111111111111111111111111111110
#define BIT_1_OFF(a) ((a) &= 0xfffffffe)

//11111111111111111111111111111101
#define BIT_2_OFF(a) ((a) &= 0xfffffffd)

//11111111111111111111111111111011
#define BIT_3_OFF(a) ((a) &= 0xfffffffb)

//11111111111111111111111111110111
#define BIT_4_OFF(a) ((a) &= 0xfffffff7)

//11111111111111111111111111101111
#define BIT_5_OFF(a) ((a) &= 0xffffffef)

//11111111111111111111111111011111
#define BIT_6_OFF(a) ((a) &= 0xffffffdf)

//11111111111111111111111110111111
#define BIT_7_OFF(a) ((a) &= 0xffffffbf)

//11111111111111111111111101111111
#define BIT_8_OFF(a) ((a) &= 0xffffff7f)

//11111111111111111111111011111111
#define BIT_9_OFF(a) ((a) &= 0xfffffeff)

//11111111111111111111110111111111
#define BIT_10_OFF(a) ((a) &= 0xfffffdff)

//11111111111111111111101111111111
#define BIT_11_OFF(a) ((a) &= 0xfffffbff)

//11111111111111111111011111111111
#define BIT_12_OFF(a) ((a) &= 0xfffff7ff)

//11111111111111111110111111111111
#define BIT_13_OFF(a) ((a) &= 0xffffefff)

//11111111111111111101111111111111
#define BIT_14_OFF(a) ((a) &= 0xffffdfff)

//11111111111111111011111111111111
#define BIT_15_OFF(a) ((a) &= 0xffffbfff)

//11111111111111110111111111111111
#define BIT_16_OFF(a) ((a) &= 0xffff7fff)

//11111111111111101111111111111111
#define BIT_17_OFF(a) ((a) &= 0xfffeffff)

//11111111111111011111111111111111
#define BIT_18_OFF(a) ((a) &= 0xfffdffff)

//11111111111110111111111111111111
#define BIT_19_OFF(a) ((a) &= 0xfffbffff)

//11111111111101111111111111111111
#define BIT_20_OFF(a) ((a) &= 0xfff7ffff)

//11111111111011111111111111111111
#define BIT_21_OFF(a) ((a) &= 0xffefffff)

//11111111110111111111111111111111
#define BIT_22_OFF(a) ((a) &= 0xffdfffff)

//11111111101111111111111111111111
#define BIT_23_OFF(a) ((a) &= 0xffbfffff)

//11111111011111111111111111111111
#define BIT_24_OFF(a) ((a) &= 0xff7fffff)

//11111110111111111111111111111111
#define BIT_25_OFF(a) ((a) &= 0xfeffffff)

//11111101111111111111111111111111
#define BIT_26_OFF(a) ((a) &= 0xfdffffff)

//11111011111111111111111111111111
#define BIT_27_OFF(a) ((a) &= 0xfbffffff)

//11110111111111111111111111111111
#define BIT_28_OFF(a) ((a) &= 0xf7ffffff)

//11101111111111111111111111111111
#define BIT_29_OFF(a) ((a) &= 0xefffffff)

//11011111111111111111111111111111
#define BIT_30_OFF(a) ((a) &= 0xdfffffff)

//10111111111111111111111111111111
#define BIT_31_OFF(a) ((a) &= 0xbfffffff)

//01111111111111111111111111111111
#define BIT_32_OFF(a) ((a) &= 0x7fffffff)

#endif