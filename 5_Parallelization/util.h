#ifndef _util_h
#define _util_h

/**
 * C's mod ('%') operator is mathematically correct, but it may return
 * a negative remainder even when both inputs are nonnegative.  This
 * function always returns a nonnegative remainder (x mod m), as long
 * as x and m are both positive.  This is helpful for computing
 * toroidal boundary conditions.
 */
static inline int 
mod (int x, int m)
{
  int result = (x < 0) ? ((x % m) + m) : (x % m);
  //printf("x is %d, m is %d, result is %d\n",x, m , result);
  return result;
}

/**
 * Given neighbor count and current state, return zero if cell will be
 * dead, or nonzero if cell will be alive, in the next round. */

 // Here, we changed comparison operator from AND to OR.

static inline char 
alivep (char count, char state)
{
  return (! state && (count == (char) 3)) ||
    (state && (count == 2 || count == 3));  //   <----- modified code
}

#endif /* _util_h */
