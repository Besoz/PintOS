#include "threads/fixed-point.h"
#include <stdint.h>

/* This file handels operations done on real numbers that is not supported
by PintOS by changing them to 17.14 representation in 32 bit */

/* This function Changes integer to fixed-point represenation */
int64_t 
int_to_fp(int n)
{
  return n*f14;
}

/* This function Changes fixed-point to integer represenation by 
rounding to zero */
int 
fp_to_int_round_zero(int64_t x)
{
  return x/f14;
}

/* This function Changes fixed-point to integer represenation by 
rounding to nearset zero */
int 
fp_to_int_round_nearset(int64_t x)
{
  if(x >= 0) return (x+f14/2)/f14;
    else return (x-f14/2)/f14;
}

/* This function adds two fixed-point numbers */
int64_t 
add_fp(int64_t x, int64_t y)
{
  return x+y;
}

/* This function subtract two fixed-point numbers */
int64_t 
sub_fp(int64_t x, int64_t y)
{
  return x-y;
}

/* This function multiply two fixed-point numbers */
int64_t 
mul_fp(int64_t x, int64_t y)
{
  return ((int64_t)x)*y/f14;
}

/* This function divides two fixed-point numbers */
int64_t 
div_fp(int64_t x, int64_t y)
{
  return ((int64_t)x)*f14/y;
}

/* This function adds a fixed-point to integer number */
int64_t 
add_fp_int(int64_t x, int n)
{
  return x+n*f14;
}

/* This function subract a fixed-point from integer number */
int64_t 
sub_fp_int(int64_t x, int n)
{
  return x-n*f14;
}

/* This function multiply a fixed-point by integer number */
int64_t 
mul_fp_int(int64_t x, int n) /* int64_t is used here to avoid overlow */
{
  return x*n;
}

/* This function divide a fixed-point from integer number */
int64_t 
div_fp_int(int64_t x, int n) /* int64_t is used here to avoid overlow */
{
  return x/n;
}
