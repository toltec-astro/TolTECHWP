#define S826_CM_XS_CH0         2
#define QUAD_BUFFER_LENGTH     400
#define PPS_BUFFER_LENGTH      40
#define ZEROPT_BUFFER_LENGTH   100
#define SENSOR_BUFFER_LENGTH   150
#define BUFSIZE                2048
#define PACKET_SIZE            8192
#define ANALOG_OUT_CNT         8
#define ANALOG_IN_CNT          16

#include "../vend/ini/ini.h"
#include "../vend/sdk_826_linux_3.3.11/demo/826api.h"

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdlib.h> 
#include <signal.h>
#include <pthread.h>
#include <netdb.h>
#include <math.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../vend/logging/logging.c"

// storage parameters
int sensor_in_ptr = 0;
int sensor_id[SENSOR_BUFFER_LENGTH];
uint sensor_cpu_time[SENSOR_BUFFER_LENGTH];
float sensor_voltage[SENSOR_BUFFER_LENGTH];

int zeropoint_in_ptr = 0;
uint zeropoint_id[ZEROPT_BUFFER_LENGTH];
uint zeropoint_cpu_time[ZEROPT_BUFFER_LENGTH];
uint zeropoint_card_time[ZEROPT_BUFFER_LENGTH];

int pps_in_ptr = 0;
uint pps_id[PPS_BUFFER_LENGTH];
uint pps_cpu_time[PPS_BUFFER_LENGTH];
uint pps_card_time[PPS_BUFFER_LENGTH];

int quad_in_ptr = 0;
uint quad_counter[QUAD_BUFFER_LENGTH];
uint quad_cpu_time[QUAD_BUFFER_LENGTH];
uint quad_card_time[QUAD_BUFFER_LENGTH];

int shutdown_flag = 0;

void ConfigureSensorPower(int board, ini_t *config)
{
    char scope[] = "ConfigureSensorPower";
    char logmsg[2000];

    // get voltage information
    char *range = ini_get(config, "sensors.power", "output_voltage_range");
    char *voltage_setpoint = ini_get(config, "sensors.power", "output_voltage_setpoint");

    // normal runmode / not safemode.
    int runmode = 0;

    for(int channel = 0; channel < ANALOG_OUT_CNT; ++channel)
    {
    int pwr = S826_DacRangeWrite(board, channel, atoi(range), runmode);
    if (pwr < 0)
    {
            printf("ConfigureSensorPower - Output Channel %i - S826_DacRangeWrite Error: %d\n", channel, pwr);
            exit(1);
        }
        
        int pwr_set = S826_DacDataWrite(board, channel, 0xFFFF, runmode);
        if (pwr_set = 0)
        {         
            printf("ConfigureSensorPower - Output Channel %i - S826_DacDataWrite Error: %d\n", channel, pwr_set);
        exit(1);
    }
    
        printf("ConfigureSensorPower - Output Channel %i - Power Configured\n", channel);
        sprintf(logmsg, "Output Channel %i - Power Configured\n", channel);
        //logInfo(scope, logmsg);
    }
}

void ConfigureSensors(int board, ini_t *config)
{
    char scope[] = "ConfigureSensors";
    char logmsg[2000];

    int err;
    // loop over and set all the sensors
    for(int sens = 0; sens < ANALOG_IN_CNT; ++sens)
    {
    int sensor_config = S826_AdcSlotConfigWrite(
            board, sens, sens, 5, S826_ADC_GAIN_1
        );
        if (sensor_config < 0)
        {
            printf("ConfigureSensors - Input Channel %i - S826_AdcSlotConfigWrite Error: %d\n", sens, sensor_config);
            exit(1);
        }
    }

    err = S826_AdcSlotlistWrite(board, 0xFFFF, S826_BITWRITE);
    if (err < 0)
    {
        printf("ConfigureSensors - S826_AdcSlotlistWrite Error: %d", err);
        exit(1);
    }
    
    err = S826_AdcTrigModeWrite(board, 0); 
    if (err < 0)
    {
        printf("ConfigureSensors - S826_AdcTrigModeWrite Error: %d", err);
        exit(1);
    }
    
    err = S826_AdcEnableWrite(board, 1);  
    if (err < 0)
    {
        printf("ConfigureSensors - S826_AdcEnableWrite Error: %d", err);
        exit(1);    
    }

    printf("ConfigureSensors - Analog Inputs Configured\n");
}

void ConfigurePulsePerSecondCounter(int board, ini_t *config)
{
    int pps_flags;
    char *countpps = ini_get(config, "counter.pps", "counter_num");

    pps_flags = S826_CounterModeWrite(
        board, 
        atoi(countpps),      // Configure counter:
        S826_CM_K_AFALL 
    );   // clock = ClkA (external digital signal)
    
    if (pps_flags < 0)
    {
        printf("ConfigurePulsePerSecondCounter - S826_CounterModeWrite Error: %d", pps_flags); 
        exit(1);
    }
    
    pps_flags = S826_CounterFilterWrite(
                board, atoi(countpps),  // Filer Configuration:
                (1 << 31) | (1 << 30) | // Apply to IX and clock
                100                     // 100x20ns = 2us filter constant
    );                 
    if (pps_flags < 0)
    {
        printf("ConfigurePulsePerSecondCounter - S826_CounterFilterWrite Error: %d", pps_flags);
        exit(1);
    } 

    pps_flags = S826_CounterSnapshotConfigWrite(
        board, 
        atoi(countpps),  
        S826_SSRMASK_IXRISE, // Acquire counts upon tick rising edge. 
        S826_BITWRITE
    );
    if (pps_flags < 0)
    {
        printf("ConfigurePulsePerSecondCounter - S826_CounterSnapshotConfigWrite Error: %d", pps_flags);
        exit(1);
    }

    pps_flags = S826_CounterStateWrite(board, atoi(countpps), 1); // start the counter
    if (pps_flags < 0)
    {
        printf("ConfigurePulsePerSecondCounter - S826_CounterStateWrite Error: %d", pps_flags);
        exit(1);
    }   
    printf("ConfigurePulsePerSecondCounter - Counter Channel %d- Configured PPS\n", atoi(countpps));
}

void ConfigureTimerCounter(int board, ini_t *config)
{
    int timer_flags;

    int *datausec = ini_get(config, "intervals", "carddata_intervals");
  
    int *counttime = ini_get(config, "counter.timer", "counter_num");
    timer_flags = S826_CounterModeWrite(board, atoi(counttime),  // Configure counter mode:
            S826_CM_K_1MHZ |                     // clock source = 1 MHz internal
            S826_CM_PX_START | S826_CM_PX_ZERO | // preload @start and counts==0
            S826_CM_UD_REVERSE |                 // count down
            S826_CM_OM_NOTZERO
    );

    // set period in microseconds
    timer_flags = S826_CounterPreloadWrite(board, atoi(counttime), 0, atoi(datausec));    
    if (timer_flags < 0)
    {
        printf("S826_CounterPreloadWrite Error: %d", timer_flags);
        exit(1);
    }
    
    // start the timer running.
    timer_flags = S826_CounterStateWrite(board, atoi(counttime), 1);    
    if (timer_flags < 0)
    {
        printf("S826_CounterStateWrite Error: %d", timer_flags);
        exit(1);
    }
    printf("ConfigureTimerCounter - Counter Channel %d - Configured Timer\n", atoi(counttime));
}

void ConfigureQuadCounter(int board, ini_t *config)
{
    int quad_flags;

    //printf("Configure QuadCounter \n");
    int *countquad = ini_get(config, "counter.quad", "counter_num");
    //printf("countquad (0): %d\n", atoi(countquad));    
    int *counttime = ini_get(config, "counter.timer", "counter_num");
    //printf("counttime (2): %d\n", atoi(counttime));    

    printf("ConfigureQuadCounter - Counter Channel %d - Using this channel for timer\n", atoi(counttime));
    quad_flags = S826_CounterModeWrite(
        board, 
        atoi(countquad),                   // Configure counter:
        S826_CM_K_QUADX4 |                 // Quadrature x1/x2/x4 multiplier
        //S826_CM_K_ARISE |                // clock = ClkA (external digital signal)
        //S826_XS_100HKZ |                 // route tick generator to index input
        (atoi(counttime) + S826_CM_XS_CH0) // route CH1 to Index input
    );   
    if (quad_flags < 0)
    {
        printf("ConfigureQuadCounter - S826_CounterModeWrite Error: %d", quad_flags);
        exit(1);
    }

    quad_flags = S826_CounterSnapshotConfigWrite(
        board, atoi(countquad),  // Acquire counts upon tick rising edge.
        S826_SSRMASK_IXRISE, 
        S826_BITWRITE
    );
    if (quad_flags < 0)
    {
        printf("ConfigureQuadCounter - S826_CounterSnapshotConfigWrite Error: %d", quad_flags);
        exit(1);
    }
  
    quad_flags = S826_CounterStateWrite(board, atoi(countquad), 1); // start the counter
    if (quad_flags < 0)
    {
        printf("ConfigureQuadCounter - S826_CounterStateWrite Error: %d", quad_flags);
        exit(1);
    }
    printf("ConfigureQuadCounter - Counter Channel %d - Configured Quadrature\n", atoi(countquad));
}

void ConfigureZeroPointCounter(int board, ini_t *config)
{
    int zeropoint_flags;
    char *countzeropoint = ini_get(config, "counter.zeropoint", "counter_num");

    zeropoint_flags = S826_CounterModeWrite(
        board, 
        atoi(countzeropoint),      // Configure counter:
        S826_CM_K_AFALL 
    );   // clock = ClkA (external digital signal)
    
    if (zeropoint_flags < 0)
    {
        printf("ConfigureZeroPointCounter - S826_CounterModeWrite Error: %d", zeropoint_flags); 
        exit(1);
    }
    zeropoint_flags = S826_CounterFilterWrite(
                board, atoi(countzeropoint),  // Filer Configuration:
                (1 << 31) | (1 << 30) | // Apply to IX and clock
                100                     // 100x20ns = 2us filter constant
    );                 
    if (zeropoint_flags < 0)
    {
        printf("ConfigureZeroPointCounter - S826_CounterFilterWrite Error: %d", zeropoint_flags); 
        exit(1);
    }

    zeropoint_flags = S826_CounterSnapshotConfigWrite(
        board, 
        atoi(countzeropoint),  
        S826_SSRMASK_IXRISE, // Acquire counts upon tick rising edge. 
        S826_BITWRITE
    );
    if (zeropoint_flags < 0)
    {
        printf("ConfigureZeroPointCounter - S826_CounterSnapshotConfigWrite Error: %d", zeropoint_flags); 
        exit(1);
    }
    
    zeropoint_flags = S826_CounterStateWrite(board, atoi(countzeropoint), 1); // start the counter
    {
        printf("ConfigureZeroPointCounter - S826_CounterStateWrite Error: %d", zeropoint_flags); 
        exit(1);
    }
    printf("ConfigureZeroPointCounter - Counter Channel %d - Configured Zero Point", atoi(countzeropoint));
}

struct p_args {
    int board;
    ini_t *config;
};

void *PPSThread(void *input){

    int board = ((struct p_args*)input)->board;
    int debug = 0;

    int errcode;
    int sampcount = 0;
    uint counts, tstamp, reason;

    // Configure PPS Counter
    ConfigurePulsePerSecondCounter(board, ((struct p_args*)input)->config);

    // read in the interval for pps
    char *countpps_id = ini_get(((struct p_args*)input)->config, "counter.pps", "counter_num");
    int countpps = atoi(countpps_id);

    // read in time interval
    char *pps_interval = ini_get(((struct p_args*)input)->config, "intervals", "pps_intervals");
    struct timespec pps_sleep_time;
    pps_sleep_time.tv_sec = atoi(pps_interval);
    pps_sleep_time.tv_nsec = 1000000000 * (atof(pps_interval) - atoi(pps_interval));

    // pps main loop
    while (1) {
        errcode = S826_CounterSnapshotRead(board, countpps, &counts, &tstamp, &reason, 0);
        
        if (errcode == S826_ERR_OK || errcode == S826_ERR_FIFOOVERFLOW){

            pps_id[pps_in_ptr] = counts;
            pps_cpu_time[pps_in_ptr] = time(NULL); // milliseconds 
            pps_card_time[pps_in_ptr] = tstamp;

            if (debug == 1) {
                printf("PPS: BufferPosition = %d \t Count = %d \t CPUTime = %d \t RawCardTime = %u \n", 
                     pps_in_ptr, pps_id[pps_in_ptr], pps_cpu_time[pps_in_ptr], pps_card_time[pps_in_ptr]);
            }
    
            pps_in_ptr++;
            
            // reset write-in head to start
            if (pps_in_ptr > PPS_BUFFER_LENGTH - 1){
                pps_in_ptr = 0;
                //if (debug == 1){
                //    printf("Reset to beginning %i! \n", BUFFER_LENGTH - 1);
                //}
            }
        }
        
        // wait designated time
        nanosleep(&pps_sleep_time, NULL);
    }
    return 0;
}

void *ZeroPointThread(void *input) {

    int board = ((struct p_args*)input)->board;
    int debug = 1;

    int errcode;
    int sampcount = 0;
    uint counts, tstamp, reason;

    // Configure Zero Point Counter
    ConfigureZeroPointCounter(board, ((struct p_args*)input)->config);

    // read in the interval for zeropoint
    char *countzeropoint_id = ini_get(((struct p_args*)input)->config, "counter.zeropoint", "counter_num");
    int countzeropoint = atoi(countzeropoint_id);

    // read in time interval
    char *zeropoint_interval = ini_get(((struct p_args*)input)->config, "intervals", "zeropoint_intervals");
    struct timespec zeropoint_sleep_time;
    zeropoint_sleep_time.tv_sec = atoi(zeropoint_interval);
    zeropoint_sleep_time.tv_nsec = 1000000000 * (atof(zeropoint_interval) - atoi(zeropoint_interval));

    // zero point main loop
    while (1) {
        errcode = S826_CounterSnapshotRead(board, countzeropoint, &counts, &tstamp, &reason, 0);
        
        if (errcode == S826_ERR_OK || errcode == S826_ERR_FIFOOVERFLOW){

            zeropoint_id[zeropoint_in_ptr] = counts;
            zeropoint_cpu_time[zeropoint_in_ptr] = time(NULL); // milliseconds 
            zeropoint_card_time[zeropoint_in_ptr] = tstamp;

            if (debug == 1) {
                printf("Zero Point: BufferPosition = %d \t Count = %d \t CPUTime = %d \t RawCardTime = %u \n", 
                    zeropoint_in_ptr, 
                    zeropoint_id[zeropoint_in_ptr], 
                    zeropoint_cpu_time[zeropoint_in_ptr],
                    zeropoint_card_time[zeropoint_in_ptr]);
            }
            zeropoint_in_ptr++;
            
            // reset write-in head to start
            if (zeropoint_in_ptr > ZEROPT_BUFFER_LENGTH - 1){
                zeropoint_in_ptr = 0;
            }
        }
        
        // wait designated time
        nanosleep(&zeropoint_sleep_time, NULL);
    }
    return 0;
}

void *SensorThread(void *input){

    int board = ((struct p_args*)input)->board;
    int debug = 0;
    uint slotlist = 0xFFFF;
    int errcode;             // errcode 
    int slotval[16];         // buffer must be sized for 16 slots
    int slot;    
    int adcdata;
    unsigned int burstnum;
    float voltage;

    // configure sensor data
    ConfigureSensors(board, ((struct p_args*)input)->config);

    // read in the interval for sensors
    char *sensor_interval = ini_get(((struct p_args*)input)->config, "intervals", "sensor_intervals");
    struct timespec sensor_wait_time;
    sensor_wait_time.tv_sec = atoi(sensor_interval);
    sensor_wait_time.tv_nsec = 1000000000 * (atof(sensor_interval) - atoi(sensor_interval));
    
   // begin reading loop
    while (1) {

        // read data
        errcode = S826_AdcRead(board, slotval, NULL, &slotlist, S826_WAIT_INFINITE); 
        if (errcode != S826_ERR_OK)
        {
            printf("SensorThread - S826_AdcRead Error: %d", errcode);
            exit(1);
        }

        // loop through sensors 
        for (slot = 0; slot < ANALOG_IN_CNT; slot++) 
        {
            // read in data (voltage in volts)
            adcdata = (int)((slotval[slot] & 0xFFFF));
            burstnum = ((unsigned int)slotval[slot] >> 24);
            voltage = (float)((float)adcdata / (float)(0x7FFF)) * 10;

            // update buffer
            sensor_id[sensor_in_ptr] = slot;
            sensor_cpu_time[sensor_in_ptr] = time(NULL); 
            sensor_voltage[sensor_in_ptr] = voltage;
            
            // print out data if debug
            if (debug == 1) {
                printf("SensorThread - Channel: %d \t Time: %u \t Slot: %u \t Voltage: %f \n", slot, 
                    sensor_cpu_time[sensor_in_ptr], sensor_id[sensor_in_ptr], sensor_voltage[sensor_in_ptr]
                );
            }

            // next spot in the buffer
            sensor_in_ptr++;

            // reset write in head to start
            if (sensor_in_ptr > SENSOR_BUFFER_LENGTH - 1){
                sensor_in_ptr = 0;
                if (debug == 1) {
                    printf("SensorThread - Reset to Sensor Buffer beginning %i! \n", SENSOR_BUFFER_LENGTH - 1);
                }
            }
        };

        // wait designated time
        nanosleep(&sensor_wait_time, NULL);
    }

    return 0;
}

void SystemCloseHandler(int sig)
{
    signal(sig, SIG_IGN);
    printf("\nSystemCloseHandler - Signal Caught!\n");

    // power shut off
    S826_SystemClose();
    printf("SystemCloseHandler - Power Shutoff + System Closed!\n");
    exit(0);
}

void SystemOpenHandler(void)
{
    int id;
    int flags = S826_SystemOpen();
    if (flags < 0)
    {
        printf("SystemOpenHandler - S826_SystemOpen Error %d\n", flags);
        exit(1);
    }
    else if (flags == 0)
    {
        printf("SystemOpenHandler - No boards were detected\n");
        exit(1);
    }
    else {
        printf("SystemOpenHandler - Boards were detected with these IDs:");
        for (id = 0; id < 16; id++) {
            if (flags & (1 << id))
                printf(" %d \n", id);
        }
    }
}

void *QuadThread(void *input){

    int debug = 0;
    int errcode;
    int sampcount = 0;
    int lastcount = 0;
    int dcount = 1;
    uint tstart;
    uint counts, tstamp, reason;


    int board = ((struct p_args*)input)->board;

    ConfigureTimerCounter(board, ((struct p_args*)input)->config);
    ConfigureQuadCounter(board, ((struct p_args*)input)->config);

    char *countquad_id = ini_get(((struct p_args*)input)->config, "counter.quad", "counter_num");
    int countquad = atoi(countquad_id);

    // interval between quad reads
    char *quad_interval = ini_get(((struct p_args*)input)->config, "intervals", "quad_intervals");
    struct timespec quad_sleep_time;
    quad_sleep_time.tv_sec = atoi(quad_interval);
    quad_sleep_time.tv_nsec = 1000000000 * (atof(quad_interval) - atoi(quad_interval));

    while (1) {
        errcode = S826_CounterSnapshotRead(board, countquad, &counts, &tstamp, &reason, 0);    
        // read until you can't anymore! FIFO style.
        while (errcode == S826_ERR_OK || errcode == S826_ERR_FIFOOVERFLOW){

            if (errcode == S826_ERR_FIFOOVERFLOW) {
                printf("QuadThread - Quadrature Counter: Error Overflow Detected.\n");            
            };
        
            // store     
            quad_counter[quad_in_ptr] = counts;
            quad_cpu_time[quad_in_ptr] = time(NULL);
            quad_card_time[quad_in_ptr] = tstamp;

            if (debug == 1) {
                printf("Quad: Buffer ID = %d, Count = %d, CardTime = %u, CPUTime = %u \n", 
                   quad_in_ptr, quad_counter[quad_in_ptr], quad_card_time[quad_in_ptr], quad_cpu_time[quad_in_ptr]);  
            }
            
            // increase counter
            quad_in_ptr++;

            // reset write-in head to start
            if (quad_in_ptr > QUAD_BUFFER_LENGTH - 1){
                quad_in_ptr = 0;
                if (debug == 1) {
                    printf("Reset to beginning %i! \n", QUAD_BUFFER_LENGTH - 1);
                }
            }

            // read next snapshot
            errcode = S826_CounterSnapshotRead(board, countquad, &counts, &tstamp, &reason, 0);
        }

        // sleep for the designated time
        nanosleep(&quad_sleep_time, NULL);
    }
    return 0;
}


void *GeneratePacket(void *input){
    char destination_ip[] = "localhost";

    int sockfd;                             /* socket */
    int portno;                             /* destination port */
    struct sockaddr_in destaddr;            /* destination address */                              
    int n;
    portno = 1213;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    inet_pton(AF_INET, destination_ip, &(destaddr.sin_addr));
    destaddr.sin_port = htons((unsigned short)portno);

    char ip_str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(destaddr.sin_addr), ip_str, INET_ADDRSTRLEN);
    printf("GeneratePacket - Packet Destination - %s::%i\n", ip_str, portno);

    int cycle = 0;

    // packet info
    time_t packettime;  
    char packetname[200];   
    // data packet buffer
    // (divide by size of an [uint/int/float])
    uint packet_buffer[PACKET_SIZE / 4];
    // pre buffer (in units of 4 bytes)
    // uint prepackbuffer[2];
    while (1) {


        // Quadrature Readout Data.
        // Put in the quadrature readout data.
        // quad_counter[], quad_card_time[]

        // read the top
        if ((quad_in_ptr < QUAD_BUFFER_LENGTH/2) && (cycle == 1)) {
            for (int i = QUAD_BUFFER_LENGTH/2; i < QUAD_BUFFER_LENGTH; i++) {
                packet_buffer[(i * 2)]     = quad_counter[i];
                packet_buffer[(i * 2) + 1] = quad_card_time[i];
            }
            cycle = 0;
        }

        // read the bottom
        if ((quad_in_ptr > QUAD_BUFFER_LENGTH/2) && (cycle == 0)){
            for (int i = 0; i < QUAD_BUFFER_LENGTH/2; i++) {                
                packet_buffer[i * 2] = quad_counter[i];
                packet_buffer[(i * 2) + 1] = quad_card_time[i];
            }
            cycle = 1;
        }

        // PPS: grab the last four (starting with the most recent)
        // write in pointer is: pps_in_ptr - 1 (after writing in)
        // pps_id[], pps_card_time[]
        int i_index;
        int pps_start_position = (PACKET_SIZE/4) - 9;
        for (int i = 1; i < 5; i++){
            if (pps_in_ptr - i < 0){
                i_index = PPS_BUFFER_LENGTH + (pps_in_ptr - i);
            } else {
                i_index = (pps_in_ptr - i);
            }

            packet_buffer[((i - 1) * 2 + pps_start_position)]     = pps_id[i_index];
            packet_buffer[((i - 1) * 2 + pps_start_position + 1)] = pps_card_time[i_index];                
        }       

        // ZERO POINT
        // grab the last four
        // zeropoint_id[], zeropoint_card_time[]
        int zeropt_start_position = (PACKET_SIZE / 4) - 17;
        for (int i = 1; i < 5; i++){
            if (zeropoint_in_ptr - i < 0){
                i_index = ZEROPT_BUFFER_LENGTH + (zeropoint_in_ptr - i);
            } else {
                i_index = (zeropoint_in_ptr - i);
            }

            packet_buffer[((i - 1) * 2 + zeropt_start_position)]     = zeropoint_id[i_index];
            packet_buffer[((i - 1) * 2 + zeropt_start_position + 1)] = zeropoint_card_time[i_index];  
        }

        // SENSORS
        // grab the last four for each sensor (4 * 16)
        // sensor_cpu_time[], sensor_id[], sensor_voltage[]
        int sensor_start_position = (PACKET_SIZE / 4) - 209; //53
        for (int i = 1; i < 65; i++){
            if (sensor_in_ptr - i < 0){
                i_index = SENSOR_BUFFER_LENGTH + (sensor_in_ptr - i);
            } else {
                i_index = (sensor_in_ptr - i);
            }
            packet_buffer[((i - 1) * 3 + sensor_start_position)]     = sensor_cpu_time[i_index];
            packet_buffer[((i - 1) * 3 + sensor_start_position + 1)] = sensor_id[i_index];
            packet_buffer[((i - 1) * 3 + sensor_start_position + 2)] = sensor_voltage[i_index];

    
            //printf("%d|%u %u| %0.3f \n", i_index, sensor_cpu_time[i_index], sensor_id[i_index], sensor_voltage[i_index]);
        }
        // TIMESTAMP */
        // get and package the timestamp at the end
        time(&packettime);
        packet_buffer[(PACKET_SIZE / 4) - 1] = packettime;

        // generate and save
        strftime(packetname, 100, "%y-%m-%d_%H-%M-%S_quad_data.data", localtime(&packettime));
        printf("%s %u\n", packetname, packettime);
        FILE *f = fopen(packetname, "wb");
        fwrite(packet_buffer, sizeof(char), sizeof(packet_buffer), f);
        fclose(f); 

        // send packet 
        n = sendto(sockfd, packet_buffer, sizeof(packet_buffer), 0, (struct sockaddr *)&destaddr, sizeof(destaddr));
    } 
    return 0;
}


void *ControlThread(void *input){
    
    int sockfd;                         /* socket */
    int portno;                         /* port to listen on */
    int clientlen;                      /* byte size of client's address */
    struct sockaddr_in serveraddr;      /* server's addr */
    struct sockaddr_in clientaddr;      /* client addr */
    struct hostent *hostp;              /* client host info */
    char *buf;                          /* message buf */
    char *hostaddrp;                    /* dotted decimal host addr string */
    int optval;                         /* flag value for setsockopt */
    int n;                              /* message byte size */

    portno = 8787;
    printf("ControlThread - Listening on port %i for commands\n", portno);

    // socket: create the parent socket 
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
        printf("ControlThread - Error opening socket");

    /* setsockopt: Handy debugging trick that lets 
     * us rerun the server immediately after we kill it; 
     * otherwise we have to wait about 20 secs. 
     * Eliminates "Error on binding: Address already in use" error. 
     */
    optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));

    // build the server's Internet address
    bzero((char *)&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)portno);

    // bind: associate the parent socket with a port 
    if (bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
        printf("ControlThread - Error on binding");

    // main loop: wait for a datagram, then echo it
    clientlen = sizeof(clientaddr);
    while (1) 
    {
        // recvfrom: receive a UDP datagram from a client
        buf = malloc(BUFSIZE);
        memset(buf, 0, BUFSIZE);
        n = recvfrom(sockfd, buf, BUFSIZE, 0,
                 (struct sockaddr *)&clientaddr, &clientlen);
        if (n < 0)
            printf("recvfrom Error\n");

        // gethostbyaddr: determine who sent the datagram
        hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr,
                      sizeof(clientaddr.sin_addr.s_addr),
                      AF_INET);
        if (hostp == NULL)
            printf("gethostbyaddr Error\n");        
    hostaddrp = inet_ntoa(clientaddr.sin_addr);
        if (hostaddrp == NULL)
            printf("inet_ntoa Error\n");

        printf("ControlThread - Command received: %s\n", buf); 

 
        if (strcmp(buf, "shutdown") == 0) {
            printf("ControlThread - Shutdown Command Received\n");  
            S826_SystemClose();
            
            char response[] = "ControlThread - Power Shutoff + System Closed!";
            printf("%s \n", response);
            n = sendto(sockfd, response, sizeof(response), 0, (struct sockaddr *)&clientaddr, clientlen);
            if (n < 0)
        printf("ControlThread - Reponse sendto Error\n");
            exit(0);    
        }

        if (strcmp(buf, "status") == 0) {
            printf("ControlThread - Status Command Received\n");

            printf("\n--- Data Retrieval Initiated ---\n");
            printf("Sensor Buffer Position: %i \t PPS Buffer Position: %i \t Quad Buffer Position: %i \n", 
                sensor_in_ptr, pps_in_ptr, quad_in_ptr      
            );      
            printf("PPS: BufferPosition = %i \t Count = %d \t CPUTime = %i \t RawCardTime = %f \n", 
                pps_in_ptr, pps_id[pps_in_ptr], pps_cpu_time[pps_in_ptr], pps_card_time[pps_in_ptr]
            );
            printf("Sensors: %i: \t Time: %i \t Slot: %d \t Voltage: %f \n", 
                sensor_in_ptr, sensor_cpu_time[sensor_in_ptr], sensor_id[sensor_in_ptr], sensor_voltage[sensor_in_ptr]
            );
            printf("Quad: BufferPosition = %i, Count = %d, CardTime = %.3f, CPUTime = %i \n", 
                quad_in_ptr, quad_counter[quad_in_ptr], quad_card_time[quad_in_ptr], quad_cpu_time[quad_in_ptr]
            );  
            printf("--- Data Retrieval Ended ---\n");
        }
        
        free(buf);
    }

    
    return 0;
}


void *PublishThread(void *input){
    int sockfd;                             /* socket */
    int portno;                             /* destination port */
    struct sockaddr_in destaddr;            /* destination address */
    int n;                              

    portno = 1212;
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    bzero((char *)&destaddr, sizeof(destaddr));
    destaddr.sin_family = AF_INET;
    destaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    destaddr.sin_port = htons((unsigned short)portno);

    char response[200];
    while (1) {
        sprintf(response, "BEGIN %i, %i, %d, %f END", sensor_in_ptr, sensor_cpu_time[sensor_in_ptr], sensor_id[sensor_in_ptr], sensor_voltage[sensor_in_ptr]);
            n = sendto(sockfd, response, sizeof(response), 0, (struct sockaddr *)&destaddr, sizeof(destaddr));
        printf("\n%i %i %d %f\n", sensor_in_ptr, sensor_cpu_time[sensor_in_ptr], sensor_id[sensor_in_ptr], sensor_voltage[sensor_in_ptr]);        
        sleep(1);
    }
    return 0;
}

int main(int argc, char **argv){
    // signal handler for abrupt/any close
    signal(SIGINT, SystemCloseHandler);

    if(argc < 1) {
        printf("Use: readout \n");
        printf("     configfilename \n");
        exit(0);
    }

    // set logging port
    setLogPort("10.105.114.14", 6969);

    // defined board number
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

    // TODO: iterate over when initing threads, no need to init individually?
    // TODO: can you pass them each a different function
    // TODO: reevaluate the necessity of the write to disk threads --> maybe remove them for now.

    // define threads.
    pthread_t control_thread, packet_thread;//, publish_thread;
    pthread_t quad_thread, pps_thread, sensor_thread;
    //pthread_t quad_write_thread, pps_write_thread, sensor_write_thread; 
    
    // start reading to buffer threads
    pthread_create(&quad_thread, NULL, QuadThread, (void *)thread_args);
    pthread_create(&pps_thread, NULL, PPSThread, (void *)thread_args);
    pthread_create(&sensor_thread, NULL, SensorThread, (void *)thread_args);   

    // start the writing threads
    //pthread_create(&quad_write_thread, NULL, QuadBufferToDisk, (void *)thread_args);
    //pthread_create(&pps_write_thread, NULL, PPSBufferToDisk, (void *)thread_args);
    //pthread_create(&sensor_write_thread, NULL, SensorBufferToDisk, (void *)thread_args);
    pthread_create(&control_thread, NULL, ControlThread, (void *)thread_args);   

    pthread_create(&packet_thread, NULL, GeneratePacket, (void *)thread_args);   
    //pthread_create(&publish_thread, NULL, PublishThread, (void *)thread_args);   
     
    // join back to main.
    pthread_join(control_thread, NULL);
    pthread_join(quad_thread, NULL);
    pthread_join(pps_thread, NULL);
    pthread_join(sensor_thread, NULL);
    pthread_join(packet_thread, NULL);

    // free malloc'ed data.
    free(thread_args);
}

/*
gcc -D_LINUX -Wall -Wextra -DOSTYPE_LINUX -c -no-pie readout.c ../vend/ini/ini.c
gcc -D_LINUX readout.o ini.o -no-pie -o readout -L ../vend/sdk_826_linux_3.3.11/demo -l826_64 -lpthread
gcc -D_LINUX -Wall -Wextra -DOSTYPE_LINUX -c -no-pie readout_serial.c ../vend/ini/ini.c
gcc -D_LINUX readout_serial.o ini.o -no-pie -o readout_serial -L ../vend/sdk_826_linux_3.3.11/demo -l826_64 -lpthread
*/