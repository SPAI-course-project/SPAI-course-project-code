// Include files
#include "finddelay.h"
#include "alignsigs.h"
#include "xcorr.h"
#include <cmath>

// Function Declarations
static void vFindDelay(const double x[48000], const double y[48000], double cxx0,
    double cyy0, int* d, double* max_c);

// Function Definitions
static void vFindDelay(const double x[48000], const double y[48000], double cxx0,
    double cyy0, int* d, double* max_c)
{
    double scale;
    static double c[95999];
    *d = 0;
    *max_c = 0.0;
    scale = std::sqrt(cxx0 * cyy0);
    if (!(scale == 0.0)) {
        int index_max;
        int index_max_pos;
        int index_max_neg;
        double max_c_neg;
        int k;
        double vneg;
        index_max = 0;
        index_max_pos = 48000;
        index_max_neg = 1;
        xcorr(x, y, c);
        max_c_neg = std::abs(c[47998]) / scale;
        for (k = 0; k < 47998; k++) {
            vneg = std::abs(c[47997 - k]) / scale;
            if (vneg > max_c_neg) {
                max_c_neg = vneg;
                index_max_neg = k + 2;
            }
        }

        vneg = std::abs(c[47999]) / scale;
        for (k = 0; k < 47999; k++) {
            double vpos;
            vpos = std::abs(c[k + 48000]) / scale;
            if (vpos > vneg) {
                vneg = vpos;
                index_max_pos = k + 48001;
            }
        }

        if (vneg > max_c_neg) {
            index_max = index_max_pos;
            *max_c = vneg;
        }
        else if (vneg < max_c_neg) {
            index_max = 48000 - index_max_neg;
            *max_c = max_c_neg;
        }
        else {
            if (vneg == max_c_neg) {
                *max_c = vneg;
                if (index_max_pos - 47999 <= index_max_neg) {
                    index_max = index_max_pos;
                }
                else {
                    index_max = 48000 - index_max_neg;
                }
            }
        }

        *d = 48000 - index_max;
    }
}

int FindDelay(const double x_in[48000], const double y_in[48000])
{
    int d;
    double cxx0;
    double cyy0;
    double max_c;
    cxx0 = 0.0;
    cyy0 = 0.0;

    for (int k = 0; k < 48000; k++) {
         cxx0 += x_in[k] * x_in[k];
         cyy0 +=  y_in[k] * y_in[k];
    }

    
    vFindDelay(x_in, y_in, cxx0, cyy0, &d, &max_c);
    if (max_c < 1.0E-8) {
        d = 0;
    }

    return d;
}
