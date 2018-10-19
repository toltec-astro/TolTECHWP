#define S826_CM_XS_CH0     2
#define printout 0
#define SAMPLING_PERIOD 500000
#define TSETTLE SAMPLING_PERIOD - 5

#include "826api.h"
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdlib.h> 

// TODO: convert individual configurations into a struct

void AdcHandler(void)
{
    int errcode;
    int slotval[16]; // buffer must be sized for 16 slots

    uint slotlist = 0xE000; 
    errcode = S826_AdcRead(0, slotval, NULL, &slotlist, 1000); // wait for IRQ
    if (errcode != S826_ERR_OK)
        printf("ADC Read Failed. %d", errcode);

    fprintf(stdout, "Raw Channel 0 Data = %d \n", slotval[0]);
    fprintf(stdout, "Raw Channel 1 Data = %d \n", slotval[1]);
    fprintf(stdout, "Raw Channel 2 Data = %d \n", slotval[2]);

}

char t[200];
int main(int argc, char **argv){

    if( argc < 2 ) {
        printf("Use: ./housekeeping duration\n");
        printf("     duration = how long experiment is (seconds)\n");
        exit(0);
    }

    //// Preparation
    // Set timer counter interval: 
    // Determines how often count data is stored.
    int duration = atoi(argv[1]); // seconds

    int board = 0;   // only one board
    int runmode = 0; // use runmode

    // ------ Begin Configurations for Housekeeping Sensors ------
    /* 
     * Power Output for Sensors
     *  Function: Analog output channel 1
     *    - Pin = 44 
     *    - Name: AOUT1
     *  Range: 0 to +5 V
     */
    int pwr_output_channel = 1; 
    int pwr_output_range = 0;

    /* 
     * Temperature Sensor 1 Input
     *  Function: Analog input (+) channel 0
     *    - Pin = 4
     *    - Name: +AIN0
     *  Function: Analog input (-) channel 0
     *    - Pin = 3 
     *    - Name: -AIN0
     */
    int temp_1_channel = 0;
    int temp_1_settling = TSETTLE;
    int temp_1_range = 0;
    int temp_1_timeslot = 1;

    /* 
     * Temperature Sensor 2 Input
     *  Function: Analog input (+) channel 1
     *    - Pin = 
     *    - Name: +AIN1
     *  Function: Analog input (-) channel 1
     *    - Pin = 
     *    - Name: -AIN1
     */
    int temp_2_channel = 1;
    int temp_2_settling = TSETTLE;
    int temp_2_range = 0;
    int temp_2_timeslot = 2;

    /* 
     * Humidity Sensor Input 1
     *  Function: Analog input (+) channel 2
     *    - Pin = 
     *    - Name: +AIN2
     *  Function: Analog input (-) channel 2
     *    - Pin = 
     *    - Name: -AIN2
     */
    int hum_1_channel = 2;
    int hum_1_settling = TSETTLE;
    int hum_1_range = 0;
    int hum_1_timeslot = 3;

    // ------ End Configurations for Housekeeping Sensors

    // ------ Begin S826 API
    int flags = S826_SystemOpen();
    if (flags < 0)
    printf("S826_SystemOpen returned error code %d", flags);

    // configure power output for sensors
    int pwr = S826_DacRangeWrite(board, pwr_output_channel, pwr_output_range, runmode);
    if (pwr < 0)
        printf("Configure power error code %d", pwr);

    // TODO: LOOP OVER TO SET SETTINGS

    // configure power input for temperature 1 sensor
    int temp_1 = S826_AdcSlotConfigWrite(board, 
        temp_1_timeslot, 
        temp_1_channel, 
        temp_1_settling,
        temp_1_range
    );

    // configure power input for temperature 2 sensor
    int temp_2 = S826_AdcSlotConfigWrite(board, 
        temp_2_timeslot, 
        temp_2_channel, 
        temp_2_settling,
        temp_2_range
    );

    // configure power input for humidity sensor 1
    int hum_1 = S826_AdcSlotConfigWrite(board, 
        hum_1_timeslot, 
        hum_1_channel, 
        hum_1_settling,
        hum_1_range
    );

    // enable the slots in the slot list
    // 0xE000 = is the first three 
    // trigger mode = continuous
    // enable the board 
    S826_AdcSlotlistWrite(board, 0xE000, S826_BITWRITE);
    S826_AdcTrigModeWrite(board, 0); 
    S826_AdcEnableWrite(board, 1);  
    
    // time keeping
    time_t rawtime, startime;
    time(&rawtime);
    startime = rawtime;
    
    
    while (rawtime - startime < duration){

        AdcHandler();
        
        // update time and sleep time
        time(&rawtime);
        nanosleep((const struct timespec[]){{0, 500000000L}}, NULL);
    }

    // ------ Close S826 API ------
    S826_SystemClose();

};