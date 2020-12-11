// Include files
#include "FFTImplementationCallback.h"
#include "alignsigs.h"
#include <cmath>

// Function Definitions
void FFTImplementationCallback::getback_radix2_fft(creal_T y[131072], int yoff,
  const creal_T reconVar1[65536], const creal_T reconVar2[65536], const int
  wrapIndex[65536], int hnRows)
{
  int iterVar;
  double temp1_re;
  double temp1_im;
  double temp2_re;
  double temp2_im;
  double y_im;
  int i;
  int temp1_re_tmp;
  iterVar = hnRows / 2;
  temp1_re = y[yoff].re;
  temp1_im = y[yoff].im;
  temp2_re = y[yoff].re * reconVar1[0].im + y[yoff].im * reconVar1[0].re;
  temp2_im = y[yoff].re;
  y_im = -y[yoff].im;
  y[yoff].re = 0.5 * ((y[yoff].re * reconVar1[0].re - y[yoff].im * reconVar1[0].
                       im) + (temp2_im * reconVar2[0].re - y_im * reconVar2[0].
    im));
  y[yoff].im = 0.5 * (temp2_re + (temp2_im * reconVar2[0].im + y_im * reconVar2
    [0].re));
  i = yoff + hnRows;
  y[i].re = 0.5 * ((temp1_re * reconVar2[0].re - temp1_im * reconVar2[0].im) +
                   (temp1_re * reconVar1[0].re - -temp1_im * reconVar1[0].im));
  y[i].im = 0.5 * ((temp1_re * reconVar2[0].im + temp1_im * reconVar2[0].re) +
                   (temp1_re * reconVar1[0].im + -temp1_im * reconVar1[0].re));
  for (int b_i = 2; b_i <= iterVar; b_i++) {
    int temp2_re_tmp_tmp;
    int b_temp2_re_tmp_tmp;
    int temp2_re_tmp;
    int i1;
    temp1_re_tmp = (yoff + b_i) - 1;
    temp1_re = y[temp1_re_tmp].re;
    temp1_im = y[temp1_re_tmp].im;
    temp2_re_tmp_tmp = wrapIndex[b_i - 1];
    b_temp2_re_tmp_tmp = yoff + temp2_re_tmp_tmp;
    temp2_re_tmp = b_temp2_re_tmp_tmp - 1;
    temp2_re = y[temp2_re_tmp].re;
    temp2_im = y[temp2_re_tmp].im;
    y[temp1_re_tmp].re = 0.5 * ((temp1_re * reconVar1[b_i - 1].re - temp1_im *
      reconVar1[b_i - 1].im) + (temp2_re * reconVar2[b_i - 1].re - -temp2_im *
      reconVar2[b_i - 1].im));
    y[temp1_re_tmp].im = 0.5 * ((temp1_re * reconVar1[b_i - 1].im + temp1_im *
      reconVar1[b_i - 1].re) + (temp2_re * reconVar2[b_i - 1].im + -temp2_im *
      reconVar2[b_i - 1].re));
    i1 = (i + b_i) - 1;
    y[i1].re = 0.5 * ((temp1_re * reconVar2[b_i - 1].re - temp1_im *
                       reconVar2[b_i - 1].im) + (temp2_re * reconVar1[b_i - 1].
      re - -temp2_im * reconVar1[b_i - 1].im));
    y[i1].im = 0.5 * ((temp1_re * reconVar2[b_i - 1].im + temp1_im *
                       reconVar2[b_i - 1].re) + (temp2_re * reconVar1[b_i - 1].
      im + -temp2_im * reconVar1[b_i - 1].re));
    temp1_re_tmp = temp2_re_tmp_tmp - 1;
    y[temp2_re_tmp].re = 0.5 * ((temp2_re * reconVar1[temp1_re_tmp].re -
      temp2_im * reconVar1[temp1_re_tmp].im) + (temp1_re *
      reconVar2[temp1_re_tmp].re - -temp1_im * reconVar2[temp1_re_tmp].im));
    y[temp2_re_tmp].im = 0.5 * ((temp2_re * reconVar1[temp1_re_tmp].im +
      temp2_im * reconVar1[temp1_re_tmp].re) + (temp1_re *
      reconVar2[temp1_re_tmp].im + -temp1_im * reconVar2[temp1_re_tmp].re));
    i1 = (b_temp2_re_tmp_tmp + hnRows) - 1;
    y[i1].re = 0.5 * ((temp2_re * reconVar2[temp1_re_tmp].re - temp2_im *
                       reconVar2[temp1_re_tmp].im) + (temp1_re *
      reconVar1[temp1_re_tmp].re - -temp1_im * reconVar1[temp1_re_tmp].im));
    y[i1].im = 0.5 * ((temp2_re * reconVar2[temp1_re_tmp].im + temp2_im *
                       reconVar2[temp1_re_tmp].re) + (temp1_re *
      reconVar1[temp1_re_tmp].im + -temp1_im * reconVar1[temp1_re_tmp].re));
  }

  if (iterVar != 0) {
    double d;
    temp1_re_tmp = yoff + iterVar;
    temp1_re = y[temp1_re_tmp].re;
    temp1_im = y[temp1_re_tmp].im;
    temp2_im = temp1_re * reconVar2[iterVar].re;
    y_im = temp1_re * reconVar1[iterVar].re;
    y[temp1_re_tmp].re = 0.5 * ((y_im - temp1_im * reconVar1[iterVar].im) +
      (temp2_im - -temp1_im * reconVar2[iterVar].im));
    d = temp1_re * reconVar2[iterVar].im;
    temp2_re = temp1_re * reconVar1[iterVar].im;
    y[temp1_re_tmp].im = 0.5 * ((temp2_re + temp1_im * reconVar1[iterVar].re) +
      (d + -temp1_im * reconVar2[iterVar].re));
    i += iterVar;
    y[i].re = 0.5 * ((temp2_im - temp1_im * reconVar2[iterVar].im) + (y_im -
      -temp1_im * reconVar1[iterVar].im));
    y[i].im = 0.5 * ((d + temp1_im * reconVar2[iterVar].re) + (temp2_re +
      -temp1_im * reconVar1[iterVar].re));
  }
}

void FFTImplementationCallback::doHalfLengthRadix2(const double x[48000], int
  xoffInit, creal_T y[131072], const double costab[65537], const double sintab
  [65537])
{
  int i;
  int iy;
  static double hcostab[32768];
  int ju;
  static creal_T reconVar1[65536];
  static double hsintab[32768];
  int k;
  static creal_T reconVar2[65536];
  static int bitrevIndex[65536];
  static int wrapIndex[65536];
  double temp_re;
  double temp_im;
  int iheight;
  int j;
  for (i = 0; i < 32768; i++) {
    iy = ((i + 1) << 1) - 2;
    hcostab[i] = costab[iy];
    hsintab[i] = sintab[iy];
  }

  for (i = 0; i < 65536; i++) {
    reconVar1[i].re = sintab[i] + 1.0;
    reconVar1[i].im = -costab[i];
    reconVar2[i].re = 1.0 - sintab[i];
    reconVar2[i].im = costab[i];
    if (i + 1 != 1) {
      wrapIndex[i] = 65537 - i;
    } else {
      wrapIndex[0] = 1;
    }
  }

  ju = 0;
  iy = 1;
  for (k = 0; k < 65535; k++) {
    boolean_T tst;
    bitrevIndex[k] = iy;
    iy = 65536;
    tst = true;
    while (tst) {
      iy >>= 1;
      ju ^= iy;
      tst = ((ju & iy) == 0);
    }

    iy = ju + 1;
  }

  bitrevIndex[65535] = iy;
  iy = xoffInit;
  for (i = 0; i < 24000; i++) {
    y[bitrevIndex[i] - 1].re = x[iy];
    y[bitrevIndex[i] - 1].im = x[iy + 1];
    iy += 2;
  }

  for (i = 0; i <= 65534; i += 2) {
    temp_re = y[i + 1].re;
    temp_im = y[i + 1].im;
    y[i + 1].re = y[i].re - y[i + 1].re;
    y[i + 1].im = y[i].im - y[i + 1].im;
    y[i].re += temp_re;
    y[i].im += temp_im;
  }

  iy = 2;
  ju = 4;
  k = 16384;
  iheight = 65533;
  while (k > 0) {
    int istart;
    int temp_re_tmp;
    for (i = 0; i < iheight; i += ju) {
      temp_re_tmp = i + iy;
      temp_re = y[temp_re_tmp].re;
      temp_im = y[temp_re_tmp].im;
      y[temp_re_tmp].re = y[i].re - temp_re;
      y[temp_re_tmp].im = y[i].im - temp_im;
      y[i].re += temp_re;
      y[i].im += temp_im;
    }

    istart = 1;
    for (j = k; j < 32768; j += k) {
      double twid_re;
      double twid_im;
      int ihi;
      twid_re = hcostab[j];
      twid_im = hsintab[j];
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
        i += ju;
      }

      istart++;
    }

    k /= 2;
    iy = ju;
    ju += ju;
    iheight -= iy;
  }

  FFTImplementationCallback::getback_radix2_fft((y), (0), (reconVar1),
    (reconVar2), (wrapIndex), (65536));
}

void FFTImplementationCallback::generate_twiddle_tables(double costab[65537],
  double sintab[65537])
{
  static double costab1q[32769];
  int k;
  costab1q[0] = 1.0;
  for (k = 0; k < 16384; k++) {
    costab1q[k + 1] = std::cos(4.7936899621426287E-5 * (static_cast<double>(k) +
      1.0));
  }

  for (k = 0; k < 16383; k++) {
    costab1q[k + 16385] = std::sin(4.7936899621426287E-5 * (32768.0 - (
      static_cast<double>(k) + 16385.0)));
  }

  costab1q[32768] = 0.0;
  costab[0] = 1.0;
  sintab[0] = 0.0;
  for (k = 0; k < 32768; k++) {
    double costab_tmp;
    double sintab_tmp;
    costab_tmp = costab1q[k + 1];
    costab[k + 1] = costab_tmp;
    sintab_tmp = costab1q[32767 - k];
    sintab[k + 1] = sintab_tmp;
    costab[k + 32769] = -sintab_tmp;
    sintab[k + 32769] = costab_tmp;
  }
}