#define S826_CM_XS_CH0     2
#define printout 0
#define SAMPLING_PERIOD 50000         // Sampling period in microseconds (50000 = 20 samples/s).
#define TSETTLE SAMPLING_PERIOD - 3;  // Compensate for nominal ADC conversion time.


#include "826api.h"
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdlib.h> 



void AdcHandler(void)
{
  int errcode;
  int adcdata[16]; 
  int slotval[16];  // buffer must be sized for 16 slots
  while (1) {
    uint slotlist = 1;  // only slot 0 is of interest in this example
    errcode = S826_AdcRead(0, adcdata, NULL, &slotlist, S826_WAIT_INFINITE); // wait for IRQ
    if (errcode != S826_ERR_OK)
      break;
    printf("Raw adc data = %d", slotval[0] & 0xFFFF);
  }
}


char t[200];
int main(int argc, char **argv){

    if( argc < 2 ) {
        printf("Use: ./housekeeping duration\n");
        printf("     duration = how long experiment is (seconds)\n");
        exit(0);
    }    
    int board = 0;
    int runmode = 0; // use runmode

    /* 
     * Power Output for Sensors
     *  Function: Analog output channel 1
     *    - Pin = 44 
     *    - Name: AOUT1
     *  Range: 0 to +5 V
     */
    int pwr_output_channel = 1; 
    int pwr_output_range = 0;
    int pwr_output_setpoint = 0xFFFF;

    //begin power
    int flags = S826_SystemOpen();
    if (flags < 0)
    printf("S826_SystemOpen returned error code %d", flags);

    // configure power output for sensors
    int pwr = S826_DacRangeWrite(board, pwr_output_channel, pwr_output_range, runmode);
    if (pwr < 0)
        printf("Configure power error code %d", pwr);

    int pwr_set = S826_DacDataWrite(board, pwr_output_channel, pwr_output_setpoint, runmode);
    if (pwr_set < 0)
        printf("Configure power data error code %d", pwr);

    // Configure the ADC subsystem and start it running
    S826_AdcSlotConfigWrite(board, 0, 0, TSETTLE, S826_ADC_GAIN_1); // measuring ain 0 on slot 0
    S826_AdcSlotlistWrite(board, 1, S826_BITWRITE);                 // enable slot 0
    S826_AdcTrigModeWrite(board, 0);                                // trigger mode = continuous
    S826_AdcEnableWrite(board, 1);                                  // start adc conversions


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

    // explicitly set power to zero
    int pwr_set_off = S826_DacDataWrite(board, pwr_output_channel, 0x0000, runmode);
    if (pwr_set_off < 0)
        printf("Configure power data off code %d", pwr);

    // ------ Close S826 API ------
    S826_SystemClose();

};