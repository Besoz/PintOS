#ifndef THREADS_FIXED-POINT_H
#define THREADS_FIXED-POINT_H
#include <stdint.h>

#define f14 16384                 /* using 17.14  signed 32 bit int f14 = 1 << 14 */

int64_t int_to_fp(int);
int fp_to_int_round_zero(int64_t);
int fp_to_int_round_nearset(int64_t);
int64_t add_fp(int64_t, int64_t);
int64_t sub_fp(int64_t, int64_t);
int64_t mul_fp(int64_t, int64_t);
int64_t div_fp(int64_t, int64_t);
int64_t add_fp_int(int64_t, int);
int64_t sub_fp_int(int64_t, int);
int64_t mul_fp_int(int64_t, int);
int64_t div_fp_int(int64_t, int);

#endif
