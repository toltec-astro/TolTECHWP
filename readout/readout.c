#define S826_CM_XS_CH0         2
#define QUAD_BUFFER_LENGTH     8000
#define BUFFER_LENGTH          20
#define BUFSIZE                2048

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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


// storage parameters
int sensor_in_ptr = 0;
int sensor_id[BUFFER_LENGTH];
int sensor_cpu_time[BUFFER_LENGTH];
float sensor_voltage[BUFFER_LENGTH];

int pps_in_ptr = 0;
int pps_id[BUFFER_LENGTH];
int pps_cpu_time[BUFFER_LENGTH];
float pps_card_time[BUFFER_LENGTH];

int quad_in_ptr = 0;
int quad_counter[QUAD_BUFFER_LENGTH];
int quad_cpu_time[QUAD_BUFFER_LENGTH];
float quad_card_time[QUAD_BUFFER_LENGTH];

int shutdown_flag = 0;

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
                printf("PPS: BufferPosition = %i \t Count = %d \t CPUTime = %i \t RawCardTime = %f \n", 
                     pps_in_ptr, pps_id[pps_in_ptr], pps_cpu_time[pps_in_ptr], pps_card_time[pps_in_ptr]);
            }
    
            pps_in_ptr++;
            
            // reset write-in head to start
            if (pps_in_ptr > BUFFER_LENGTH - 1){
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

void *SensorThread(void *input){

    int board = ((struct p_args*)input)->board;
    int debug = 0;

    // get sensor count
    char *sensor_count = ini_get(((struct p_args*)input)->config, "sensors", "sensor_num");
    int sens_cnt = atoi(sensor_count);

    // TODO: is there a programmatic way to grab this?
    // !(0xFFFF<< N)
    // uint slotlist = 0x0007;
    // uint slotlist = !(0xFFFF << sens_cnt);
    uint slotlist = 0x0007;

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
            printf("ADC Read Failed. %d \n", errcode);

        // loop through sensors 
        for (slot = 0; slot < sens_cnt; slot++) 
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
                printf("Sensors: %i \t Time: %i \t Slot: %d \t Voltage: %f \n", sensor_in_ptr, 
                    sensor_cpu_time[sensor_in_ptr], sensor_id[sensor_in_ptr], sensor_voltage[sensor_in_ptr]
                );
            }

            // next spot in the buffer
            sensor_in_ptr++;

            // reset write in head to start
            if (sensor_in_ptr > BUFFER_LENGTH - 1){
                sensor_in_ptr = 0;
                if (debug == 1) {
                    printf("Reset to beginning %i! \n", BUFFER_LENGTH - 1);
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
    printf("\nSignal Caught!\n");

    // power shut off
    S826_SystemClose();
    printf("Power Shutoff + System Closed!\n");
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
                printf("Quadrature Counter: Error Overflow Detected.\n");            
            };
        
            // store     
            quad_counter[quad_in_ptr] = counts;
            quad_cpu_time[quad_in_ptr] = time(NULL);
            quad_card_time[quad_in_ptr] = tstamp;

            if (debug == 1) {
                printf("Quad: Buffer ID = %i, Count = %d, CardTime = %.3f, CPUTime = %i \n", 
                   quad_in_ptr, quad_counter[quad_in_ptr], quad_card_time[quad_in_ptr], quad_cpu_time[quad_in_ptr]);  
            }
            
            // increase counter
            quad_in_ptr++;

            // reset write-in head to start
            if (quad_in_ptr > QUAD_BUFFER_LENGTH - 1){
                quad_in_ptr = 0;
                if (debug == 1) {
                    printf("Reset to beginning %i! \n", BUFFER_LENGTH - 1);
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

void *QuadBufferToDisk(void *input){


    // get file path
    char *savefilepath = ini_get(((struct p_args*)input)->config, "savefilepath", "path");
    
    // generate file name
    char fname[200];
    time_t save_date;
    time(&save_date);
    strftime(fname, 100, "quad_data_%y-%m-%d_%H-%M.txt", localtime(&save_date));

    // construct final location
    char full_path[200];
    sprintf(full_path, "%s%s", savefilepath, fname);

    // set location 
    FILE *outfile;
    outfile = fopen(full_path, "wt");

    // text out buffer
    char txt_out_buf[500];
    int cycle = 0;

    // print header
    fprintf(outfile, "buffer_id, count, cardcounter, computertime\n");

    while (1) {
        // if the bottom is ready  printf("Quad Bottom: %i, Quad Top: %i \n", quad_bot_flag, quad_top_flag);
        
        // read the bottom
        if ((quad_in_ptr > QUAD_BUFFER_LENGTH/2) && (cycle == 0)){
            
            for (int i = 0; i < QUAD_BUFFER_LENGTH/2; i++) {

                sprintf(txt_out_buf, "%i, %d, %.3f, %i \n", 
                      i, quad_counter[i], quad_card_time[i], quad_cpu_time[i]
                );
                fprintf(outfile, "%s", txt_out_buf);
                continue;
            }
            cycle = 1;
        }

        // read the top
        if ((quad_in_ptr < QUAD_BUFFER_LENGTH/2) && (cycle == 1)){
            for (int i = QUAD_BUFFER_LENGTH/2; i < QUAD_BUFFER_LENGTH; i++) {

                sprintf(txt_out_buf, "%i, %d, %.3f, %i \n", 
                      i, quad_counter[i], quad_card_time[i], quad_cpu_time[i]
                );
                fprintf(outfile, "%s", txt_out_buf);
                continue;
            }
            cycle = 0;
        }
    } 
    return 0;
}


void *SensorBufferToDisk(void *input){

    // get file path
    char *savefilepath = ini_get(((struct p_args*)input)->config, "savefilepath", "path");
    
    // generate file name
    char fname[200];
    time_t save_date;
    time(&save_date);
    strftime(fname, 100, "sensor_data_%y-%m-%d_%H-%M.txt", localtime(&save_date));

    // construct final location
    char full_path[200];
    sprintf(full_path, "%s%s", savefilepath, fname);

    // set location 
    FILE *outfile;
    outfile = fopen(full_path, "wt");

    // text out buffer
    char txt_out_buf[500];
    int cycle = 0;

    // print header
    fprintf(outfile, "buffer_id, sensor_id, sensor_voltage_volts, computertime\n");

    while (1) {
        
        // read the bottom
        if ((sensor_in_ptr > BUFFER_LENGTH/2) && (cycle == 0)){
            
            for (int i = 0; i < BUFFER_LENGTH/2; i++) {

                sprintf(txt_out_buf, "%i, %i, %f, %d \n", 
                      i, sensor_id[i], sensor_voltage[i], sensor_cpu_time[i]
                );
                fprintf(outfile, "%s", txt_out_buf);
                continue;            
            }
            cycle = 1;
        }

        // read the top
        if ((sensor_in_ptr < BUFFER_LENGTH/2) && (cycle == 1)){
            for (int i = BUFFER_LENGTH/2; i < BUFFER_LENGTH; i++) {
           

                sprintf(txt_out_buf, "%i, %i, %f, %d  \n", 
                      i, sensor_id[i], sensor_voltage[i], sensor_cpu_time[i]
                );
                fprintf(outfile, "%s", txt_out_buf);
                continue;
            }
            cycle = 0;
        }
        sleep(3);
    } 
    return 0;
}

void *PPSBufferToDisk(void *input){

    // get file path
    char *savefilepath = ini_get(((struct p_args*)input)->config, "savefilepath", "path");
    
    // generate file name
    char fname[200];
    time_t save_date;
    time(&save_date);
    strftime(fname, 100, "pps_data_%y-%m-%d_%H-%M.txt", localtime(&save_date));

    // construct final location
    char full_path[200];
    sprintf(full_path, "%s%s", savefilepath, fname);

    // set location 
    FILE *outfile;
    outfile = fopen(full_path, "wt");

    // text out buffer
    char txt_out_buf[500];
    int cycle = 0;

    // print header
    fprintf(outfile, "buffer_id, count, cardcounter, computertime\n");

    while (1) {
        
        // read the bottom
        if ((pps_in_ptr > BUFFER_LENGTH/2) && (cycle == 0)){
            
            for (int i = 0; i < BUFFER_LENGTH/2; i++) {
                
                sprintf(txt_out_buf, "%i, %d, %f, %i \n", 
                      i, pps_id[i], pps_card_time[i], pps_cpu_time[i]
                );

                fprintf(outfile, "%s", txt_out_buf);
                continue;            
            }
            cycle = 1;
        }

        // read the top
        if ((pps_in_ptr < BUFFER_LENGTH/2) && (cycle == 1)){
            for (int i = BUFFER_LENGTH/2; i < BUFFER_LENGTH; i++) {

                sprintf(txt_out_buf, "%i, %d, %f, %i \n", 
                      i, pps_id[i], pps_card_time[i], pps_cpu_time[i]
                );

                fprintf(outfile, "%s", txt_out_buf);
                continue;
            }
            cycle = 0;
        }

        sleep(3);
    } 
    return 0;
}


void *ControlThread(void *input){
    
    int sockfd;     /* socket */
    int portno;     /* port to listen on */
    int clientlen;      /* byte size of client's address */
    struct sockaddr_in serveraddr;  /* server's addr */
    struct sockaddr_in clientaddr;  /* client addr */
    struct hostent *hostp;  /* client host info */
    char *buf;      /* message buf */
    char *hostaddrp;    /* dotted decimal host addr string */
    int optval;     /* flag value for setsockopt */
    int n;          /* message byte size */

    /* 
     * check command line arguments 
     */
    portno = 8787;

    /* 
     * socket: create the parent socket 
     */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    /* setsockopt: Handy debugging trick that lets 
     * us rerun the server immediately after we kill it; 
     * otherwise we have to wait about 20 secs. 
     * Eliminates "ERROR on binding: Address already in use" error. 
     */
    optval = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));

    /*
     * build the server's Internet address
     */
    bzero((char *)&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons((unsigned short)portno);

    /* 
     * bind: associate the parent socket with a port 
     */
    if (bind(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
        error("ERROR on binding");

    /* 
     * main loop: wait for a datagram, then echo it
     */
    clientlen = sizeof(clientaddr);
    while (1) {

        /*
         * recvfrom: receive a UDP datagram from a client
         */
        buf = malloc(BUFSIZE);
        memset(buf, 0, BUFSIZE);
        n = recvfrom(sockfd, buf, BUFSIZE, 0,
                 (struct sockaddr *)&clientaddr, &clientlen);
        if (n < 0)
            error("ERROR in recvfrom");

        /* 
         * gethostbyaddr: determine who sent the datagram
         */
        hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr,
                      sizeof(clientaddr.sin_addr.s_addr),
                      AF_INET);
        if (hostp == NULL)
            error("ERROR on gethostbyaddr");
        hostaddrp = inet_ntoa(clientaddr.sin_addr);
        if (hostaddrp == NULL)
            error("ERROR on inet_ntoa\n");
        //printf("server received %d bytes\n", n);

        
        //TODO: interogate command

        printf("server received command: %s \n", buf);
        
        if (strcmp(buf, "shutdown") == 0) {
            free(buf);
            printf("Shutdown Command Received\n");  
            S826_SystemClose();
            
            char response[] = "Power Shutoff + System Closed!";
            printf("%s \n", response);
            n = sendto(sockfd, response, sizeof(response), 0, (struct sockaddr *)&clientaddr, clientlen);
            //if (n < 0) --> handle send to erro
            exit(0);    
        }

        if (strcmp(buf, "get_data") == 0) {
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


int main(int argc, char **argv){
    // signal handler for abrupt/any close
    signal(SIGINT, SystemCloseHandler);

    if( argc < 1 ) {
        printf("Use: readout \n");
        printf("     configfilename \n");
        exit(0);
    }

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

    // define threads.
    pthread_t control_thread;
    pthread_t quad_thread, pps_thread, sensor_thread;
    pthread_t quad_write_thread, pps_write_thread, sensor_write_thread; 
    
    // start reading threads
    pthread_create(&quad_thread, NULL, QuadThread, (void *)thread_args);
    pthread_create(&pps_thread, NULL, PPSThread, (void *)thread_args);
    pthread_create(&sensor_thread, NULL, SensorThread, (void *)thread_args);   

    // start the writing threads
    pthread_create(&quad_write_thread, NULL, QuadBufferToDisk, (void *)thread_args);
    pthread_create(&pps_write_thread, NULL, PPSBufferToDisk, (void *)thread_args);
    pthread_create(&sensor_write_thread, NULL, SensorBufferToDisk, (void *)thread_args);
    pthread_create(&control_thread, NULL, ControlThread, (void *)thread_args);   
     
    // join back to main.
    pthread_join(control_thread, NULL);
    pthread_join(quad_thread, NULL);
    pthread_join(pps_thread, NULL);
    pthread_join(sensor_thread, NULL);
    pthread_join(quad_write_thread, NULL);
    pthread_join(pps_write_thread, NULL);
    pthread_join(sensor_write_thread, NULL);
    
    // free malloc'ed data.
    free(thread_args);
}

/*
gcc -D_LINUX -Wall -Wextra -DOSTYPE_LINUX -c -no-pie readout.c ../vend/ini/ini.c
gcc -D_LINUX readout.o ini.o -no-pie -o readout -L ../vend/sdk_826_linux_3.3.11/demo -l826_64 -lpthread
gcc -D_LINUX -Wall -Wextra -DOSTYPE_LINUX -c -no-pie readout_serial.c ../vend/ini/ini.c
gcc -D_LINUX readout_serial.o ini.o -no-pie -o readout_serial -L ../vend/sdk_826_linux_3.3.11/demo -l826_64 -lpthread
*/