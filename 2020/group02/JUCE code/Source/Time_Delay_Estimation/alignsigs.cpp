
// This is the function that will be called from the main application code
// The main application is responsible for linearizing the circular TDE buffer and provide both channels as arguments to this function
// An offset is returned, which can be used by the main application to adjust/delay the leading audio stream by the returned number of samples

#include "alignsigs.h"
#include "circshift.h"
#include "finddelay.h"

// Function Definitions
double alignsigs(double X1[48000], double X2[48000])
{
  int di;
  di = FindDelay(X1, X2); //estimate delay between the two signals
  return di;
}
