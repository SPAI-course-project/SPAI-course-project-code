#ifndef XCORR_H
#define XCORR_H

// Include files
#include <cstddef>
#include <cstdlib>
#include "rtwtypes.h"
#include "alignsigs_types.h"

// Function Declarations
extern void xcorr(const double x[48000], const double varargin_1[48000], double
                  c[95999]);

#endif