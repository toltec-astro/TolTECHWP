#define S826_CM_XS_CH0     2

#include "../vend/ini/ini.h"
#include "../vend/sdk_826_linux_3.3.11/demo/826api.h"
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdlib.h> 
#include <signal.h>

void ConfigureSensorPower(int board, ini_t *config)
{
    // get voltage information
    char *range = ini_get(config, "sensors.power", "output_voltage_range");
    char *voltage_setpoint = ini_get(config, "sensors.power", "output_voltage_setpoint");
    char *channel = ini_get(config, "sensors.power", "output_channel");

    printf("range: %d\n", atoi(range));
    printf("voltage_setpoint: %d\n", (int)strtol(voltage_setpoint, NULL, 0));
    printf("channel: %d\n", atoi(channel));


    //// Only Read Sensor Power Params for now.

    int runmode = 0;
    // int pwr = S826_DacRangeWrite(board, atoi(channel), atoi(range), runmode);
    // if (pwr < 0)
    //     printf("Configure power error code %d", pwr);

    // int pwr_set = S826_DacDataWrite(board, atoi(channel), 0xFFFF, runmode);
    // if (pwr_set < 0)
    //     printf("Configure power data error code %d", pwr);
}


void ConfigureSensors(int board, ini_t *config)
{
    // get number of sensors
    int *sens_count = ini_get(config, "sensors", "sensor_num");
    printf("sensor count: %d\n", atoi(sens_count));

    // loop over and set all the sensors
    for(int sens = 0; sens <= atoi(sens_count) - 1; ++sens)
    {
        printf("sensor number: %d\n", sens);
        char buf[2];
        sprintf(buf, "%02d", sens);
        printf("sensor number string: %s\n", buf);

        // construct the string
        // char sensor_id[256];
        // snprintf(sensor_id, sizeof sensor_id, "%s%s", str1, str2);
        // printf("%s", sensor_id);

        // print out ancillary information
        // const int *debug = ini_get(config, sensor_id, "sensor_name");
        // printf("name: %s\n", debug);

        // get the configuration settings
        // const int *timeslot = ini_get(config, sensor_id, "sensor_timeslot");
        // const int *channel = ini_get(config, sensor_id, "sensor_chan");
        // const int *settling = ini_get(config, sensor_id, "sensor_num");
        // const int *range = ini_get(config, sensor_id, "sensor_num");
        
        // printf("timeslot: %d\n", &timeslot);
        // printf("channel: %d\n", &channel);
        // printf("settling: %d\n", &settling);
        // printf("range: %d\n", &range);

        // int sensor_config_write = S826_AdcSlotConfigWrite(
        //     board, 
        //     temp_1_timeslot, 
        //     temp_1_channel, 
        //     temp_1_settling,
        //     temp_1_range
        // );
        // if (sensor_config_write < 0)
        //     printf("Configure error %d\n", temp_1);
    }
}

void SystemCloseHandler(int sig)
{
    signal(sig, SIG_IGN);
    printf("\nSignal Caught!\n");
    // add power shut off here
    S826_SystemClose();
    printf("System Closed!\n");
    exit(0);
}


void SystemOpenHandler(void)
{
    int id;
    int flags = S826_SystemOpen();
    if (flags < 0)
        printf("S826_SystemOpen returned error code %d\n", flags);
    else if (flags == 0)
        printf("No boards were detected\n");
    else {
        printf("Boards were detected with these IDs:");
        for (id = 0; id < 16; id++) {
            if (flags & (1 << id))
                printf(" %d \n", id);
        }
    }
}


/*
void ConfigureQuadCounter(int board, int countquad, int counttime)
{
    S826_CounterModeWrite(
        board, 
        countquad,                  // Configure counter:
        S826_CM_K_QUADX4 |          // Quadrature x1/x2/x4 multiplier
        //S826_CM_K_ARISE |         // clock = ClkA (external digital signal)
        //S826_XS_100HKZ |          // route tick generator to index input
        (S826_CM_XS_CH0 + counttime) // route CH1 to Index input
    );   
    S826_CounterSnapshotConfigWrite(
        board, countquad,  // Acquire counts upon tick rising edge.
        S826_SSRMASK_IXRISE, 
        S826_BITWRITE
    );
  
    int flags = S826_CounterStateWrite(board, countquad, 1); // start the counter
    if (flags < 0)
        printf("Quad Counter returned error code %d\n", flags);
}


void ConfigureTimerCounter(int board, int countime, int datausec)
{
    S826_CounterModeWrite(board, countime,       // Configure counter mode:
            S826_CM_K_1MHZ |                     // clock source = 1 MHz internal
            S826_CM_PX_START | S826_CM_PX_ZERO | // preload @start and counts==0
            S826_CM_UD_REVERSE |                 // count down
            S826_CM_OM_NOTZERO
    );
    S826_CounterPreloadWrite(board, countime, 0, datausec); // Set period in microseconds.
    int flags = S826_CounterStateWrite(board, countime, 1);      // Start the timer running.
    if (flags < 0)
        printf("Timer Counter returned error code %d\n", flags);
}


void ConfigurePulsePerSecondCounter(int board, int countpps)
{
    S826_CounterModeWrite(board, countpps,      // Configure counter:
                            S826_CM_K_AFALL );   // clock = ClkA (external digital signal)
    S826_CounterFilterWrite(board, countpps,  // Filer Configuration:
              (1 << 31) | (1 << 30) | // Apply to IX and clock
              100 );                  // 100x20ns = 2us filter constant
    S826_CounterSnapshotConfigWrite(board, countpps,  // Acquire counts upon tick rising edge.
                  S826_SSRMASK_IXRISE, S826_BITWRITE);
    int flags = S826_CounterStateWrite(board, countpps, 1); // start the counter
    if (flags < 0)
        printf("PPS Counter returned error code %d\n", flags);   
}
*/

/*
 * Main Program Loop!
 *
 */
int main(int argc, char **argv){
    
    // signal handler
    signal(SIGINT, SystemCloseHandler);
    
    // argument handler
    if( argc < 1 ) {
        printf("Use: readout \n");
        printf("     configfilename \n");
        exit(0);
    }

    // get reference to configuration file
    char filename[] = "../config/masterconfig.ini"; //argv[1];
    printf("%s\n", filename);
    printf("%s\n", argv[1]);
    ini_t *config = ini_load(filename);


    // Variables
    int board = 0;
    int countquad = 0;  // quadrature counter
    int countpps = 1;   // pps counter
    int counttime = 2;  // timer counter

    ConfigureSensorPower(board, config);
    ConfigureSensors(board, config);
    

    int i;
    struct timespec treq;
    time_t rawtime, starttime;
    char s[2000000], t[200];
    char fname[100];
    FILE *outf;

    // Variables to read all snapshots while it's moving
    int lastcount = 0;   // last value which was read
    int dcount = 1;      // diff current - previous value
    int loopcount = 0;   // number of loops with delays
    int errcount = 0;    // number of overflow errors
    int sampcount = 0;   // number of samples in this readout session
    uint counts[1000], tstamp[1000], reason[1000];
    
    ////// Preparation
    // set timer counter interval: determines how often count data is stored.
    int datausec = 10; // Micro seconds
    int sleepusec = 200; // Set computer sleep interval
    int duration = 10; // Set duration seconds

    treq.tv_sec = 0;
    treq.tv_nsec = sleepusec * 1000;

    // Open the 826 API ---------------
    // int flags = S826_SystemOpen();
    // printf("S826_SystemOpen returned error code %d\n", flags);
    SystemOpenHandler();

    ////// Configure all the Things!
    // ConfigureTimerCounter(board, counttime, datausec);
    // ConfigurePulsePerSecondCounter(board, countpps);
    // ConfigureQuadCounter(board, countquad, countime);
    // ConfigureSensors();
    

    time(&rawtime);
    starttime = rawtime;
    while(rawtime - starttime < duration){
        
        // Update Time/Counter & Loop
        time(&rawtime);
        loopcount++;
        nanosleep(&treq, NULL);
    }

    // Close the 826 API
    S826_SystemClose();
}
