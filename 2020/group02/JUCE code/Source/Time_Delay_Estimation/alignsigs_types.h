#ifndef ALIGNSIGS_TYPES_H
#define ALIGNSIGS_TYPES_H

// Include files
#include "rtwtypes.h"

// Type Definitions
class FFTImplementationCallback
{
 public:
  static void generate_twiddle_tables(double costab[65537], double sintab[65537]);
  static void doHalfLengthRadix2(const double x[48000], int xoffInit, creal_T y
    [131072], const double costab[65537], const double sintab[65537]);
 protected:
  static void getback_radix2_fft(creal_T y[131072], int yoff, const creal_T
    reconVar1[65536], const creal_T reconVar2[65536], const int wrapIndex[65536],
    int hnRows);
};

#endif
