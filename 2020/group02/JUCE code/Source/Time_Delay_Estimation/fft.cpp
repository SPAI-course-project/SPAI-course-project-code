// Include files
#include "fft.h"
#include "FFTImplementationCallback.h"
#include "alignsigs.h"
#include <cmath>
#include <cstring>

// Function Definitions
void fft(const double x[48000], creal_T y[131072])
{
  static double costab1q[32769];
  int k;
  static double costab[65537];
  static double sintab[65537];
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
    sintab_tmp = -costab1q[32767 - k];
    sintab[k + 1] = sintab_tmp;
    costab[k + 32769] = sintab_tmp;
    sintab[k + 32769] = -costab_tmp;
  }

  std::memset(&y[0], 0, 131072U * sizeof(creal_T));
  FFTImplementationCallback::doHalfLengthRadix2((x), (0), (y), (costab), (sintab));
}