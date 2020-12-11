#ifndef FFT_H
#define FFT_H

// Include files
#include <cstddef>
#include <cstdlib>
#include "rtwtypes.h"
#include "alignsigs_types.h"

// Function Declarations
extern void fft(const double x[48000], creal_T y[131072]);

#endif
