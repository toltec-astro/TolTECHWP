
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


struct p_args {
    int board;
    ini_t *config;
};

void *SensorThread(void *input){

    // some defs.
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

    // Configure Sensor Data
    ConfigureSensors(
        ((struct p_args*)input)->board, 
        ((struct p_args*)input)->config
    );

    char *countpps = ini_get(((struct p_args*)input)->config, "intervals", "sensor_intervals");
    int sensor_intervals = atoi(countpps);
    sensor_intervals = 15;

    while (1) {
        errcode = S826_AdcRead(0, slotval, NULL, &slotlist, S826_WAIT_INFINITE); 
        if (errcode != S826_ERR_OK)
            printf("ADC Read Failed. %d \n", errcode);

        // iterate through list of signals
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

int main(int argc, char **argv){
    // signal handler for abrupt/any close
    signal(SIGINT, SystemCloseHandler);

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
    printf('Power Configured');
    // define threads.
    pthread_t quad_thread, pps_thread, sensor_thread, write_thread;

    // start reading threads
    // pthread_create(&quad_thread, NULL, QuadThread, (void *)thread_args);
    // pthread_create(&pps_thread, NULL, PPSThread, (void *)thread_args);
    pthread_create(&sensor_thread, NULL, SensorThread, (void *)thread_args);
        
    // join back to main.
    pthread_join(quad_thread, NULL);
    pthread_join(pps_thread, NULL);
    pthread_join(sensor_thread, NULL);

    // break up the thing into multiple
    
    // free malloc'ed data.
    free(thread_args);
    
    // end.
    return 0;
}

/*
gcc -D_LINUX -Wall -Wextra -DOSTYPE_LINUX -c -no-pie readout.c ../vend/ini/ini.c
gcc -D_LINUX readout.o ini.o -no-pie -o readout -L ../vend/sdk_826_linux_3.3.11/demo -l826_64 -lpthread
*/
