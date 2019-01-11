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

    // normal runmode / not safemode.
    int runmode = 0;

    int pwr = S826_DacRangeWrite(board, atoi(channel), atoi(range), runmode);
    if (pwr < 0)
        printf("Configure power error code %d", pwr);

    int pwr_set = S826_DacDataWrite(board, atoi(channel), 0xFFFF, runmode);
    if (pwr_set < 0)
        printf("Configure power data error code %d", pwr_set);
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

    // power shut off

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

void ConfigureQuadCounter(int board, ini_t *config)
{
    int quad_flags;

    printf("Configure QuadCounter \n");
    int *countquad = ini_get(config, "counter.quad", "counter_num");
    printf("countquad (0): %d\n", atoi(countquad));    
    int *counttime = ini_get(config, "counter.timer", "counter_num");
    printf("counttime (2): %d\n", atoi(counttime));    

    quad_flags = S826_CounterModeWrite(
        board, 
        atoi(countquad),                   // Configure counter:
        S826_CM_K_QUADX4 |                 // Quadrature x1/x2/x4 multiplier
        //S826_CM_K_ARISE |                // clock = ClkA (external digital signal)
        //S826_XS_100HKZ |                 // route tick generator to index input
        (S826_CM_XS_CH0 + atoi(counttime)) // route CH1 to Index input
    );   
    if (quad_flags < 0)
        printf("S826_CounterModeWrite returned error code %d\n", quad_flags);

    quad_flags = S826_CounterSnapshotConfigWrite(
        board, atoi(countquad),  // Acquire counts upon tick rising edge.
        S826_SSRMASK_IXRISE, 
        S826_BITWRITE
    );
    if (quad_flags < 0)
        printf("S826_CounterSnapshotConfigWrite returned error code %d\n", quad_flags);
  
    quad_flags = S826_CounterStateWrite(board, atoi(countquad), 1); // start the counter
    if (quad_flags < 0)
        printf("S826_CounterStateWrite returned error code %d\n", quad_flags);
}

void ConfigureTimerCounter(int board, ini_t *config)
{
    int timer_flags;

    int *datausec = ini_get(config, "intervals", "carddata_intervals");
    printf("datausec: %d\n", atoi(datausec));  

    int *counttime = ini_get(config, "counter.timer", "counter_num");
    printf("counttime (2): %d\n", atoi(counttime));    

    printf("Configure Timer Counter. \n");
    timer_flags = S826_CounterModeWrite(board, atoi(counttime),  // Configure counter mode:
            S826_CM_K_1MHZ |                     // clock source = 1 MHz internal
            S826_CM_PX_START | S826_CM_PX_ZERO | // preload @start and counts==0
            S826_CM_UD_REVERSE |                 // count down
            S826_CM_OM_NOTZERO
    );

    // set period in microseconds
    timer_flags = S826_CounterPreloadWrite(board, atoi(counttime), 0, atoi(datausec));    
    if (timer_flags < 0)
        printf("S826_CounterPreloadWrite returned error code %d\n", timer_flags);

    // start the timer running.
    timer_flags = S826_CounterStateWrite(board, atoi(counttime), 1);    
    if (timer_flags < 0)
        printf("S826_CounterStateWrite returned error code %d\n", timer_flags);
}

void ConfigurePulsePerSecondCounter(int board, ini_t *config)
{
    int pps_flags;

    int *countpps = ini_get(config, "counter.pps", "counter_num");
    printf("countpps: %d\n", atoi(countpps));  

    pps_flags = S826_CounterModeWrite(board, atoi(countpps),      // Configure counter:
                            S826_CM_K_AFALL );   // clock = ClkA (external digital signal)
    if (pps_flags < 0)
        printf("S826_CounterModeWrite returned error code %d\n", pps_flags); 

    pps_flags = S826_CounterFilterWrite(
                board, atoi(countpps),  // Filer Configuration:
                (1 << 31) | (1 << 30) | // Apply to IX and clock
                100 
    );                  // 100x20ns = 2us filter constant
    if (pps_flags < 0)
        printf("S826_CounterFilterWrite returned error code %d\n", pps_flags); 

    pps_flags = S826_CounterSnapshotConfigWrite(board, atoi(countpps),  // Acquire counts upon tick rising edge.
                  S826_SSRMASK_IXRISE, S826_BITWRITE);
    if (pps_flags < 0)
        printf("S826_CounterSnapshotConfigWrite returned error code %d\n", pps_flags); 

    pps_flags = S826_CounterStateWrite(board, atoi(countpps), 1); // start the counter
    if (pps_flags < 0)
        printf("S826_CounterStateWrite returned error code %d\n", pps_flags);   
}

void ReadPPSSnapshot(void)
{
    fprintf(stdout, "PPS. \n");
    int errcode;

    errcode = S826_CounterSnapshotRead(
        board, countpps,
        counts + sampcount, 
        tstamp + sampcount, 
        reason + sampcount, 
        0
    );
    if (errcode == S826_ERR_OK || errcode == S826_ERR_FIFOOVERFLOW){
        sprintf(t,"PPS:  Count = %d   Time = %.3fms   Reason = %x   Scnt = %d\n", 
            counts[sampcount], (float)(tstamp[sampcount] - tstart)/1000.0, reason[sampcount], sampcount);
        printf(t);
        fprintf(outf, t);
    }
}

void ReadQuadSnapshot(void)
{
    fprintf(stdout, "Quad. \n");
    flags = S826_CounterSnapshotRead(
        board, countquad,
        counts + sampcount, 
        tstamp + sampcount, 
        reason + sampcount, 
    0);
    
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
        flags = S826_CounterSnapshotRead(
            board, countquad,
            counts + sampcount, 
            tstamp + sampcount, 
            reason + sampcount, 
        0);
    }
}

void ReadSensorSnapshot(void)
{
    fprintf(stdout, "Sensor. \n");
    uint slotlist = 0x0007;

    int errcode;     // errcode 
    int slotval[16]; // buffer must be sized for 16 slots
    errcode = S826_AdcRead(0, slotval, NULL, &slotlist, S826_WAIT_INFINITE); 
    if (errcode != S826_ERR_OK)
        printf("ADC Read Failed. %d", errcode);

    int slot;
    for (slot = 0; slot < 3; slot++) 
    {
        int adcdata;
        unsigned int burstnum;
        float voltage;
        
        adcdata = (int)((slotval[slot] & 0xFFFF));
        burstnum = ((unsigned int)slotval[slot] >> 24);
        voltage = (float)((float)adcdata / (float)(0x7FFF)) * 10;
        printf("Slot: %d \t Voltage: %f \t ADCData: %d \t BurstNum %d \n", slot, voltage, adcdata, burstnum);   
    };
}

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
    
    // load configuration
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
    ConfigureTimerCounter(board, config);
    ConfigurePulsePerSecondCounter(board, config);
    ConfigureQuadCounter(board, config);

    int *quad_intervals   = ini_get(config, "intervals", "quad_intervals");
    int *pps_intervals    = ini_get(config, "intervals", "pps_intervals");
    int *sensor_intervals = ini_get(config, "intervals", "sensor_intervals");
        
    printf("quad_intervals: %.3f\n", atof(quad_intervals));
    printf("pps_intervals: %.3f\n", atof(pps_intervals));
    printf("sensor_intervals: %.3f\n", atof(sensor_intervals));

    time(&rawtime);
    starttime = rawtime;

    //time_t curtime, quadlastreadtime, ppslastreadtime, sensorlastreadtime;
    uint64_t delta_us;
    struct timespec curtime, quadlastreadtime, ppslastreadtime, sensorlastreadtime;

    // set last read time as start. 
    clock_gettime(CLOCK_MONOTONIC_RAW, &quadlastreadtime);
    clock_gettime(CLOCK_MONOTONIC_RAW, &ppslastreadtime);
    clock_gettime(CLOCK_MONOTONIC_RAW, &sensorlastreadtime);

    // priority flag
    int priority_flag = 0;

    //// begin reading loop
    while(rawtime - starttime < duration){
        
        priority_flag = 0;
        //// read from quadrature counter
        // get elapsed time
        clock_gettime(CLOCK_MONOTONIC_RAW, &curtime);
        delta_us = (curtime.tv_sec - quadlastreadtime.tv_sec) * 1000000000 + (curtime.tv_nsec - quadlastreadtime.tv_nsec);
        
        // read if elapsed
        if (delta_us > (1000000000 * atof(quad_intervals))){
            ReadQuadSnapshot();
            // update last read time
            clock_gettime(CLOCK_MONOTONIC_RAW, &quadlastreadtime);
        }

        //// read from pps counter
        // get elapsed time
        clock_gettime(CLOCK_MONOTONIC_RAW, &curtime);
        delta_us = (curtime.tv_sec - ppslastreadtime.tv_sec) * 1000000000 + (curtime.tv_nsec - ppslastreadtime.tv_nsec);
        
        // read if elapsed
        if (delta_us > (1000000000 * atof(pps_intervals))){
            ReadPPSSnapshot();
            // update last read time
            priority_flag = 1;
            clock_gettime(CLOCK_MONOTONIC_RAW, &ppslastreadtime);
        }

        //// read from sensors
        clock_gettime(CLOCK_MONOTONIC_RAW, &curtime);
        delta_us = (curtime.tv_sec - sensorlastreadtime.tv_sec) * 1000000000 + (curtime.tv_nsec - sensorlastreadtime.tv_nsec);
        if (delta_us > (1000000000 * atof(sensor_intervals)) && priority_flag == 0){
            ReadSensorSnapshot();
            
            // update last read time
            clock_gettime(CLOCK_MONOTONIC_RAW, &sensorlastreadtime);
        }

        // update time and loop
        time(&rawtime);
        loopcount++;
    }

    // power off the correct power channel
    char *channel = ini_get(config, "sensors.power", "output_channel");
    int pwr_set_off = S826_DacDataWrite(board, atoi(channel), 0x0000, 0);
    if (pwr_set_off < 0)
        printf("Configure power data off code %d.", pwr_set_off);

    // Close the API
    S826_SystemClose();
}
