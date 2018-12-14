//
//

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <math.h>

#include "..\826api.h"




////////////////////////////////////////////////////////////////////////////////////////////////
// Notify if error occured.

static int ErrReport(int errcode)
{
    switch (errcode)
    {

    case S826_ERR_OK:             break;
    case S826_ERR_BOARD:          printf("Illegal board number\n"); break;
    case S826_ERR_VALUE:          printf("Illegal argument value\n"); break;
    case S826_ERR_NOTREADY:       printf("Device not ready or timeout waiting for device\n"); break;
    case S826_ERR_CANCELLED:      printf("Wait cancelled\n"); break;
    case S826_ERR_DRIVER:         printf("Driver call failed\n"); break;
    case S826_ERR_MISSEDTRIG:     printf("Missed adc trigger\n"); break;
//    case S826_ERR_BUFOVERFLOW:    printf("Adc data buffer overflowed\n"); break;
    case S826_ERR_DUPADDR:        printf("Two boards set to same board number\n"); break;
    case S826_ERR_BOARDCLOSED:    printf("Board is not open\n"); break;
    case S826_ERR_CREATEMUTEX:    printf("Can't create mutex\n"); break;
    case S826_ERR_MEMORYMAP:      printf("Can't map board to memory address\n"); break;
    default:
        if (errcode < 0)          printf("Unknown error\n"); break;
    }
    return errcode;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Diagnostic mode LEDs with binary count.

static void DiagLEDs(uint board, DWORD delay)
{
    int i;

    S826_WriteReg(board, 2, 8);     // switch LEDs to diagnostic control mode
    for (i = 0; i <= 64; i++)
    {
        S826_WriteReg(board, 2, i << 16);
        Sleep(delay);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// DIO test.

static void DioDelay(uint board, int loops)
{
    uint diodataIn[2];


    int i;
    for (i = 0; i < loops; i++)
        S826_DioInputRead(board, diodataIn);

}

static void DioTest(uint board)
{
    int i;

    uint diodataOff[] = {0, 0};
    uint diodataOn[] = {0xffff, 0xffff};

    for (i = 0; i < 100000; i++)
    {
        S826_DioOutputWrite(board, diodataOff, 0);
        DioDelay(board, 1);
        S826_DioOutputWrite(board, diodataOn, 0);
        DioDelay(board, 1);

    }


}



/////////////////////////////////////////////////////////////////////////////////////////////////////////
// RAM test.

static void RamTest(uint board)
{
    int i;
    uint ramdata[512];
    int ram_errcount = 0;

    // Test RAM.
    for (i = 0; i < 512; i++)
        S826_RamWrite(board, i, i+1);
    for (i=0; i<512; i++)
    {
        S826_RamRead(board, i, &ramdata[i]);
        if (ramdata[i] != (i+1))
            ram_errcount++;
    }
    printf("RamTest errors: %d\n", ram_errcount);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Read and display board info.

static char *SplitVersion(uint ver, char *txt)
{
    sprintf(txt, "%d.%d.%d", ver >> 24, (ver >> 16) & 0xFF, ver & 0xFFFF);
    return txt;
}

static void BoardInfo(uint board)
{
    uint errcode;
    uint api;
    uint driver;
    uint boardrev;
    uint fpgarev;
    char apiStr[30];
    char driverStr[30];

    errcode = ErrReport(S826_VersionRead(board, &api, &driver, &boardrev, &fpgarev));
    printf("ApiVer=%s, DriverVer=%s, BoardRev=%d, FpgaRev=%d\n", SplitVersion(api, apiStr), SplitVersion(driver, driverStr), boardrev, fpgarev);
}




static char GetCommand(char *str, uint *addr, uint *data)
{
    char cmd;
    char seps[]   = " ";
    int ntokens;

    char *token = strtok(str, seps);    // Get first token

    cmd = ' ';     // defaults if not specified
    *addr = 0;
    *data = 0;

    for (ntokens = 0; token != NULL; ntokens++)
    {
        if (ntokens == 0)       cmd = *token;
        else if (ntokens == 1)  *addr = strtoul(token, NULL, 0);
        else if (ntokens == 2)  *data = strtoul(token, NULL, 0);
        else                    break;

        token = strtok(NULL, seps);        // Get next token 
    }
    return cmd;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////  DAC FUNCTIONS  /////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////


#define PI  3.1415926535

static uint Sine(uint step, uint stepsPerCycle, uint amplitude)
{
    uint val = 32768 + (uint)(32767 * sin(step / (stepsPerCycle * (2 * PI))));
    if (val > 65535)    val = 65535;
    else if (val < 0)   val = 0;
    return val;
}

static uint Ramp(uint step, uint stepsPerCycle)
{
    return (step * (65536 / stepsPerCycle)) & 0x0000FFFF;
}

static void SetDacRanges(uint board, uint minChan, uint maxChan, uint range)
{
    uint chan;
    for (chan = 0; chan <= 8; chan++)
        S826_DacRangeWrite(board, chan, range, 0);
}

static void GenerateAll(uint board, uint minChan, uint maxChan, uint nSteps)
{
    uint i;
    uint chan;

    SetDacRanges(board, minChan, maxChan, S826_DAC_SPAN_10_10);
    for (i = 0; i < nSteps; i++)
        for (chan = minChan; chan <= maxChan; chan++)
            S826_DacDataWrite(board, chan, (chan & 1) ? Sine(i, 1000, 65534) : Ramp(i, 1000), 0);
}

static void GenerateSine(uint board, uint minChan, uint maxChan, uint stepsPerCycle, uint nSteps)
{
    uint i;
    uint chan;

    SetDacRanges(board, minChan, maxChan, S826_DAC_SPAN_10_10);
    for (i = 0; i < nSteps; i++)
        for (chan = minChan; chan <= maxChan; chan++)
            S826_DacDataWrite(board, chan, Sine(i, stepsPerCycle, 65534), 0);
}

static void GenerateSquare(uint board, uint minChan, uint maxChan, uint nSteps)
{
    uint i;
    uint chan;

    SetDacRanges(board, minChan, maxChan, S826_DAC_SPAN_10_10);
    for (i = 0; i < nSteps; i++)
        for (chan = minChan; chan <= maxChan; chan++)
            S826_DacDataWrite(board, chan, (i & 1) * 65535, 0);
}

static void GenerateRamp(uint board, uint minChan, uint maxChan, uint nSteps)
{
    uint i;
    uint chan;

    SetDacRanges(board, minChan, maxChan, S826_DAC_SPAN_10_10);
    for (i = 0; i < nSteps; i++)
        for (chan = minChan; chan <= maxChan; chan++)
            S826_DacDataWrite(board, chan, i & 0x0000FFFF, 0);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////  ADC TEST  /////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////


static adctest(uint board)
{
    uint slot, chan, slotlist;

    uint scan[16];
    uint tstamp[16];

    int errcode;

    uint start, prev, end, deltaT;
    uint longest = 0;
    uint shortest = ~0;

    uint nSamples = 0;
    uint tsettle = 12;   // 12;


    // Configure analog input channels.
    for (slot = 0, chan=0; slot < 16; slot++, chan++)
        S826_AdcSlotConfigWrite(board, slot, chan, tsettle, S826_ADC_GAIN_1);

    S826_AdcSlotlistWrite(board, 0xFFFF, 0);        // Enable all analog input channels.
    S826_AdcTrigModeWrite(board, 0xF0);             // Trigger adc burst upon counter0 output pulse.
    S826_AdcEnableWrite(board, 1);                  // Enable adc.

    // Configure counter0 to output a pulse every 1000 microseconds.
    S826_CounterPreloadWrite(board, 0, 0, 1001);
    S826_CounterModeWrite(board, 0, 0x01502020);
    S826_CounterStateWrite(board, 0, 1);

    S826_TimestampRead(board, &start);
    prev = start;

    for (nSamples = 0; nSamples < 1000; nSamples++)
    {
        slotlist = 0xFFFF;                                              // Indicate adc slots of interest.

        errcode = S826_AdcRead(board, scan, tstamp, &slotlist, 2000);   // Blocking adc read.

        S826_TimestampRead(board, &end);

        if ((errcode == S826_ERR_OK) || (errcode == S826_ERR_MISSEDTRIG))
        {
            deltaT = end - prev;
            prev = end;

            if (deltaT > longest)
                longest = deltaT;

            if ((deltaT < shortest) && (nSamples != 0))
                shortest = deltaT;
        }
        else
        {
            printf("Error = %d\n", errcode);
            break;
        }
    }

    S826_AdcEnableWrite(board, 0);                  // Disable adc.
    S826_CounterStateWrite(board, 0, 0);            // Turn off counter0 output pulses.
    S826_CounterModeWrite(board, 0, 0);             // Turn off counter0 clock (stop LED flashing).



    printf("A/D %i sample intervals (uS), Max=%d  Min=%d  Avg=%d\n", nSamples, longest, shortest, (end - start)/nSamples);
}





////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////  TOP-LEVEL  ///////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////


static uint LcsRegRead(uint board, uint addr)
{
    S826_WriteReg(board, 0xA01, addr);
    return S826_ReadReg(board, 0xA03);
}

static void LcsRegWrite(uint board, uint addr, uint data)
{
    S826_WriteReg(board, 0xA02, data);
    S826_WriteReg(board, 0xA01, addr | 1);
}

static void MenuSystem(uint board)
{
    char str[100];
    char cmd;
    uint addr;
    uint data;
    int  errcode;

    printf("\nCommands: quit - q, read - r addr, write - w addr data\n");

    do
    {
        printf("cmd>");

        gets(str);

        cmd = GetCommand(str, &addr, &data);

        switch (cmd)
        {
        case 'r':
            printf("0x%08X\n", S826_ReadReg(board, addr));
            break;

        case 'w':
            S826_WriteReg(board, addr, data);
            break;


        case 'a':
            errcode = S826_WriteBridge(board, 1, 0x6C, data, 0, 0x1943);  // write LCS_CNTRL
            break;

        case 'b':
            errcode = S826_ReadBridge(board, 1, 0x6C, &data, 0, 0x1943);  // read LCS_CNTRL
            if (errcode != 0)
                printf("error\n");
            else
                printf("0x%08X\n", data);
            break;

        case 's':   // set LCS reg bits
            LcsRegWrite(board, addr, LcsRegRead(board, addr) | data);
            break;

        case 'c':   // clear LCS reg bits
            LcsRegWrite(board, addr, LcsRegRead(board, addr) & ~data);
            break;

        case 'p':   // put (write) LCS reg
            LcsRegWrite(board, addr, data);
            break;

        case 'g':   // get (read) LCS reg
            printf("0x%08X\n", LcsRegRead(board, addr));
            break;



        case 'z':

            adctest(board);
            break;



/*
        case 'e': // write val to eeprom
            S826_EepromWriteQuadlet(board, 0, 0x12345678);
            break;

        case 'f': // read val from eeprom
            S826_EepromReadQuadlet(board, 0, &data);
            printf("0x%08X\n", data);
            break;

        case 'd':
            printf("digitize ...");
            S826_AdcSlotConfigWrite(board, 0, 0, 1000, S826_ADC_GAIN_1);     // config slot 0 to measure ext chan 0, 10V bipolar range, 1ms settling time
            S826_AdcSlotConfigWrite(board, 1, 1, 1000, S826_ADC_GAIN_1);     // config slot 0 to measure ext chan 0, 10V bipolar range, 1ms settling time
            S826_AdcSlotlistWrite(board, 0x00000003, 0);                     // establish convert list, set to continuous (free-run) mode
            S826_AdcEnableWrite(board, 1);                                   // start adc running

            S826_WriteReg(board, 0x00, 9);              // access adc data bank

            printf("\n");
            break;



        case 'b':
            S826_CounterPreloadRead(board, 0, 0, &data);
            printf("0x%08X\n",data);

            S826_CounterPreloadRead(board, 0, 1, &data);
            printf("0x%08X\n",data);


            break;


        case 'm':
            printf("enable adc trigger ...");
            S826_AdcTrigModeWrite(board, 0x82);
            printf("\n");
            break;

        case 'n':
            printf("disable adc trigger ...");
            S826_AdcTrigModeWrite(board, 0x43);
            printf("\n");
            break;


        case 'a':
            printf("generating all waves ...");
            GenerateAll(board, 0, 7, 15 * 65536);
            printf("\n");
            break;
        case 's':
            printf("generating sine wave ...");
            GenerateSine(board, 1, 1, 10, 500000);
            printf("\n");
            break;
        case 't':
            printf("generating square wave ...");
            GenerateSquare(board, 1, 1, 20000);
            printf("\n");
            break;
        case 'u':
            printf("generating ramp ...");
            GenerateRamp(board, 0, 7, 8 * 65536);
            printf("\n");
            break;
*/

        }
    }
    while (cmd != 'q');
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Main I/O process.

int IoProc(uint board)
{



    BoardInfo(board);           // Get board version info.











    MenuSystem(board);



    return 0;

    DiagLEDs(board, 20);        // Switch leds to diagnostic mode and count up on leds in binary

    RamTest(board);             // Test RAM.

//    DioTest(board);


    {
        uint chanlist[] = {0xffff, 0xffff};

        // test dio capture
        S826_DioCapRead(board, chanlist, 0, 10);

        return 0;
        
    }




//    errcode = S826_CounterSetPreload(board, 0, 0, 55);




    return 0;

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Main function.

int main(void)
{
    int boardflags = S826_SystemOpen();

    if (boardflags >= 0)
    {
        if (boardflags == 0)
            printf ("NO BOARDS FOUND\n");
        else
        {
            uint board;
            for (board = 0; board < 16; board++)            // Find board with lowest board number and use it.
            {
                if (boardflags & (1 << board))
                    break;
            }
            IoProc(board);
        }
        S826_SystemClose();
    }

    printf("Keypress to exit ...");
    while (!_kbhit());
    _getch();

	return boardflags;

}

