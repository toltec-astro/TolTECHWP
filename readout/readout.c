#include "../vend/ini/ini.h"
#include "../vend/sdk_826_linux_3.3.11/driver/826api.h"
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdlib.h> 

void LoadConfig(void)
{
    ini_t *config = ini_load("../config/masterconfig.ini"); 
    const char *logging_port = ini_get(config, "ports", "logging_port");
    if (logging_port) {
      printf("name: %s\n", logging_port);
    }   
}

void ConfigureQuadCounter(void)
{
    S826_CounterModeWrite(
        board, 
        countquad,                // Configure counter:
        S826_CM_K_QUADX4 |        // Quadrature x1/x2/x4 multiplier
        //S826_CM_K_ARISE |       // clock = ClkA (external digital signal)
        //S826_XS_100HKZ |        // route tick generator to index input
        (S826_CM_XS_CH0+countime) // route CH1 to Index input
    );   
    S826_CounterSnapshotConfigWrite(
        board, countquad,  // Acquire counts upon tick rising edge.
        S826_SSRMASK_IXRISE, S826_BITWRITE
    );
  
    flags = S826_CounterStateWrite(board, countquad, 1); // start the counter
    if (flags < 0)
        printf("Quad Counter returned error code %d", flags);
}

void ConfigureTimerCounter(void)
{
    S826_CounterModeWrite(board, countime,       // Configure counter mode:
            S826_CM_K_1MHZ |                     // clock source = 1 MHz internal
            S826_CM_PX_START | S826_CM_PX_ZERO | // preload @start and counts==0
            S826_CM_UD_REVERSE |                 // count down
            S826_CM_OM_NOTZERO
    ); // ExtOut = (counts!=0)
    S826_CounterPreloadWrite(board, countime, 0, datausec); // Set period in microseconds.
    flags = S826_CounterStateWrite(board, countime, 1);      // Start the timer running.
    if (flags < 0)
        printf("Timer Counter returned error code %d", flags);
}

void ConfigurePulsePerSecondCounter(void)
{
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
}

//ini_free(config);

int main(int argc, char **argv){

    LoadConfig();
    
    if( argc < 3 ) {
        printf("Use: readout datadtime sleepdtime duration\n");
        printf("     datadtime = intervals how often data is read (us)\n");
        printf("     sleepdtime = how long computer sleeps between reads (us)\n");
        printf("     duration = how long experiment is (seconds)\n");
        exit(0);
    }    
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


}