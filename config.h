#ifndef UL_CONFIG_H
#define UL_CONFIG_H

/**
 * To support long double uncomment the following line
 */
//#define UL_HAS_LONG_DOUBLE

// Don't change anything beyond this line
//-----------------------------------------------------------------------------

#ifdef UL_HAS_LONG_DOUBLE

typedef long double ul_number;
#define _strton strtold
#define _fabsn  fabsl
#define _sqrtn  sqrtl
#define _pown   pow

#define N_FMT     "%Lg"
#define N_EPSILON LDBL_EPSILON

#else

typedef double ul_number;
#define _strton strtod
#define _fabsn  fabs
#define _sqrtn  sqrt
#define _pown   powl

#define N_FMT     "%g"
#define N_EPSILON DBL_EPSILON

#endif

#endif /*UL_CONFIG_H*/
