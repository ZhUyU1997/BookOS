#ifndef _MATH_H_
#define _MATH_H_
#include "kernel/types.h"

/* max() & min() */
#define	max(a,b)	((a) > (b) ? (a) : (b))
#define	min(a,b)	((a) < (b) ? (a) : (b))

#define DIV_ROUND_UP(X, STEP) ((X + STEP - 1) / (STEP))

#endif
