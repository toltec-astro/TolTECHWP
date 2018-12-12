/*
   Logs Quadrature Counter for Sensoray 826

   This program is a draft for the TolTEC HWP program. It reads
   the Quadrature signal into one counter (countquad) while using
   a different counter for timing the snapshots in the quad
   counter.

   The program uses a third counter (countpps) to log pulses.

   2DO:
   - Set up countpps (snapshot on rising signal)
   - Read out countpps, display on screen
   - Put countpps into logfile
 */

// Logs counter for 826

// Define Index Source from Counter Channel 0
# define S826_CM_XS_CH0     2
# define printout 0

// Includes
#include "826api.h"
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdlib.h> // for exit

int main(int argc, char **argv){
  // Variables
  int i;
  int board = 0;
  int countquad = 0; // quadrature counter
  int countime = 2; // timer counter
  int countpps = 1; // pps counter
  struct timespec treq;
  time_t rawtime,startime;
  char s[2000000],t[200];
  char fname[100];
  FILE *outf;
  // Variables to Read all snapshots while it's moving
  int lastcount = 0; // last value which was read
  int dcount = 1; // diff current - previous value
  int loopcount = 0; // number of loops with delays
  int errcount = 0; // number of overflow errors
  int sampcount = 0; // number of samples in this readout session
  uint counts[1000], tstamp[1000], reason[1000];


  //// Get inputs Arguments
  if( argc < 3 ) {
    printf("Use: countlog datadtime sleepdtime duration\n");
    printf("     datadtime = intervals how often data is read (us)\n");
    printf("     sleepdtime = how long computer sleeps between reads (us)\n");
    printf("     duration = how long experiment is (seconds)\n");
    exit(0);
  }
  
  //// Preparation
  // Set timer counter interval: Determines how often count data is stored.
  int datausec = atoi(argv[1]); // Micro seconds
  // Set computer sleep interval
  int sleepusec = atoi(argv[2]);
  treq.tv_sec = 0;
  treq.tv_nsec = sleepusec * 1000;
  // Set duration
  int duration = atoi(argv[3]); // Seconds
  // Open the 826 API ---------------
  int flags = S826_SystemOpen();
  if (flags < 0)
    printf("S826_SystemOpen returned error code %d", flags);

  // Timer Counter Configuration
  S826_CounterModeWrite(board, countime,       // Configure counter mode:
			S826_CM_K_1MHZ |     // clock source = 1 MHz internal
			S826_CM_PX_START | S826_CM_PX_ZERO | // preload @start and counts==0
			S826_CM_UD_REVERSE | // count down
			S826_CM_OM_NOTZERO); // ExtOut = (counts!=0)
  S826_CounterPreloadWrite(board, countime, 0, datausec); // Set period in microseconds.
  flags = S826_CounterStateWrite(board, countime, 1); // Start the timer running.
  if (flags < 0)
    printf("Timer Counter returned error code %d", flags);
  // Quadrature Counter Configuration
  
  S826_CounterModeWrite(board, countquad,      // Configure counter:
			S826_CM_K_QUADX4 |  // Quadrature x1/x2/x4 multiplier
			//S826_CM_K_ARISE |   // clock = ClkA (external digital signal)
			//S826_XS_100HKZ |    // route tick generator to index input
			(S826_CM_XS_CH0+countime) );   // route CH1 to Index input
  
  S826_CounterSnapshotConfigWrite(board, countquad,  // Acquire counts upon tick rising edge.
				  S826_SSRMASK_IXRISE, S826_BITWRITE);
  
  flags = S826_CounterStateWrite(board, countquad, 1); // start the counter
  if (flags < 0)
    printf("Quad Counter returned error code %d", flags);
  
  // Pulse Per Second Counter Configuration
  // - Need filter when using arduino on breadboard
  // - Need count on FALL and snap on RISE to avoid race condition
  S826_CounterModeWrite(board, countpps,      // Configure counter:
			S826_CM_K_AFALL );   // clock = ClkA (external digital signal)
  S826_CounterFilterWrite(board, countpps,  // Filer Configuration:
			  (1 << 31) | (1 << 30) | // Apply to IX and clock
			  100 );                  // 100x20ns = 2us filter constant
  S826_CounterSnapshotConfigWrite(board, countpps,  // Acquire counts upon tick rising edge.
				  S826_SSRMASK_IXRISE, S826_BITWRITE);
  flags = S826_CounterStateWrite(board, countpps, 1); // start the counter
  if (flags < 0)
    printf("PPS Counter returned error code %d", flags);

  // Get starting timesamp
  time(&rawtime);
  startime = rawtime;
  // Setup filename
  strftime(fname,100,"Data_%y-%m-%d_%H-%M.txt",localtime(&startime));
  outf = fopen(fname,"wt");
  printf("Opening File = %s\n", fname);

  //// Read snapshots loop
  // Get start time
  uint tstart;
  S826_TimestampRead(board, &tstart);
  //strcpy(s,"Start:\n");
  //while( loopcount < 100000 || (dcount && loopcount < 100000 ) ){
  while( rawtime-startime < duration ){
    sampcount = 0;
    //** Read from quadrature counter
    // Read first snapshot (also clears ERR_FIFOOVERFLOW)
    flags = S826_CounterSnapshotRead(board, countquad,
				     counts+sampcount, tstamp+sampcount, reason+sampcount, 0);
    // Read all snapshots
    while(flags == S826_ERR_OK || flags == S826_ERR_FIFOOVERFLOW){
      // Keep track of counts
      dcount = counts[sampcount] - lastcount;
      lastcount = counts[sampcount];
      // Print result
      sprintf(t,"Count = %d   Time = %.3fms   Reason = %x   Scnt = %d", counts[sampcount],
	      (float)(tstamp[sampcount]-tstart)/1000.0, reason[sampcount], sampcount);
      fprintf(outf,"%s", t);
      if(printout) printf("%s\n" , t);
      // Check FIFO overflow
      if(flags == S826_ERR_FIFOOVERFLOW){
	    errcount++;
	    //sprintf(t,"  ####  FiFo Overflow samp=%d\n", sampcount);
	    //strcat(s,t);
	    fprintf(outf,"  ####  FiFo Overflow samp=%d\n", sampcount);
      } else {
	    fprintf(outf,"\n");
      }
      // Increase counter
      sampcount++;
      // Read next snapshot
      flags = S826_CounterSnapshotRead(board, countquad,
				       counts+sampcount, tstamp+sampcount, reason+sampcount, 0);
    }
    //printf("QUAD: flags=%d ok=%d fifooverflow=%d\n", flags, S826_ERR_OK, S826_ERR_FIFOOVERFLOW);
    // Print out samples and statistics to  s
    //for(i=0;i<1;i++){
    //sprintf(t,"Count = %d   Time = %.3fms   Reason = %x\n", counts[i],
    //	      (float)(tstamp[i]-tstart)/1000.0, reason[i]);
      //strcat(s,t);
    //}
    //strcat(s," . . .\n . . .\n");
    //sprintf(t,"Number of samples = %d\n", sampcount);
    //strcat(s,t);
    //sprintf(t,"Got all Snapshots - waiting - flags = %d\n", flags);
    //** Read from PPS counter
    // Read one snapshot
    flags = S826_CounterSnapshotRead(board, countpps,
				     counts+sampcount, tstamp+sampcount, reason+sampcount, 0);
    // If snapshot found, log it
    //printf("PPS:  flags=%d ok=%d fifooverflow=%d\n", flags, S826_ERR_OK, S826_ERR_FIFOOVERFLOW);
    if( flags == S826_ERR_OK || flags == S826_ERR_FIFOOVERFLOW ){
    	sprintf(t,"PPS:  Count = %d   Time = %.3fms   Reason = %x   Scnt = %d\n", counts[sampcount],
			      (float)(tstamp[sampcount]-tstart)/1000.0, reason[sampcount], sampcount);
    	printf(t);
	fprintf(outf, t);
    }
    // Update time and counter
    time(&rawtime);
    loopcount ++;
    // Wait
    nanosleep(&treq,NULL);
  } // while ( rawtime-startime < duration )
  //printf("%s", s);
  printf("Number of Loops = %d\n" , loopcount);
  printf("Number of Errors = %d\n", errcount);
  printf("Data dTime  = %dus\n", datausec);
  printf("Sleep dTime = %dus\n", (int)(treq.tv_nsec/1000));
  printf("Duration    = %ds\n", duration);
  // Close system
  S826_SystemClose();
  // Close file
  fclose(outf);
}

// gcc -D_LINUX -Wall -Wextra -DOSTYPE_LINUX -c  -no-pie -I /home/poluser/toltec/sdk_826_linux_3.3.11/demo countlog.c

// gcc -D_LINUX countlog.o -no-pie -o countlog -lm -L /home/poluser/toltec/sdk_826_linux_3.3.11/demo -l826_64
