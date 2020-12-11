// Include files
#include "circshift.h"
#include "alignsigs.h"
#include <cstring>

// Function Definitions
void circshift(double a[48000], double p)
{
  int ns;
  boolean_T shiftright;
  double unusedU0[24000];
  if (p < 0.0) {
    ns = static_cast<int>(-p);
    shiftright = false;
  } else {
    ns = static_cast<int>(p);
    shiftright = true;
  }

  if (ns > 48000) {
    ns -= ns / 48000 * 48000;
  }

  if (ns > 24000) {
    ns = 48000 - ns;
    shiftright = !shiftright;
  }

  std::memset(&unusedU0[0], 0, 24000U * sizeof(double));
  if (ns > 0) {
    if (shiftright) {
      int k;
      int i;
      for (k = 0; k < ns; k++) {
        unusedU0[k] = a[(k - ns) + 48000];
      }

      i = ns + 1;
      for (k = 48000; k >= i; k--) {
        a[k - 1] = a[(k - ns) - 1];
      }

      if (0 <= ns - 1) {
        std::memcpy(&a[0], &unusedU0[0], ns * sizeof(double));
      }
    } else {
      int k;
      int i;
      if (0 <= ns - 1) {
        std::memcpy(&unusedU0[0], &a[0], ns * sizeof(double));
      }

      i = 47999 - ns;
      for (k = 0; k <= i; k++) {
        a[k] = a[k + ns];
      }

      for (k = 0; k < ns; k++) {
        a[(k - ns) + 48000] = unusedU0[k];
      }
    }
  }
}