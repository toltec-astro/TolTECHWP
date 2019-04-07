
#define S826_CM_XS_CH0    2
#define BUFFER_LENGTH     8000

#include "../vend/ini/ini.h"
#include "../vend/sdk_826_linux_3.3.11/demo/826api.h"

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdlib.h> 
#include <signal.h>
#include <pthread.h>
#include <sys/socket.h>


void ConfigureSensorPower(int board, ini_t *config)
{
    // get voltage information
    char *range = ini_get(config, "sensors.power", "output_voltage_range");
    char *voltage_setpoint = ini_get(config, "sensors.power", "output_voltage_setpoint");
    char *channel = ini_get(config, "sensors.power", "output_channel");

    // print data
    // printf("range: %d\n", atoi(range));
    // printf("voltage_setpoint: %d\n", (int)strtol(voltage_setpoint, NULL, 0));
    // printf("channel: %d\n", atoi(channel));

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
    char *sens_count = ini_get(config, "sensors", "sensor_num");
    printf("sensor count: %d\n", atoi(sens_count));    
    //char buf[2];

    // loop over and set all the sensors
    char sensor_id[30];
    for(int sens = 1; sens <= atoi(sens_count); ++sens)
    {
        printf("sensor number: %d\n", sens);
        
        //// construct the string
        sprintf(sensor_id, "sensors.details.%02d", sens);

        printf("-------------\n");
        printf("id: %s\n", sensor_id);

        // get the configuration settings
        char *timeslot = ini_get(config, sensor_id, "sensor_timeslot");
        char *channel = ini_get(config, sensor_id, "sensor_chan");
        
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

void ConfigurePulsePerSecondCounter(int board, ini_t *config)
{
    int pps_flags;

    char *countpps = ini_get(config, "counter.pps", "counter_num");
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
        atoi(countpps),  
        S826_SSRMASK_IXRISE, // Acquire counts upon tick rising edge. 
        S826_BITWRITE
    );
    if (pps_flags < 0)
        printf("S826_CounterSnapshotConfigWrite returned error code %d\n", pps_flags); 

    pps_flags = S826_CounterStateWrite(board, atoi(countpps), 1); // start the counter
    if (pps_flags < 0)
        printf("S826_CounterStateWrite returned error code %d\n", pps_flags);   
}


struct p_args {
    int board;
    ini_t *config;
};

void *PPSThread(void *input){

    int errcode;
    int sampcount = 0;
    uint counts[1000], tstamp[1000], reason[1000];

    ConfigurePulsePerSecondCounter(
        ((struct p_args*)input)->board, 
        ((struct p_args*)input)->config
    );

    // read in the interval for pps
    char *pps_interval = ini_get(((struct p_args*)input)->config, "intervals", "pps_intervals");
    int pps_intervals = atoi(pps_interval);

    // storage parameters
    int pps_in_ptr = 0;
    int pps_id[BUFFER_LENGTH];
    int pps_cpu_time[BUFFER_LENGTH];
    int pps_card_time[BUFFER_LENGTH];

    while (1) {

        errcode = S826_CounterSnapshotRead(
            board, countpps,
            counts + sampcount, 
            tstamp + sampcount, 
            reason + sampcount, 
            0
        );

        if (errcode == S826_ERR_OK || errcode == S826_ERR_FIFOOVERFLOW){
            printf("PPS:  Count = %d   Time = %.3fms   Reason = %x   Scnt = %d\n", 
                    counts[sampcount], (float)(tstamp[sampcount] - tstart)/1000.0, reason[sampcount], sampcount);
        }    
        sleep(pps_intervals);
    }

    return 0;
}

void *SensorThread(void *input){

    uint slotlist = 0x0007;
    int errcode;            // errcode 
    int slotval[16];         // buffer must be sized for 16 slots
    int slot;
    
    int adcdata;
    unsigned int burstnum;
    float voltage;

    // storage parameters
    int sensor_in_ptr = 0;
    int sensor_id[BUFFER_LENGTH];
    int sensor_cpu_time[BUFFER_LENGTH];
    float sensor_voltage[BUFFER_LENGTH];

    // configure sensor data
    ConfigureSensors(
        ((struct p_args*)input)->board, 
        ((struct p_args*)input)->config
    );

    // read in the interval for sensors
    char *sensor_interval = ini_get(((struct p_args*)input)->config, "intervals", "sensor_intervals");
    int sensor_intervals = atoi(sensor_interval);
    
    // begin reading loop
    while (1) {

        errcode = S826_AdcRead(0, slotval, NULL, &slotlist, S826_WAIT_INFINITE); 
        if (errcode != S826_ERR_OK)
            printf("ADC Read Failed. %d \n", errcode);

        for (slot = 0; slot < 3; slot++) 
        {
            adcdata = (int)((slotval[slot] & 0xFFFF));
            burstnum = ((unsigned int)slotval[slot] >> 24);
            voltage = (float)((float)adcdata / (float)(0x7FFF)) * 10;

            // this is where we will update the array
            // printf("Slot: %d \t Voltage: %f \n", slot, voltage);   
            sensor_id[sensor_in_ptr] = slot;
            sensor_cpu_time[sensor_in_ptr] = slot;
            sensor_voltage[sensor_in_ptr] = voltage;
            printf("Slot: %d \t Voltage: %f \n", sensor_id[sensor_in_ptr], sensor_voltage[sensor_in_ptr]);   
            sensor_in_ptr++;

            // reset write in head to start
            if (sensor_in_ptr > BUFFER_LENGTH - 1)
                sensor_in_ptr = 0;
                printf('Reset to beginning!');
        };

        // wait designated time
        sleep(sensor_intervals);
    }

    return 0;
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

void *QuadThread(void *input){
    
    int errcode;
    int sampcount = 0;
    int lastcount = 0;
    int dcount = 1;

    ConfigureTimerCounter(
        ((struct p_args*)input)->board, 
        ((struct p_args*)input)->config
    );

    ConfigureQuadCounter(
        ((struct p_args*)input)->board, 
        ((struct p_args*)input)->config
    );

    while (1) {

        errcode = S826_CounterSnapshotRead(
            board, countquad,
            counts + sampcount, 
            tstamp + sampcount, 
            reason + sampcount, 
        0);
        
        while(errcode == S826_ERR_OK || errcode == S826_ERR_FIFOOVERFLOW){
          
            // keep track of counts
            dcount = counts[sampcount] - lastcount;
            lastcount = counts[sampcount];
              
            // this is where we will update the array
            printf("Quad: Count = %d   Time = %.3fms   Reason = %x   Scnt = %d", counts[sampcount],
                (float)(tstamp[sampcount]-tstart)/1000.0, reason[sampcount], sampcount);
            printf("\n");
            
            // increase counter
            sampcount++;

            // read next snapshot
            errcode = S826_CounterSnapshotRead(
                board, countquad,
                counts + sampcount, 
                tstamp + sampcount, 
                reason + sampcount, 
            0);
        }
    }
    return 0;
}

int main(int argc, char **argv){
    // signal handler for abrupt/any close
    signal(SIGINT, SystemCloseHandler);

    if( argc < 1 ) {
        printf("Use: readout \n");
        printf("     configfilename \n");
        exit(0);
    }

    int board = 0;

    // read config into memory.
    char filename[] = "../config/masterconfig.ini";
    ini_t *config = ini_load(filename);

    // argument for the various threads
    struct p_args *thread_args = (struct p_args *)malloc(sizeof(struct p_args));
    thread_args->board = board;  
    thread_args->config = config;

    // start the power from the board.
    SystemOpenHandler();
    ConfigureSensorPower(board, config);

    // define threads.
    pthread_t quad_thread, pps_thread, sensor_thread, write_thread;
    
    // start reading threads
    // pthread_create(&quad_thread, NULL, QuadThread, (void *)thread_args);
    // pthread_create(&pps_thread, NULL, PPSThread, (void *)thread_args);
    pthread_create(&sensor_thread, NULL, SensorThread, (void *)thread_args);    

    // join back to main.
    // pthread_join(quad_thread, NULL);
    // pthread_join(pps_thread, NULL);
    pthread_join(sensor_thread, NULL);

    // break up the thing into multiple
    
    // free malloc'ed data.
    free(thread_args);
    
    // end.
    // return 0;
}

/*
gcc -D_LINUX -Wall -Wextra -DOSTYPE_LINUX -c -no-pie readout.c ../vend/ini/ini.c
gcc -D_LINUX readout.o ini.o -no-pie -o readout -L ../vend/sdk_826_linux_3.3.11/demo -l826_64 -lpthread
*/
