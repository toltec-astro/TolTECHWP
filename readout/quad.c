/*
 * readout.c
 * - Read from the Sensoray Card.
 * - Timer / PPS / Quad
 */

#include "826api.h"
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdlib.h> 

int main(int argc, char **argv){

	int board = 0;
	char filename[100];
	FILE *outf;

	// get & handle inputs.
	if( argc < 3 ) {
		printf("Use: readout datadtime sleepdtime duration\n");
		printf("     datadtime = intervals how often data is read (us)\n");
		printf("     sleepdtime = how long computer sleeps between reads (us)\n");
		printf("     duration = how long experiment is (seconds)\n");
		exit(0);
	}
	int datausec  = atoi(argv[1]); // Set timer counter interval: Determines how often count data is stored.
	int sleepusec = atoi(argv[2]); // Set computer sleep interval (microseconds)
	int duration  = atoi(argv[3]); // seconds

	treq.tv_sec = 0;
	treq.tv_nsec = sleepusec * 1000;	

	// Initiate the API
	int flags = S826_SystemOpen();
	if (flags < 0)
		printf("API Open returned error code %d", flags);

	//
	// Time Configuration
	//
	S826_CounterModeWrite(board, countime,       // Configure counter mode:
			S826_CM_K_1MHZ |     // clock source = 1 MHz internal
			S826_CM_PX_START | S826_CM_PX_ZERO | // preload @start and counts==0
			S826_CM_UD_REVERSE | // count down
			S826_CM_OM_NOTZERO // ExtOut = (counts!=0)
	); 
	S826_CounterPreloadWrite(board, countime, 0, datausec); // Set period in microseconds.
	flags = S826_CounterStateWrite(board, countime, 1);     // Start the timer running.
	if (flags < 0)
		printf("Time Counter returned error code %d", flags);
	
	//
	// Quadrature Counter Configuration
	//
	S826_CounterModeWrite(board, countquad,              // Configure counter:
			S826_CM_K_QUADX4 |  			             // Quadrature x1/x2/x4 multiplier
			(S826_CM_XS_CH0 + countime)                  // route CH1 to Index input
	);   
	S826_CounterSnapshotConfigWrite(board, countquad,    // Acquire counts upon tick rising edge.
			S826_SSRMASK_IXRISE, S826_BITWRITE);
	flags = S826_CounterStateWrite(board, countquad, 1); // start the counter
	if (flags < 0)
    	printf("Quad Counter returned error code %d", flags);

    //
	//  Pulse Per Second Counter Configuration
	//
	S826_CounterModeWrite(board, countpps,    
			S826_CM_K_AFALL );   
	S826_CounterFilterWrite(board, countpps,  // Filer Configuration:
			  (1 << 31) | (1 << 30) | // Apply to IX and clock
			  100 );                  // 100x20ns = 2us filter constant
	S826_CounterSnapshotConfigWrite(board, countpps, 
				  S826_SSRMASK_IXRISE, S826_BITWRITE);
	flags = S826_CounterStateWrite(board, countpps, 1); 
	if (flags < 0)
		printf("PPS Counter returned error code %d", flags);

	while(rawtime - starttime < duration){
		sampcount = 0;

		/*
		 *  Read from the Quadrature Counter
		 */
		flags = S826_CounterSnapshotRead(board, countquad,
				counts + sampcount, tstamp + sampcount, 
				reason + sampcount, 0);

		while(flags == S826_ERR_OK || flags == S826_ERR_FIFOOVERFLOW){
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
				fprintf(outf,"  ####  FiFo Overflow samp=%d\n", sampcount);
			} else {
				fprintf(outf,"\n");
			}
			
			sampcount++;
			
			// Read next snapshot
			flags = S826_CounterSnapshotRead(board, countquad,
					counts + sampcount, tstamp + sampcount, 
					reason + sampcount, 0);
		}
			
		/*
		 *  Read from the PPS Counter
		 */
		flags = S826_CounterSnapshotRead(board, countpps,
					 counts+sampcount, tstamp+sampcount, reason+sampcount, 0);
		if(flags == S826_ERR_OK || flags == S826_ERR_FIFOOVERFLOW){
			sprintf(t,"PPS:  Count = %d   Time = %.3fms   Reason = %x   Scnt = %d\n", counts[sampcount],
				  (float)(tstamp[sampcount]-tstart)/1000.0, reason[sampcount], sampcount);
			printf(t);
			fprintf(outf, t);
		}

		// update time/counter & loop
		time(&rawtime);
		loopcount++;
		nanosleep(&treq, NULL);
	}
	
	// close API and system
	S826_SystemClose(); 
}