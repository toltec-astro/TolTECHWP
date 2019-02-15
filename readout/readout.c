
#define S826_CM_XS_CH0     2


#include "../vend/ini/ini.h"
#include "../vend/sdk_826_linux_3.3.11/demo/826api.h"

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdlib.h> 
#include <signal.h>
#include <pthread.h>

void ConfigureSensorPower(int board, ini_t *config)
{
    // get voltage information
    char *range = ini_get(config, "sensors.power", "output_voltage_range");
    char *voltage_setpoint = ini_get(config, "sensors.power", "output_voltage_setpoint");
    char *channel = ini_get(config, "sensors.power", "output_channel");

    // print data
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

        // get the configuration settings
        int *timeslot = ini_get(config, sensor_id, "sensor_timeslot");
        int *channel = ini_get(config, sensor_id, "sensor_chan");
        
        printf("timeslot: %d\n", atoi(timeslot));
        printf("channel: %d\n", atoi(channel));

        int sensor_config_write = S826_AdcSlotConfigWrite(
            board, 
            atoi(timeslot), 
            atoi(channel), 
            5,
            S826_ADC_GAIN_1
        );
        if (sensor_config_write < 0)
            printf("Configure error %d\n", sensor_config_write);
    }

    int err_AdcSlotlistWrite = S826_AdcSlotlistWrite(board, 0x0007, S826_BITWRITE);
    if (err_AdcSlotlistWrite < 0)
        printf("S826_AdcSlotlistWrite error code %d", err_AdcSlotlistWrite);

    int err_AdcTrigModeWrite = S826_AdcTrigModeWrite(board, 0); 
    if (err_AdcTrigModeWrite < 0)
        printf("S826_AdcTrigModeWrite error code %d", err_AdcTrigModeWrite);

    int err_AdcEnableWrite = S826_AdcEnableWrite(board, 1);  
    if (err_AdcEnableWrite < 0)
        printf("S826_AdcEnableWrite error code %d", err_AdcEnableWrite);    
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
        (atoi(counttime) + S826_CM_XS_CH0) // route CH1 to Index input
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

    // set period in microseconds.
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

    pps_flags = S826_CounterModeWrite(
        board, 
        atoi(countpps),      // Configure counter:
        S826_CM_K_AFALL 
    );   // clock = ClkA (external digital signal)
    
    if (pps_flags < 0)
        printf("S826_CounterModeWrite returned error code %d\n", pps_flags); 

    pps_flags = S826_CounterFilterWrite(
                board, atoi(countpps),  // Filer Configuration:
                (1 << 31) | (1 << 30) | // Apply to IX and clock
                100                     // 100x20ns = 2us filter constant
    );                 
    if (pps_flags < 0)
        printf("S826_CounterFilterWrite returned error code %d\n", pps_flags); 

    pps_flags = S826_CounterSnapshotConfigWrite(
        board, 
        atoi(countpps),  // Acquire counts upon tick rising edge.
                  S826_SSRMASK_IXRISE, S826_BITWRITE);
    if (pps_flags < 0)
        printf("S826_CounterSnapshotConfigWrite returned error code %d\n", pps_flags); 

    pps_flags = S826_CounterStateWrite(board, atoi(countpps), 1); // start the counter
    if (pps_flags < 0)
        printf("S826_CounterStateWrite returned error code %d\n", pps_flags);   
}

void ReadPPSSnapshot(int board, int countpps, uint tstart)
{
    int errcode;
    int sampcount = 0;
    uint counts[1000], tstamp[1000], reason[1000];

    errcode = S826_CounterSnapshotRead(
        board, countpps,
        counts + sampcount, 
        tstamp + sampcount, 
        reason + sampcount, 
        0
    );
    printf("PPS:  flags=%d ok=%d fifooverflow=%d\n", errcode, S826_ERR_OK, S826_ERR_FIFOOVERFLOW);
    if (errcode == S826_ERR_OK || errcode == S826_ERR_FIFOOVERFLOW){
        printf("PPS:  Count = %d   Time = %.3fms   Reason = %x   Scnt = %d\n", 
            counts[sampcount], (float)(tstamp[sampcount] - tstart)/1000.0, reason[sampcount], sampcount);
    }
}

void ReadQuadSnapshot(int board, int countquad, uint tstart)
{
    int errcode;
    int sampcount = 0;
    int lastcount = 0;
    int dcount = 1;
    uint counts[1000], tstamp[1000], reason[1000];

    errcode = S826_CounterSnapshotRead(
        board, countquad,
        counts + sampcount, 
        tstamp + sampcount, 
        reason + sampcount, 
    0);
    
    while(errcode == S826_ERR_OK || errcode == S826_ERR_FIFOOVERFLOW){
      
        // Keep track of counts
        dcount = counts[sampcount] - lastcount;
        lastcount = counts[sampcount];
          
        // Print result
        printf("Quad: Count = %d   Time = %.3fms   Reason = %x   Scnt = %d", counts[sampcount],
            (float)(tstamp[sampcount]-tstart)/1000.0, reason[sampcount], sampcount);
        printf("\n");
        // Increase counter
        sampcount++;

        // Read next snapshot
        errcode = S826_CounterSnapshotRead(
            board, countquad,
            counts + sampcount, 
            tstamp + sampcount, 
            reason + sampcount, 
        0);
    }
}

void ReadSensorSnapshot(void)
{
    uint slotlist = 0x0007;

    int errcode;     // errcode 
    int slotval[16]; // buffer must be sized for 16 slots
    errcode = S826_AdcRead(0, slotval, NULL, &slotlist, S826_WAIT_INFINITE); 
    if (errcode != S826_ERR_OK)
        printf("ADC Read Failed. %d \n", errcode);

    int slot;
    for (slot = 0; slot < 3; slot++) 
    {
        int adcdata;
        unsigned int burstnum;
        float voltage;
        
        adcdata = (int)((slotval[slot] & 0xFFFF));
        burstnum = ((unsigned int)slotval[slot] >> 24);
        voltage = (float)((float)adcdata / (float)(0x7FFF)) * 10;
        printf("Slot: %d \t Voltage: %f \n", slot, voltage);   
    };
}

// arguments for threads
struct p_args {
    int board;
    int number;
    struct timespec time;
    uint tstart;
};

void *LoopQuadRead(void *input) {
    printf("Quad Read Thread Start\n"); 
    while (1) {
        ReadQuadSnapshot(
            ((struct p_args*)input)->board, 
            ((struct p_args*)input)->number, 
            ((struct p_args*)input)->tstart
        );
        // sleep(500);
        // sleep for 500000000L = 0.5 seconds
        nanosleep((const struct timespec[]) {{0, 500000000L}}, NULL);
    }
    return 0;
}

void *LoopPPSRead(void *input) {
    printf("PPS Read Thread Start\n");
    while (1) {
        ReadPPSSnapshot(
            ((struct p_args*)input)->board, 
            ((struct p_args*)input)->number, 
            ((struct p_args*)input)->tstart
        );  
        // implement sleep since our delay between
        // reads is not very short.
        sleep(1);
    }
    return 0;

}

void *LoopSensorRead(void *input){
    printf("Sensor Read Thread Start\n");
    while (1) {
        ReadSensorSnapshot();
        // implement sleep since our delay between
        // reads is not very short.
        sleep(15);
    }
    return 0;
}

void *WriteData(void *input){

    // write buffers 
    char s_buf[2000000], t_buf[200];

    // format filenames
    char quad_fname[100], pps_fname[100], sensors_fname[100];
    strftime(quad_fname, 100, "quad_data_%y-%m-%d_%H-%M.txt", localtime(&startime));
    strftime(pps_fname, 100, "pps_data_%y-%m-%d_%H-%M.txt", localtime(&startime));
    strftime(sensors_fname, 100, "sensors_data_%y-%m-%d_%H-%M.txt", localtime(&startime));
    
    // open the files
    FILE *quadf, *ppsf, *sensorf;
    // quadf = fopen(quad_fname, "wt");
    // ppsf = fopen(pps_fname, "wt");
    // sensorsf = fopen(sensors_fname, "wt");
    
    while(1){
        // check buffer
        // and write
        sleep(1);
    }
    // format the string
    // sprintf(t, "Count = %d Time = %.3fms Reason = %x Scnt = %d", 
    //     counts[sampcount], (float)(tstamp[sampcount]-tstart)/1000.0, reason[sampcount], sampcount
    // ); 
    // fprintf(quadf,"%s", t);

    // fclose(quadf);
    // fclose(ppsf);
    // fclose(sensorsf);

    return 0;
}


int main(int argc, char **argv){

    // signal handler for abrupt close
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
    int i; 
    time_t rawtime, starttime;
    char s[2000000], t[200];
    int *countpps = ini_get(config, "counter.pps", "counter_num");
    int *countquad = ini_get(config, "counter.quad", "counter_num");

    // Open the System
    SystemOpenHandler();

    // Configurations
    ConfigureSensorPower(board, config);
    ConfigureSensors(board, config);
    ConfigureTimerCounter(board, config);
    ConfigurePulsePerSecondCounter(board, config);
    ConfigureQuadCounter(board, config);
    
    // struct to hold time!
    struct timespec treq;
    struct timespec curtime;

    // create argument struct for quad
    struct p_args *thread_args_quad = (struct p_args *)malloc(sizeof(struct p_args));
    thread_args_quad->board = board;
    thread_args_quad->number = atoi(countquad);

    // create argument struct for quad.
    struct p_args *thread_args_pps = (struct p_args *)malloc(sizeof(struct p_args));
    thread_args_pps->board = board;  
    thread_args_pps->number = atoi(countpps);

    // create struct for sensor thread.
    struct p_args *thread_args_sensor = (struct p_args *)malloc(sizeof(struct p_args));
    thread_args_sensor->board = board;
        
    // set the time for all
    //thread_args_sensor->number = 0;
        
    // grab time from card.
    uint tstart;
    S826_TimestampRead(board, &tstart);
    thread_args_sensor->tstart = tstart;
    thread_args_quad->tstart = tstart;    
    thread_args_pps->tstart = tstart;  

    // set the time for all.
    clock_gettime(CLOCK_MONOTONIC_RAW, &curtime);
    thread_args_sensor->time = curtime;
    thread_args_quad->time = curtime;    
    thread_args_pps->time = curtime;  

    // define threads
    pthread_t quad_thread, pps_thread, sensor_thread, write_thread;

    // start all the threads.
    pthread_create(&quad_thread, NULL, LoopQuadRead, (void *)thread_args_quad);
    pthread_create(&pps_thread, NULL, LoopPPSRead, (void *)thread_args_pps);
    pthread_create(&sensor_thread, NULL, LoopSensorRead, (void *)thread_args_sensor);
    
    // this is one takes nothing right now
    pthread_create(&write_thread, NULL, WriteData, (void *)thread_args_sensor);

    // join back to main.
    pthread_join(quad_thread, NULL);
    pthread_join(pps_thread, NULL);
    pthread_join(sensor_thread, NULL);

    // free malloc'ed data.
    free(thread_args_quad);
    free(thread_args_pps);
    free(thread_args_sensor);

    // return 0
    return 0;
}
/*
gcc -D_LINUX -Wall -Wextra -DOSTYPE_LINUX -c -no-pie readout.c ../vend/ini/ini.c
gcc -D_LINUX readout.o ini.o -no-pie -o readout -L ../vend/sdk_826_linux_3.3.11/demo -l826_64 -pthread 

*/