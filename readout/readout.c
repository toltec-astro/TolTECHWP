#define S826_CM_XS_CH0     2

#include "../vend/ini/ini.h"
#include "../vend/sdk_826_linux_3.3.11/demo/826api.h"

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdlib.h> 
#include <signal.h>

void ReadSensorSnapshot(void)
{
    fprintf(stdout, "Sensor. \n");
    // //////////// configurations 
    // uint slotlist = 0x0007; // i don't know bytes well enough for this.
    // int numberofsensors = 3; // grab this from config
    // ////////////

    // int errcode;     // errcode 
    // int slotval[16]; // buffer must be sized for 16 slots
    
    // errcode = S826_AdcRead(0, slotval, NULL, &slotlist, S826_WAIT_INFINITE); 
    // if (errcode != S826_ERR_OK)
    //     printf("ADC Read Failed. %d", errcode);

    // int slot;
    // for (slot = 0; slot < 3; slot++) 
    // {
    //     int adcdata;
    //     unsigned int burstnum;
    //     float voltage;
        
    //     adcdata = (int)((slotval[slot] & 0xFFFF));
    //     burstnum = ((unsigned int)slotval[slot] >> 24);
    //     voltage = (float)((float)adcdata / (float)(0x7FFF)) * 10;
    //     printf("Slot: %d \t Voltage: %f \t ADCData: %d \t BurstNum %d \n", slot, voltage, adcdata, burstnum);   
    // };
    // printf("\n");   
}

void ConfigureSensorPower(int board, ini_t *config)
{
    // get voltage information
    char *range = ini_get(config, "sensors.power", "output_voltage_range");
    char *voltage_setpoint = ini_get(config, "sensors.power", "output_voltage_setpoint");
    char *channel = ini_get(config, "sensors.power", "output_channel");

    printf("range: %d\n", atoi(range));
    printf("voltage_setpoint: %d\n", (int)strtol(voltage_setpoint, NULL, 0));
    printf("channel: %d\n", atoi(channel));

    // normal runmode / not safemode.
    int runmode = 0;

    //// only Read Sensor Power Params for now.
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
    //char buf[2];

    // loop over and set all the sensors
    char sensor_id[30];
    for(int sens = 1; sens <= atoi(sens_count); ++sens)
    {
        printf("sensor number: %d\n", sens);
        
        // sprintf(buf, "%02d", sens);
        // printf("sensors string %s\n", buf);
        //char sensor_id[18];

        //// construct the string
        sprintf(sensor_id, "sensors.details.%02d", sens);

        printf("-------------\n");
        printf("id: %s\n", sensor_id);

        // construct the string
        // char sensor_id[] = "sensors.details.";
        // snprintf(sensor_id, sizeof sensor_id, "%s%s", str1, str2);
        // printf("%s", sensor_id);

        // print out ancillary information
        // const int *debug = ini_get(config, sensor_id, "sensor_name");
        // printf("name: %s\n", debug);

        // get the configuration settings
        int *timeslot = ini_get(config, sensor_id, "sensor_timeslot");
        int *channel = ini_get(config, sensor_id, "sensor_chan");
        
        printf("timeslot: %d\n", atoi(timeslot));
        printf("channel: %d\n", atoi(channel));

        // int sensor_config_write = S826_AdcSlotConfigWrite(
        //     board, 
        //     atoi(timeslot), 
        //     atoi(channel), 
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

    char *channel = ini_get(config, "sensors.power", "output_channel");

    int pwr_set_off = S826_DacDataWrite(board, atoi(channel), 0x0000, 0);
    if (pwr_set_off < 0)
        printf("Configure power data off code %d.", pwr_set_off);

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



void ConfigureQuadCounter(int board, int countquad, int counttime)
{
    printf("\n Configure QuadCounter \n");
    // S826_CounterModeWrite(
    //     board, 
    //     countquad,                   // Configure counter:
    //     S826_CM_K_QUADX4 |           // Quadrature x1/x2/x4 multiplier
    //     //S826_CM_K_ARISE |          // clock = ClkA (external digital signal)
    //     //S826_XS_100HKZ |           // route tick generator to index input
    //     (S826_CM_XS_CH0 + counttime) // route CH1 to Index input
    // );   
    // S826_CounterSnapshotConfigWrite(
    //     board, countquad,  // Acquire counts upon tick rising edge.
    //     S826_SSRMASK_IXRISE, 
    //     S826_BITWRITE
    // );
  
    // int flags = S826_CounterStateWrite(board, countquad, 1); // start the counter
    // if (flags < 0)
    //     printf("Quad Counter returned error code %d\n", flags);
}


void ConfigureTimerCounter(int board, int countime, int datausec)
{
    printf("\n Configure Timer Counter \n");
    // S826_CounterModeWrite(board, countime,       // Configure counter mode:
    //         S826_CM_K_1MHZ |                     // clock source = 1 MHz internal
    //         S826_CM_PX_START | S826_CM_PX_ZERO | // preload @start and counts==0
    //         S826_CM_UD_REVERSE |                 // count down
    //         S826_CM_OM_NOTZERO
    // );
    // S826_CounterPreloadWrite(board, countime, 0, datausec);    // Set period in microseconds.
    // int flags = S826_CounterStateWrite(board, countime, 1);    // Start the timer running.
    // if (flags < 0)
    //     printf("Timer Counter returned error code %d\n", flags);
}


void ConfigurePulsePerSecondCounter(int board, int countpps)
{
    // S826_CounterModeWrite(board, countpps,      // Configure counter:
    //                         S826_CM_K_AFALL );   // clock = ClkA (external digital signal)
    // S826_CounterFilterWrite(
    //             board, countpps,  // Filer Configuration:
    //             (1 << 31) | (1 << 30) | // Apply to IX and clock
    //             100 
    // );                  // 100x20ns = 2us filter constant
    // S826_CounterSnapshotConfigWrite(board, countpps,  // Acquire counts upon tick rising edge.
    //               S826_SSRMASK_IXRISE, S826_BITWRITE);
    // int flags = S826_CounterStateWrite(board, countpps, 1); // start the counter
    // if (flags < 0)
    //     printf("PPS Counter returned error code %d\n", flags);   
}

void ReadPPSSnapshot(void)
{
    fprintf(stdout, "PPS. \n");
    // flags = S826_CounterSnapshotRead(
    //     board, countpps,
    //     counts+sampcount, tstamp+sampcount, reason+sampcount, 0
    // );

    // if(flags == S826_ERR_OK || flags == S826_ERR_FIFOOVERFLOW){
    //     sprintf(t,"PPS:  Count = %d   Time = %.3fms   Reason = %x   Scnt = %d\n", counts[sampcount],
    //             (float)(tstamp[sampcount]-tstart)/1000.0, reason[sampcount], sampcount);
    //     printf(t);
    //     fprintf(outf, t);
    // }
}

void ReadQuadSnapshot(void)
{
    fprintf(stdout, "Quad. \n");
    // flags = S826_CounterSnapshotRead(
    //     board, countquad,
    //     counts+sampcount, 
    //     tstamp+sampcount, 
    //     reason+sampcount, 
    // 0);
    // while(flags == S826_ERR_OK || flags == S826_ERR_FIFOOVERFLOW){
      
    //     // Keep track of counts
    //     dcount = counts[sampcount] - lastcount;
    //     lastcount = counts[sampcount];
          
    //     // Print result
    //     sprintf(t,"Count = %d   Time = %.3fms   Reason = %x   Scnt = %d", counts[sampcount],
    //         (float)(tstamp[sampcount]-tstart)/1000.0, reason[sampcount], sampcount);
    //     fprintf(outf,"%s", t);
    //     if(printout) printf("%s\n" , t);
          
    //     // Check FIFO overflow
    //     if(flags == S826_ERR_FIFOOVERFLOW){
    //         errcount++;
    //         //sprintf(t,"  ####  FiFo Overflow samp=%d\n", sampcount);
    //         //strcat(s,t);
    //     fprintf(outf,"  ####  FiFo Overflow samp=%d\n", sampcount);
    //     } else {
    //         fprintf(outf,"\n");
    //     }
    //     // Increase counter
    //     sampcount++;

    //     // Read next snapshot
    //     flags = S826_CounterSnapshotRead(
    //         board, countquad,
    //         counts+sampcount, 
    //         tstamp+sampcount, 
    //         reason+sampcount, 
    //     0);
    // }
}

/*
 * Main Program Loop!
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

    // get handler to configuration file
    char filename[] = "../config/masterconfig.ini"; //argv[1];
    printf("configfile: %s\n", filename);
    printf("runtime: %s\n", argv[1]);
    ini_t *config = ini_load(filename);

    // Variables
    int board = 0;
    int countquad = 0;   // quadrature counter
    int countpps  = 1;   // pps counter
    int counttime = 2;   // timer counter

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

    // Configurations
    ConfigureSensorPower(board, config);
    ConfigureSensors(board, config);
    // ConfigurePulsePerSecondCounter();
    // ConfigureQuadCounter();
    // ConfigureTimerCounter();

    int *quad_intervals   = ini_get(config, "intervals", "quad_intervals");
    int *pps_intervals    = ini_get(config, "intervals", "pps_intervals");
    int *sensor_intervals = ini_get(config, "intervals", "sensor_intervals");
        
    printf("quad_intervals: %.3f\n", atof(quad_intervals));
    printf("pps_intervals: %.3f\n", atof(pps_intervals));
    printf("sensor_intervals: %.3f\n", atof(sensor_intervals));

    // BEGIN MAIN LOOP
    time(&rawtime);
    starttime = rawtime;

    //time_t curtime, quadlastreadtime, ppslastreadtime, sensorlastreadtime;
    uint64_t delta_us;
    struct timespec curtime, quadlastreadtime, ppslastreadtime, sensorlastreadtime;

    // set last read time as start. 
    clock_gettime(CLOCK_MONOTONIC_RAW, &quadlastreadtime);
    clock_gettime(CLOCK_MONOTONIC_RAW, &ppslastreadtime);
    clock_gettime(CLOCK_MONOTONIC_RAW, &sensorlastreadtime);

    while(rawtime - starttime < duration){
        
        clock_gettime(CLOCK_MONOTONIC_RAW, &curtime);
        delta_us = (curtime.tv_sec - quadlastreadtime.tv_sec) * 1000000000 + (curtime.tv_nsec - quadlastreadtime.tv_nsec);
        if (delta_us > (1000000000 * atof(quad_intervals))){
            ReadQuadSnapshot();
            clock_gettime(CLOCK_MONOTONIC_RAW, &quadlastreadtime);
        }

        clock_gettime(CLOCK_MONOTONIC_RAW, &curtime);
        delta_us = (curtime.tv_sec - ppslastreadtime.tv_sec) * 1000000000 + (curtime.tv_nsec - ppslastreadtime.tv_nsec);
        if (delta_us > (1000000000 * atof(pps_intervals))){
            ReadPPSSnapshot();
            clock_gettime(CLOCK_MONOTONIC_RAW, &ppslastreadtime);
        }

        clock_gettime(CLOCK_MONOTONIC_RAW, &curtime);
        delta_us = (curtime.tv_sec - sensorlastreadtime.tv_sec) * 1000000000 + (curtime.tv_nsec - sensorlastreadtime.tv_nsec);
        if (delta_us > (1000000000 * atof(sensor_intervals))){
            ReadSensorSnapshot();
            clock_gettime(CLOCK_MONOTONIC_RAW, &sensorlastreadtime);
        }

        // update time and loop
        time(&rawtime);
        loopcount++;
    }

    // close the s826 api
    S826_SystemClose();
}
