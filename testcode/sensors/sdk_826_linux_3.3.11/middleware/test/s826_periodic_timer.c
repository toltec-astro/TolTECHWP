///////////////////////////////////////////////////////////////////////////////////////////////
// SENSORAY MODEL 826 PROGRAMMING EXAMPLES
// This file contains simple functions that demonstrate how to program the 826.
// For clarity, error checking is not as robust as it might be in a real-world application.
///////////////////////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <math.h>

#include "..\826api.h"


#define PI  3.1415926535

///////////////////////////////////////////////////////
// Utility functions used by the demos.

// Configure counter channel to function as a periodic timer and start it running.
static int StartPeriodicTimer(uint board, uint counter, uint period)
{
    int errcode = S826_ERR_OK;
    if (errcode == S826_ERR_OK) errcode = S826_CounterStateWrite(board, counter, 0);                // halt timer if it's running
    if (errcode == S826_ERR_OK) errcode = S826_CounterModeWrite(board, counter, 0x00402020);        // config counter
    if (errcode == S826_ERR_OK) errcode = S826_CounterSnapshotConfigWrite(board, counter, 4, 0);    // snapshot at zero counts
    if (errcode == S826_ERR_OK) errcode = S826_CounterPreloadWrite(board, counter, 0, period);      // 1s period
    if (errcode == S826_ERR_OK) errcode = S826_CounterStateWrite(board, counter, 1);                // start timer
    return errcode;
}

/////////////////////////////////////////////////////
// Demo: Simple periodic timer.

static int DemoPeriodicTimer(uint board, uint counter)
{
    int i;
    int errcode;
    uint counts, timestamp, reason;
    uint prevstamp = 0;

    printf("\nDemoPeriodicTimer\n");

    errcode = StartPeriodicTimer(board, counter, 1000000);  // Configure counter as 1 second (1000000 microseconds) periodic timer and start it running.

    for (i = 0; i < 10; i++)    // Loop for 10 periods ...
    {
        if (errcode == S826_ERR_OK) errcode = S826_CounterSnapshotRead(board, counter, &counts, &timestamp, &reason, S826_WAIT_INFINITE);   // wait for timer snapshot
        if (errcode != S826_ERR_OK)
            break;
        else
            printf("%d %d\n", timestamp, timestamp - prevstamp);          // report current timestamp and elapsed time (in microseconds) since previous snapshot

        prevstamp = timestamp;
    }

    if (errcode == S826_ERR_OK) errcode = S826_CounterStateWrite(board, counter, 0);        // halt periodic timer

    return errcode;
}

///////////////////////////////////////////////////////////////////////
// Demo: DIO edge detection using PWM generator to generate edges.

static int DemoDioEdgeDetect(uint board)
{
    int i;
    int errcode     = S826_ERR_OK;
    uint counter    = 0;                // use counter0 for pwm generator
    uint diomask[2] = {0, 1};           // detect edges on dio0

    printf("\nDemoDioEdgeDetect\n");

    // Configure counter0 as pwm generator, route pwm signal to dio0, enable edge captures on dio0.
    if (errcode == S826_ERR_OK) errcode = S826_CounterStateWrite(board, counter, 0);                        // halt counter channel if it's running
    if (errcode == S826_ERR_OK) errcode = S826_CounterModeWrite(board, counter, 0x00682020);                // config counter as pwm generator
    if (errcode == S826_ERR_OK) errcode = S826_CounterSnapshotConfigWrite(board, counter, 0, 0);            // don't need counter snapshots -- we're just outputting pwm signal
    if (errcode == S826_ERR_OK) errcode = S826_CounterPreloadWrite(board, counter, 0, 700000);              // program pwm timing: period = 1 second (700ms off, 300ms on)
    if (errcode == S826_ERR_OK) errcode = S826_CounterPreloadWrite(board, counter, 1, 300000);
    if (errcode == S826_ERR_OK) errcode = S826_DioOutputSourceWrite(board, diomask);                        // route counter's ExtOut signal to dio0 pin
    if (errcode == S826_ERR_OK) errcode = S826_DioCapEnablesWrite(board, diomask, diomask, 0);              // enable both rising and falling edge capturing on dio0 pin
    if (errcode == S826_ERR_OK) errcode = S826_DioCapRead(board, diomask, 0, 0);                            // reset dio0 capture flag if it's already set; note that this can modify diomask
    if (errcode == S826_ERR_OK) errcode = S826_CounterStateWrite(board, counter, 1);                        // start pwm generator

    for (i = 0; i <= 20; i++)    // Loop for 20 edge events on dio0 ...
    {
        uint diostates[2];
        uint chanlist[] = {0, 1};
        if (errcode == S826_ERR_OK) errcode = S826_DioCapRead(board, chanlist, 0, S826_WAIT_INFINITE);      // wait for dio0 edge event
        if (errcode == S826_ERR_OK) errcode = S826_DioInputRead(board, diostates);                          // read dio pin states
        if (errcode != S826_ERR_OK)
            break;
        printf("%d ", diostates[1] & 1);           // report dio0 pin state
    }
    printf("\n");

    if (errcode == S826_ERR_OK) errcode = S826_CounterStateWrite(board, counter, 0);                        // halt pwm generator

    return errcode;
}

/////////////////////////////////////////////////////////////////////////////////////
// Demo: Output sine wave using DAC and periodic timer.

static int DemoSinewaveGenerator(uint board)
{
    // Configuration values
    double freq     = 50.0;         // sine frequency in Hz
    uint amplitude  = 32767;        // peak amplitude (32767=10V). Must be < 32768
    uint tsample    = 100;          // sample time in microseconds. Must be fast enough to meet nyquist
    double duration = 5.0;          // waveform duration in seconds
    uint dac        = 0;            // dac channel to output from
    uint counter    = 0;            // counter channel to use as periodic sample timer

    int errcode  = S826_ERR_OK;     // no errors so far
    uint counts;
    uint tbegin;
    uint tstamp;
    uint reason;
    double runtime;                 // runtime in microseconds
    uint dacval;                    // dac setpoint

    printf("\nDemoSinewaveGenerator\n");

    if (errcode == S826_ERR_OK) errcode = S826_DacRangeWrite(board, dac, S826_DAC_SPAN_10_10, 0);   // program dac output range: -10V to +10V
    if (errcode == S826_ERR_OK) errcode = StartPeriodicTimer(board, 0, tsample);                    // configure counter0 as periodic sample timer and start it running.

    if (errcode == S826_ERR_OK) errcode = S826_CounterSnapshotRead(board, counter, &counts, &tstamp, &reason, S826_WAIT_INFINITE);

    if (errcode == S826_ERR_OK)
    {
        tbegin = tstamp;
        do
        {
            runtime = (double)(tstamp - tbegin) / 1000000.0;
            dacval  = (uint)(32768 + amplitude * sin(2.0 * PI * freq * runtime));

            if ((errcode = S826_DacDataWrite(board, dac, dacval, 0)) != S826_ERR_OK)
                break;

            if ((errcode = S826_CounterSnapshotRead(board, counter, &counts, &tstamp, &reason, S826_WAIT_INFINITE)) != S826_ERR_OK)
                break;

        }
        while (runtime < duration);
    }

    if (errcode == S826_ERR_OK) errcode = S826_CounterStateWrite(board, counter, 0);        // halt periodic timer

    return errcode;
}

/////////////////////////////////////////////////////
// Demo: Oversample and average two analog inputs.

static int DemoAnalogOversample(uint board)
{
    int errcode  = S826_ERR_OK;
    uint counter = 0;               // use counter0 for periodic timer

    printf("\nDemoAnalogOversample\n");




    if (errcode == S826_ERR_OK) errcode = S826_CounterStateWrite(board, counter, 0);        // halt periodic timer

    return errcode;
}

/////////////////////////////////////////////////////
// Demonstrate DIO edge detect, PWM generator

static int DemoAnalogTrack(uint board)
{
    int errcode  = S826_ERR_OK;
    uint counter = 0;               // use counter0 for periodic timer

    printf("\nDemoAnalogTrack\n");



    if (errcode == S826_ERR_OK) errcode = S826_CounterStateWrite(board, counter, 0);        // halt periodic timer

    return errcode;
}



///////////////////////////////////////////////
// Main function.

int main(void)
{
    uint board      = 0;                        // change this if you want to use other than board number 0
    int errcode     = S826_ERR_OK;

    int boardflags  = S826_SystemOpen();        // open 826 driver and find all 826 boards
    if (boardflags < 0)
        errcode = boardflags;                       // problem during open
    else if ((boardflags & (1 << board)) == 0)
        printf("TARGET BOARD NOT FOUND\n");         // driver didn't find board you want to use
    else
    {
        // Run the suite of demo functions.
//        if (errcode == S826_ERR_OK) errcode = DemoPeriodicTimer(board, 0);      // simple periodic timer using counter0
//        if (errcode == S826_ERR_OK) errcode = DemoDioEdgeDetect(board);         // dio edge detection
        if (errcode == S826_ERR_OK) errcode = DemoSinewaveGenerator(board);     // analog output
        if (errcode == S826_ERR_OK) errcode = DemoAnalogOversample(board);      // oversampled analog input
        if (errcode == S826_ERR_OK) errcode = DemoAnalogTrack(board);           // 
    }

    switch (errcode)
    {
    case S826_ERR_OK:           break;
    case S826_ERR_BOARD:        printf("Illegal board number"); break;
    case S826_ERR_VALUE:        printf("Illegal argument"); break;
    case S826_ERR_NOTREADY:     printf("Device not ready or wait timeout"); break;
    case S826_ERR_CANCELLED:    printf("Wait cancelled"); break;
    case S826_ERR_DRIVER:       printf("Driver call failed"); break;
    case S826_ERR_MISSEDTRIG:   printf("Missed adc trigger"); break;
    case S826_ERR_BUFOVERFLOW:  printf("Adc data overflow"); break;
    case S826_ERR_DUPADDR:      printf("Two boards set to same board number"); break;
    case S826_ERR_BOARDCLOSED:  printf("Board not open"); break;
    case S826_ERR_CREATEMUTEX:  printf("Can't create mutex"); break;
    case S826_ERR_MEMORYMAP:    printf("Can't map board"); break;
    default:                    printf("Unknown error"); break;
    }

    printf("\nKeypress to exit ...");
    while (!_kbhit());
    _getch();

    S826_SystemClose();
}

