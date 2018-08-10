// Frequency Counter for 826

// Usage: connect to A (or A and B if QUAD detection) of selected channel

#include "826api.h"
#include <stdio.h>
#include <unistd.h>

// Configure a frequency counter and start it running.
int CreateFrequencyCounter(uint board, uint chan, uint ticksel)
{
  if ((ticksel < S826_CM_XS_R1HZ) || (ticksel > S826_CM_XS_1MHZ))
    return S826_ERR_VALUE;
  // Abort if invalid tick selector.
  S826_CounterModeWrite(board, chan,      // Configure counter:
			//S826_CM_K_QUADX4 |  // Quadrature x1/x2/x4 multiplier
			S826_CM_K_ARISE |   // clock = ClkA (external digital signal)
			ticksel |           // route tick generator to Index input
			S826_CM_PX_IXRISE); // preload upon tick rising edge
  S826_CounterPreloadWrite(board, chan, 0, 0);  // Reset counts upon tick rising edge.
  S826_CounterSnapshotConfigWrite(board, chan,  // Acquire counts upon tick rising edge.
				  S826_SSRMASK_IXRISE, S826_BITWRITE);
  return S826_CounterStateWrite(board, chan, 1); // Start the frequency counter running.
}

// Receive the next frequency sample.
int ReadFrequency(uint board, uint chan, uint *sample, uint *tstamp, uint *reason)
{
  return S826_CounterSnapshotRead(board, chan, // Block until next sample
				  sample, tstamp, reason,  // and then receive accumulated counts.
				  S826_WAIT_INFINITE);
}

int main(){
  int board = 0;
  int chan = 1;

  // Open the 826 API ---------------
  int flags = S826_SystemOpen();
  if (flags < 0)
    printf("S826_SystemOpen returned error code %d", flags);

  // Measure frequency 10 times per second.
  uint counts=10, tstamp, reason;
  CreateFrequencyCounter(board, chan, S826_CM_XS_1HZ);
  ReadFrequency(board, chan, NULL, NULL, NULL);
  while (counts && !ReadFrequency(board, chan, &counts, &tstamp, &reason)) {
    printf("Frequency = %.1f Hz   Time=%d   Reason=%x\n", (float)counts/1.0, tstamp, reason);
  }
  S826_CounterStateWrite(board, chan, 0); // Stop the counter

  // Close system
  S826_SystemClose();
}

// gcc -D_LINUX -Wall -Wextra -DOSTYPE_LINUX -c  -I /home/polboss/toltec/sdk_826_linux_3.3.10/demo freqcounter.c

// gcc -D_LINUX freqcounter.o -o freqcounter -lm -L /home/polboss/toltec/sdk_826_linux_3.3.10/demo -l826_64
