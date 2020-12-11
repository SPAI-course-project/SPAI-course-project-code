// Include files
#include "xcorr.h"
#include "FFTImplementationCallback.h"
#include "alignsigs.h"
#include "fft.h"
#include <cstring>

// Function Definitions
void xcorr(const double x[48000], const double varargin_1[48000], double c[95999])
{
  static creal_T b_x[131072];
  static creal_T y[131072];
  int iy;
  static double costab[65537];
  static double sintab[65537];
  double twid_re;
  int ix;
  int ju;
  int i;
  double twid_im;
  double temp_re;
  int iheight;
  double temp_im;
  double re;
  static double c1[131072];
  int j;
  fft(x, b_x);
  fft(varargin_1, y);
  for (iy = 0; iy < 131072; iy++) {
    twid_re = b_x[iy].re * -y[iy].im + b_x[iy].im * y[iy].re;
    b_x[iy].re = b_x[iy].re * y[iy].re - b_x[iy].im * -y[iy].im;
    b_x[iy].im = twid_re;
  }

  FFTImplementationCallback::generate_twiddle_tables((costab), (sintab));
  ix = 0;
  iy = 0;
  ju = 0;
  for (i = 0; i < 131071; i++) {
    boolean_T tst;
    y[iy] = b_x[ix];
    iy = 131072;
    tst = true;
    while (tst) {
      iy >>= 1;
      ju ^= iy;
      tst = ((ju & iy) == 0);
    }

    iy = ju;
    ix++;
  }

  y[iy] = b_x[ix];
  for (i = 0; i <= 131070; i += 2) {
    double d;
    twid_im = y[i + 1].re;
    temp_re = twid_im;
    d = y[i + 1].im;
    temp_im = d;
    re = y[i].re;
    twid_re = y[i].im;
    twid_im = y[i].re - twid_im;
    y[i + 1].re = twid_im;
    d = y[i].im - d;
    y[i + 1].im = d;
    y[i].re = re + temp_re;
    y[i].im = twid_re + temp_im;
  }

  iy = 2;
  ix = 4;
  ju = 32768;
  iheight = 131069;
  while (ju > 0) {
    int istart;
    int temp_re_tmp;
    for (i = 0; i < iheight; i += ix) {
      temp_re_tmp = i + iy;
      temp_re = y[temp_re_tmp].re;
      temp_im = y[temp_re_tmp].im;
      y[temp_re_tmp].re = y[i].re - y[temp_re_tmp].re;
      y[temp_re_tmp].im = y[i].im - y[temp_re_tmp].im;
      y[i].re += temp_re;
      y[i].im += temp_im;
    }

    istart = 1;
    for (j = ju; j < 65536; j += ju) {
      int ihi;
      twid_re = costab[j];
      twid_im = sintab[j];
      i = istart;
      ihi = istart + iheight;
      while (i < ihi) {
        temp_re_tmp = i + iy;
        temp_re = twid_re * y[temp_re_tmp].re - twid_im * y[temp_re_tmp].im;
        temp_im = twid_re * y[temp_re_tmp].im + twid_im * y[temp_re_tmp].re;
        y[temp_re_tmp].re = y[i].re - temp_re;
        y[temp_re_tmp].im = y[i].im - temp_im;
        y[i].re += temp_re;
        y[i].im += temp_im;
        i += ix;
      }

      istart++;
    }

    ju /= 2;
    iy = ix;
    ix += ix;
    iheight -= iy;
  }

  for (i = 0; i < 131072; i++) {
    re = 7.62939453125E-6 * y[i].re;
    y[i].re = re;
    y[i].im *= 7.62939453125E-6;
    c1[i] = re;
  }

  std::memcpy(&c[0], &c1[83073], 47999U * sizeof(double));
  std::memcpy(&c[47999], &c1[0], 48000U * sizeof(double));
}
