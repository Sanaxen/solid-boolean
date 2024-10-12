#ifndef _OPENMP_COMFIG_H

#define _OPENMP_COMFIG_H

#ifdef _OPENMP
#define USE_OPENMP
#endif

#ifdef USE_OPENMP
#include <omp.h>
#endif

#endif
