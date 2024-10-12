#ifndef _DEBUG_PRINT_H
#define _DEBUG_PRINT_H

#define DEBUG_PRINT_ON	0

#if DEBUG_PRINT_ON

#define DEBUG_PRINT( s )	printf s

#else

#define DEBUG_PRINT( s )	/* empty */

#endif

//#define DEBUG_PRINT_D( s )	printf s
#define DEBUG_PRINT_D( s )	/* empty */

#endif