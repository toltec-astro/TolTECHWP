////////////////////////////////////////////////////////////////////////////////////////
// File      : 826lib.c
// Function  : OS independent code for 826 middleware API
//             Everything in this file must be OS independent.
// Copyright : (C) 2012 Sensoray
////////////////////////////////////////////////////////////////////////////////////////

#include "platform.h"
#include "826api.h"
#include "826drv.h"

// VERSION INFO ///////////////////////////////////////////////////////////////////


// ********** UPDATE EVERY RELEASE ***********
#include "version.h"
// *******************************************



// Constants /////////////////////////////////////////////////////////////////////////////////////////////////////

#define TRUE                    1
#define FALSE                   0

#define VACANT_BOARDINDEX       (~0)                // Indicates no board present at the specified index

#define GA_BASEREV_FLATADDR     64                  // Base revision for large (XC3S700) fpga

// Direct address space ----------------------------------

// Miscellaneous
#define S826_ADR_INDEX          0x0000              // RW index / ram page / board signature
#define S826_ADR_TSTAMP         0x0001              // R- current timestamp
#define S826_ADR_COMCMD         0x0002              // -W common command
#define S826_ADR_CONFIG         0x0002              // R- config
//      S826_ADR_INTEN          0x0003              // interrupt enable *** controlled by driver
#define S826_ADR_INTSTS         0x0004              // interrupt status
#define S826_ADR_EEPROM         0x0005              // eeprom interface
#define S826_ADR_SAFEN          0x0006              // write-enable control for safemode-related registers
#define S826_ADR_REV            0x0007              // Fpga major/minor version numbers (added in fpga v.0.0.69)

// GPIO
#define S826_ADR_GPSAFENL       0x0008              // DO safemode enable 0-23
#define S826_ADR_GPSAFENH       0x0009              // DO safemode enable 24-47
#define S826_ADR_GPOL           0x000A              // DOUT 0-23
#define S826_ADR_GPOH           0x000B              // DOUT 24-47
#define S826_ADR_GPIL           0x000C              // DIN 0-23
#define S826_ADR_GPIH           0x000D              // DIN 24-47
#define S826_ADR_GPOSRCL	    0x000E	            // DO source select 0-23
#define S826_ADR_GPOSRCH	    0x000F	            // DO source select 24-47
#define S826_ADR_GPRISEL	    0x0010	            // DIN capture rising edge 0-23
#define S826_ADR_GPRISEH	    0x0011	            // DIN capture rising edge 24-47
#define S826_ADR_GPFALLL	    0x0012	            // DIN capture falling edge 0-23
#define S826_ADR_GPFALLH	    0x0013	            // DIN capture falling edge 24-47
#define S826_ADR_GPSAFEL        0x0014              // DIO safemode output data 0-23
#define S826_ADR_GPSAFEH        0x0015              // DIO safemode output data 24-47
#define S826_ADR_GPSTSL         0x0016              // DIN capture polarity/status 0-23
#define S826_ADR_GPSTSH         0x0017              // DIN capture polarity/status 24-47
//      S826_ADR_CAPIENL        0x0018              // DIN capture interrupt enables 0-23 *** controlled by driver
//      S826_ADR_CAPIENH        0x0019              // DIN capture interrupt enables 24-47 *** controlled by driver
#define S826_ADR_DIOFILTL       0x02A0              // DIN filter enable 0-23
#define S826_ADR_DIOFILTH       0x02A1              // DIN filter enable 24-47
#define S826_ADR_DIOFILTIM      0x02A2              // DIN filter interval, common to all DINs (20ns increments)

// ADC
#define S826_ADR_ADSTS         0x001A               // ADC status
#define S826_ADR_ADCTL         0x001B               // -W ADC control/overflow reset
#define S826_ADR_ADOVR         0x001B               // R- ADC overflow status
//      S826_REG_ADINTEN       0x001C               // ADC channel interrupt enables *** controlled by driver
#define S826_ADR_ADEN          0x001D               // ADC channel scan enables

// Spares
//      S826_ADR_RESERVED       0x001E              //  reserved for future use
//      S826_ADR_RESERVED       0x001F              //  reserved for future use

// Base addresses
#define S826_ADR_INDIRECT       0x0020              // Indirect (banked) register window
#define S826_ADR_CTR(CHAN)      (0x0040 + (CHAN) * 0x0020)   // Counters
#define S826_ADR_DAC            0x0200              // Dac; note: data/config values are mixed, so use S826_OS_DACDATA/S826_OS_DACCFG macros to address
#define S826_ADR_ADCDATA        0x0240              // Adc data
#define S826_ADR_ADCCFG         0x0260              // Adc config
#define S826_ADR_ADCTSTAMP      0x0280              // Adc timestamps
#define S826_ADR_ROUTER         0x0300              // Signal routing matrix
#define S826_ADR_WDOG           0x0320              // Watchdog
#define S826_ADR_BGPIO          0x0340              // Variables (buried gpio)
#define S826_ADR_RAM            0x0400              // General-purpose ram
#define S826_ADR_EXTERN         0x0800              // Factory test

// Bank selects (legacy) ------------------------------------

#define S826_BANK_CTR(CHAN)     (CHAN)              // Counters
#define S826_BANK_DAC           0x08                // DAC
#define S826_BANK_ADCDATA       0x09                // ADC data
#define S826_BANK_ADCCFG        0x0A                // ADC configuration
#define S826_BANK_ROUTER        0x0B                // Signal router
#define S826_BANK_RAM           0x0C                // General-purpose ram
#define S826_BANK_ADCTSTAMP     0x0D                // ADC timestamps
#define S826_BANK_WDOG          0x0E                // Watchdog

// Register address offsets from peripheral base address --------------------

// Signal router.
#define S826_OS_ROUTECEN(CHAN)  (CHAN)              // CounterEnable routing control
#define S826_OS_ROUTEADCT       0x8                 // ADC trigger routing control

// DAC.
#define S826_OS_DACCFG(CHAN)    (2 * (CHAN) + 0)    // DAC configuration
#define S826_OS_DACDATA(CHAN)   (2 * (CHAN) + 1)    // DAC data

// Counters.
#define S826_OS_COUNTS          0x0000              // snapshot counts
#define S826_OS_CTRTSTAMP       0x0001              // snapshot timestamp
#define S826_OS_PRELOAD0        0x0002              // preload0 register
#define S826_OS_PRELOAD1        0x0003              // preload1 register
#define S826_OS_CMP0            0x0004              // compare0 register
#define S826_OS_CMP1            0x0005              // compare1 register
#define S826_OS_SSCTL           0x0006              // snapshot control
#define S826_OS_QCOUNTS         0x0007              // instantaneous counts
#define S826_OS_CTRMODE         0x0009              // mode control register
#define S826_OS_CTRCMD          0x000A              // -W counter command
#define S826_OS_CTRSTS          0x000A              // R- counter status register
#define S826_OS_SSLEVEL         0x000B              // R- snapshot fifo level
#define S826_OS_CTRFILT         0x000C              // debounce filter control register

// Watchdog.
#define S826_OS_WDDELAY0        0x0000              // stage0 interval (@50MHz)
#define S826_OS_WDDELAY1        0x0001              // stage1 interval (@50MHz)
#define S826_OS_WDDELAY2        0x0002              // stage2 interval (@50MHz)
#define S826_OS_WDPWIDTH        0x0003              // ResetOut pulse width (@50MHz)
#define S826_OS_WDPGAP          0x0004              // ResetOut gap (@50MHz)
#define S826_OS_WDCFG           0x0008              // configuration: EN, SEN, NIE, PEN, OP, OEN
#define S826_OS_WDCTRL          0x0009              // status (rd) / kick (wr)

// Other constants =======================================================================

// Common port masks for bit write/set/clear.
#define PORT_BITSET             0x80000000          // Set all port bits that are specified with '1'
#define PORT_BITCLR             0x40000000          // Clear all port bits that are specified with '1'
#define PORT_WRITE              0x00000000          // Unconditionally write all port bits

// Private bit masks for COMCMD
#define S826_CONFIG_RST         0x00000001          // SafeMode active
#define S826_CONFIG_LM          0x00000004          // SafeMode active
#define S826_CONFIG_LEDMASK     0x0003F000          // SafeMode active
// User-accessible bits:
#define COMCMD_MASK             (S826_CONFIG_LEDMASK | S826_CONFIG_XSF | S826_CONFIG_LM | S826_CONFIG_SAF | S826_CONFIG_RST)

// Compute array item count
#define LISTSIZE(L)     (sizeof(L)/sizeof(L[0]))


// Static storage ========================================================================

static int  bdindex[S826_MAX_BOARDS];           // Map board number to index number (bdindex[board] == index). Set to VACANT_BOARDINDEX if board is unregistered.
static int  flataddr[S826_MAX_BOARDS];          // Booleans. When true, use flat register address space (vs. banked) for associated board index.
static uint eeprom_fault[S826_MAX_BOARDS];      // Detected eeprom faults (missing/faulty device) for each board index.


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////  EEPROM FUNCTIONS (NOT THREAD SAFE!)  ///////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// EEPROM Memory Map:
// EEADDR   RANGE   VALUE
// ------   -----   ------
// 00-03    10V     offset ADC cal constants ...
// 04-07    10V     scalar
// 08-0B    5V      offset
// 0C-0F    5V      scalar
// 10-13    2V      offset
// 14-17    2V      scalar
// 18-1B    1V      offset
// 1C-1F    1V      scalar
// 20-23    uni5V   scalar DAC cal constants ...
// 24-27    uni10V  scalar
// 28-2B    bip5V   scalar
// 2C-2F    bip10V  scalar
// 30       --      set to 0xA5 when adc cal constants have been programmed into eeprom
// 31       --      set to 0xA5 when dac cal constants have been programmed into eeprom

#define S826_EEADR_ADCDATA  0x00
#define S826_EEADR_DACDATA  0x20
#define S826_EEADR_ADCVALID 0x30
#define S826_EEADR_DACVALID 0x31

#define S826_EEDAT_CALVALID 0xA5

#define CAL_FIXPT_1R0       ((uint)0x80000000)          // Default fixed-point, unsigned calibration scalar as stored in eeprom (0x80000000 = +1.0).


// EEPROM commands
#define EECMD_READ          0x00030000
#define EECMD_WRITE         0x00020000
#define EECMD_WRDI          0x00040000
#define EECMD_WREN          0x00060000
#define EECMD_RDSR          0x00050000
#define EECMD_WRSR          0x00010000

// EEPROM status register bit masks
#define EESTATUS_BP1        0x00000008          // Block Protection. 0=none, 1=upper 1/4, 2=upper 1/2, 3=all.
#define EESTATUS_BP0        0x00000004
#define EESTATUS_WEL        0x00000002          // Write Enable Latch. '1' when eeprom is write-enabled.
#define EESTATUS_WIP        0x00000001          // Write In Progress. '1' when eeprom internal write operation in progress.

// SPI busy/idle status
#define SPI_BUSY            0x80000000

#define EECTRL_CS_NEGATE    0x00000000
#define EECTRL_CS_ASSERT    0x40000000
#define EECTRL_TRANSCEIVE   0x80000000


///////////////////////////////////////////////////////////////////////////////////////
// SPI/EEPROM helper functions. EEPROM LOCK (#) MUST BE SET BEFORE CALLING.

// Poll until spi is idle.
static void SpiWaitIdle_EEL(uint index)
{
    do {} while (sys826_readreg(index, S826_ADR_EEPROM) & SPI_BUSY);
}

// Shift value out spi and simultaneously receive value from spi.
// This doesn't modify eeprom chip select.
static uint SpiTransact_EEL(uint index, uint bitcnt, uint txval)
{
    uint rxval;
    sys826_writereg(index, S826_ADR_EEPROM, EECTRL_TRANSCEIVE | (bitcnt << 24) | txval);    //# shift out cmd
    do
    {
        rxval = sys826_readreg(index, S826_ADR_EEPROM);    //# wait for completion
    }
    while (rxval & SPI_BUSY);
    return rxval & 0xFF;                                   //# return received byte val
}

// Assert eeprom chip select.
static void EepromCsAssert_EEL(uint index)
{
    sys826_writereg(index, S826_ADR_EEPROM, EECTRL_CS_ASSERT);      // assert cs; no need to wait for completion
}

// Negate eeprom chip select. Returns received eeprom byte, if there is one.
static void EepromCsNegate_EEL(uint index)
{
    sys826_writereg(index, S826_ADR_EEPROM, EECTRL_CS_NEGATE);      //# negate cs
        SpiWaitIdle_EEL(index);                                     //# wait for completion
}

// Issue pre-formatted value to eeprom device and simultaneously receive value from eeprom.
static uint EepromCmd_EEL(uint index, uint bitcnt, uint cmd)
{
    uint rxval;
    EepromCsAssert_EEL(index);                          //# assert eeprom cs
    rxval = SpiTransact_EEL(index, bitcnt, cmd);        //# shift out cmd
    EepromCsNegate_EEL(index);                          //# negate cs
    return rxval;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// Basic eeprom commands. EEPROM LOCK (#) MUST BE SET BEFORE CALLING.

#define EEADR(addr)        ( (((addr) & 0x100) << 3) | (((addr) & 0xFF) << 8) )     // high eeprom address bit embedded in eeprom command opcode

static uint EepromCmdREAD_EEL(uint index, uint addr)
{
    return EepromCmd_EEL(index, 24, EECMD_READ | EEADR(addr));
}

static uint EepromCmdRDSR_EEL(uint index)
{
    return EepromCmd_EEL(index, 16, EECMD_RDSR);
}

static void EepromCmdWRITE_byte_EEL(uint index, uint addr, uint data)
{
    EepromCmd_EEL(index, 24, EECMD_WRITE | EEADR(addr) | data);
}

static void EepromCmdWRITE_quadlet_EEL(uint index, uint addr, uint data)
{
    EepromCsAssert_EEL(index);
    SpiTransact_EEL(index, 24, EECMD_WRITE | EEADR(addr) | (data >> 24));  //# big endian, so MSB first
    SpiTransact_EEL(index, 24, data & 0x00FFFFFF);
    EepromCsNegate_EEL(index);
}

static void EepromCmdWREN_EEL(uint index)
{
    EepromCmd_EEL(index, 8, EECMD_WREN);
}
#if 0
static void EepromCmdWRDI_EEL(uint index)
{
    EepromCmd_EEL(index, 8, EECMD_WRDI);
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////
// Detect eeprom presence.
// This should be called before eeprom writes to prevent hangs due to missing/faulty eeproms.
// NOT THREAD SAFE. EEPROM LOCK (#) MUST BE SET BEFORE CALLING.

static int EepromDetect_EEL(uint index)
{
    return ((EepromCmdRDSR_EEL(index) & (EESTATUS_WEL | EESTATUS_WIP)) == 0);  //# Return TRUE if verified enabled
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Internal byte and quadlet read/write functions. Quadlets are stored in eeprom as big endian.

// Write byte to eeprom. EEPROM LOCK (#) MUST BE SET BEFORE CALLING.
static void EepromByteWrite_EEL(uint index, uint addr, uint data)
{
    EepromCmdWREN_EEL(index);                                   //# enable eeprom writes
    EepromCmdWRITE_byte_EEL(index, addr, data);                 //# write byte to eeprom
    do {} while (EepromCmdRDSR_EEL(index) & EESTATUS_WIP);      //# wait for internal eeprom write to complete
}

// Write quadlet to eeprom. EEPROM LOCK (#) MUST BE SET BEFORE CALLING.
static void EepromQuadletWrite_EEL(uint index, uint addr, uint data)
{
    EepromCmdWREN_EEL(index);                                   //# enable eeprom writes
    EepromCmdWRITE_quadlet_EEL(index, addr, data);              //# write quadlet to eeprom
    do {} while (EepromCmdRDSR_EEL(index) & EESTATUS_WIP);      //# wait for internal eeprom write to complete
}

// Read quadlet from eeprom. EEPROM LOCK (#) MUST BE SET BEFORE CALLING.
static uint EepromQuadletRead_EEL(uint index, uint addr)
{
    int i;
    uint data = 0;
    for (i = 0; i < 4; i++)                                     //# For each of quadlet's 4 bytes (starting with MSB):
        data = (data << 8) | EepromCmdREAD_EEL(index, addr++);  //#   read and gather eeprom data byte
    return data;
}

////////////////////////////////////////////////////////////////////////////////////////
// Write to eeprom. These are thread safe functions.

S826_API int S826_CC S826_EepromWriteByte(uint board, uint addr, uint data)
{
    uint index;
    int errcode;
    if (board >= S826_MAX_BOARDS)                                       // Error if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)            // Map board to index number, error if board is closed.
        return S826_ERR_BOARDCLOSED;
    if ((addr > 511) || (data > 255))                                   // Abort if illegal eeprom data or byte address.
        return S826_ERR_VALUE;
    if (eeprom_fault[index])                                            // Abort if eeprom fault detected.
        return S826_ERR_NOTREADY;

    if ((errcode = sys826_lock(index, MUTEX_EEPROM)) == S826_ERR_OK)    //# Lock eeprom.
    {
        EepromByteWrite_EEL(index, addr, data);                         //# Write byte to eeprom.
        sys826_unlock(index, MUTEX_EEPROM);                             //# Unlock eeprom.
    }
    return errcode;
}

S826_API int S826_CC S826_EepromWriteQuadlet(uint board, uint addr, uint data)
{
    uint index;
    int errcode;
    if (board >= S826_MAX_BOARDS)                                       // Error if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)            // Map board to index number, error if board is closed.
        return S826_ERR_BOARDCLOSED;
    if ((addr > 507) || ((addr & 3) != 0))                              // Abort if illegal eeprom quadlet address.
        return S826_ERR_VALUE;
    if (eeprom_fault[index])                                            // Abort if eeprom fault detected.
        return S826_ERR_NOTREADY;

    if ((errcode = sys826_lock(index, MUTEX_EEPROM)) == S826_ERR_OK)    //# Lock board to ensure thread safety.
    {
        EepromQuadletWrite_EEL(index, addr, data);                      //# Write quadlet to eeprom.
        sys826_unlock(index, MUTEX_EEPROM);                             //# Unlock board.
    }
    return errcode;
}

//////////////////////////////////////////////////////////////////////////////////////////////////
// Read from eeprom. These are thread safe functions.

S826_API int S826_CC S826_EepromReadByte(uint board, uint addr, uint *data)
{
    uint index;
    int errcode;
    if (board >= S826_MAX_BOARDS)                                       // Error if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)            // Map board to index number, error if board is closed.
        return S826_ERR_BOARDCLOSED;
    if (addr > 511)                                                     // Abort if illegal eeprom data or byte address.
        return S826_ERR_VALUE;
    if (eeprom_fault[index])                                            // Abort if eeprom fault detected.
        return S826_ERR_NOTREADY;

    if ((errcode = sys826_lock(index, MUTEX_EEPROM)) == S826_ERR_OK)    //# Lock board to ensure thread safety.
    {
        *data = EepromCmdREAD_EEL(index, addr);                         //# Read byte from eeprom.
        sys826_unlock(index, MUTEX_EEPROM);                             //# Unlock board.
    }
    return errcode;
}

S826_API int S826_CC S826_EepromReadQuadlet(uint board, uint addr, uint *data)
{
    uint index;
    int errcode;
    if (board >= S826_MAX_BOARDS)                                       // Error if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)            // Map board to index number, error if board is closed.
        return S826_ERR_BOARDCLOSED;
    if ((addr > 507) || ((addr & 3) != 0))                              // Abort if illegal eeprom quadlet address.
        return S826_ERR_VALUE;
    if (eeprom_fault[index])                                            // Abort if eeprom fault detected.
        return S826_ERR_NOTREADY;

    if ((errcode = sys826_lock(index, MUTEX_EEPROM)) == S826_ERR_OK)    //# Lock board to ensure thread safety.
    {
        *data = EepromQuadletRead_EEL(index, addr);                     //# Read quadlet from eeprom.
        sys826_unlock(index, MUTEX_EEPROM);                             //# Unlock board.
    }
    return errcode;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////  RAM FUNCTIONS  //////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// RAM capacity in quadlets.
#define RAM_NUMQUADLETS_FLAT    512     // Flat address space has full access (512 quadlets).
#define RAM_NUMQUADLETS_BANKED   32     // Banked access limited to first 32 quadlets.


////////////////////////////////////////////////////////////////////////
// Write/read quadlet to onboard ram.
// Returns error code.

S826_API int S826_CC S826_RamWrite(uint board, uint addr, uint data)
{
    uint index;
    int errcode;
    if (board >= S826_MAX_BOARDS)                                   // Error if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)        // Map board to index number, error if board is closed.
        return S826_ERR_BOARDCLOSED;
    
    if (flataddr[index])                                            // FLAT
    {
        if (addr >= RAM_NUMQUADLETS_FLAT)
            return S826_ERR_VALUE;
        sys826_writereg(index, S826_ADR_RAM + addr, data);
        return S826_ERR_OK;
    }

    if (addr >= RAM_NUMQUADLETS_BANKED)                             // BANKED
        return S826_ERR_VALUE;
    if ((errcode = sys826_lock(index, MUTEX_BANK)) == S826_ERR_OK)  //* Lock board.
    {
        sys826_writereg(index, S826_ADR_INDEX, S826_BANK_RAM);      //* Select ram bank.
        sys826_writereg(index, S826_ADR_INDIRECT + addr, data);     //* Write ram.
        sys826_unlock(index, MUTEX_BANK);                           //* Unlock board
    }
    return errcode;
}

S826_API int S826_CC S826_RamRead(uint board, uint addr, uint *data)
{
    uint index;
    int errcode;
    if (board >= S826_MAX_BOARDS)                                   // Error if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)        // Map board to index number, error if board is closed.
        return S826_ERR_BOARDCLOSED;

    if (flataddr[index])                                            // FLAT
    {
        if (addr >= RAM_NUMQUADLETS_FLAT)
            return S826_ERR_VALUE;
        *data = sys826_readreg(index, S826_ADR_RAM + addr);
        return S826_ERR_OK;
    }

    if (addr >= RAM_NUMQUADLETS_BANKED)                             // BANKED
        return S826_ERR_VALUE;
    if ((errcode = sys826_lock(index, MUTEX_BANK)) == S826_ERR_OK)  //* Lock board.
    {
        sys826_writereg(index, S826_ADR_INDEX, S826_BANK_RAM);      //* Select ram bank.
        *data = sys826_readreg(index, S826_ADR_INDIRECT + addr);    //* Read ram.
        sys826_unlock(index, MUTEX_BANK);                           //* Unlock board.
    }
    return errcode;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////  DAC FUNCTIONS  //////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// Manifest constants.
#define DAC_NUM_RANGES          4               // Number of physical output ranges.
#define DAC_RANGE_BIPOLAR       0x00000002      // Output range is bipolar when this range bit is set.
#define DAC_BIPOLAR_ZERO        0x8000

// Device opcodes.
#define DAC_CMD_SPAN            0x200000        // Set span, data is a span code. Span won't become active until DAC_CMD_SETOUT is executed for the addressed channel.
#define DAC_CMD_SETOUT          0x700000        // Set output, data is output value. New span will be activated if DAC_CMD_SPAN was previously executed.

// Command word field masks.
#define DAC_MASK_SETPOINT       0x0000FFFF      // Setpoint mask.
#define DAC_MASK_RANGE          0x00000003      // Range mask.
#define DAC_MASK_BUSY           0x80000000      // Busy flag. Indicates command pending serial transmission to dac device.

// Command word field positions (bit numbers).
#define DAC_SHIFT_CHAN          17              // Device's channel number.
#define DAC_SHIFT_GUARDBITS     25              // Setpoint data guard bits.

// Setpoint guard-bit
#define DAC_NUM_GUARDBITS       2               // Number of setpoint guard bits stored in dual-port dacram.

// Derived constants
#define DAC_GUARDBIT_SCALAR             (1 << DAC_NUM_GUARDBITS)
#define DAC_GUARDBIT_MASK               (DAC_GUARDBIT_SCALAR - 1)
#define DAC_BIPOLAR_ZERO_FIXEDPOINT     (DAC_BIPOLAR_ZERO << DAC_NUM_GUARDBITS)


// Calibration correction scalars, one per dac output range. Copied from eeprom at app startup.
// After writing new constants to eeprom, the dac must be rewritten to activate new constants.
static double DacRangeCal[S826_MAX_BOARDS][DAC_NUM_RANGES];


/////////////////////////////////////////////////////////////////////////////////
// Helper functions

// Construct a native dac WriteRange command.
// Imports:
//  chan   - dac channel number in range 0:7
//  range  - range code: S826_DAC_SPAN_0_5 | S826_DAC_SPAN_0_10 | S826_DAC_SPAN_5_5 | S826_DAC_SPAN_10_10

static uint DacCmdSetRange(uint chan, uint range)
{
    return  ((chan & 4) << 22)                      // device select
            | DAC_CMD_SPAN                          // device command
            | ((chan & 3) << DAC_SHIFT_CHAN)        // channel number within device
            | (range & DAC_MASK_RANGE);             // range code
}

// Construct a native dac WriteSetpoint command.
// Imports:
//  chan   - dac channel number in range 0:7
//  data18 - fixed-point 18-bit setpoint expressed as 16-bit integer with two-bit fraction

static uint DacCmdSetOutput(uint chan, uint data18)
{
    return  ((chan & 4) << 22)                                              // device select
            | DAC_CMD_SETOUT                                                // device command
            | ((chan & 3) << DAC_SHIFT_CHAN)                                // channel number within device
            | ((data18 >> DAC_NUM_GUARDBITS) & DAC_MASK_SETPOINT)          // data to write, integer part
            | ((data18 & DAC_GUARDBIT_MASK) << DAC_SHIFT_GUARDBITS);    // data to write, fractional part, right justified at DAC_SHIFT_GUARDBITS
}

// Convert user setpoint to native dac value.
// Imports:
//  setpoint - unipolar ranges: 0x0000-0xFFFF = 0 to +Vmax, bipolar ranges: 0x0000-0xFFFF = -Vmax to +Vmax.

static uint DacUserToRaw(uint index, uint setpoint, uint range)
{
    double cal = DacRangeCal[index][range] * DAC_GUARDBIT_SCALAR;           // get cal scalar for specified output range

    return (range & DAC_RANGE_BIPOLAR) ?                                                            // convert user setpoint to raw:
        (uint)(int)((int16)(setpoint - DAC_BIPOLAR_ZERO) * cal + DAC_BIPOLAR_ZERO_FIXEDPOINT) : //   if bipolar range
        (uint)(setpoint * cal);                                                                   //   if unipolar range
}

// Convert current dac setpoint to user value.
// Imports:
//  raw - raw command read from dacram; we are only interested in fixed-point raw setpoint value stored in data field and guard bits

static uint DacRawToUser(uint index, uint raw, uint range)
{
    double cal = DacRangeCal[index][range] * DAC_GUARDBIT_SCALAR;       // get cal scalar for specified output range

    uint raw18 = ((raw & DAC_MASK_SETPOINT) << DAC_NUM_GUARDBITS)       // build fixed-point raw dac data by combining integer part
        | ((raw >> DAC_SHIFT_GUARDBITS) & DAC_GUARDBIT_MASK);           //   and fractional part (guard bits)

    return (range & DAC_RANGE_BIPOLAR) ?                                                        // convert raw setpoint to calibration-corrected:
        (uint)(((int)raw18 - DAC_BIPOLAR_ZERO_FIXEDPOINT) / cal + DAC_BIPOLAR_ZERO + 0.5) :     //   if bipolar range
        (uint)(raw18 / cal + 0.5);                                                              //   if unipolar range
}

/////////////////////////////////////////////////////////////////////////////
// Program DAC output range and set output to 0 volts.


static void DacRangeWrite(uint index, uint baseaddr, uint chan, uint range)
{
    static const uint DacZero[] = {0, 0, DAC_BIPOLAR_ZERO_FIXEDPOINT, DAC_BIPOLAR_ZERO_FIXEDPOINT};     // 0V setpoint values for each DAC output range.

    sys826_writereg(index, baseaddr + S826_OS_DACCFG(chan),  DacCmdSetRange(chan, range));              // Write SetRange w/new range to dac span reg
    sys826_writereg(index, baseaddr + S826_OS_DACDATA(chan), DacCmdSetOutput(chan, DacZero[range]));    // Write SetDataAndUpdate w/0V setpoint to dac data reg
}

S826_API int S826_CC S826_DacRangeWrite(uint board, uint chan, uint range, uint safemode)
{
    uint index;
    int errcode;
    if (board >= S826_MAX_BOARDS)                                   // Error if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)        // Map board to index number, error if board is closed.
        return S826_ERR_BOARDCLOSED;
    if ((chan >= S826_NUM_DAC) || (range > DAC_MASK_RANGE))         // Abort if bad argument.
        return S826_ERR_VALUE;

    if (safemode)                                                   // If programming safemode output range
        chan += S826_NUM_DAC;                                       //   adjust base register address offset to access safemode values.

    if (flataddr[index])                                            // FLAT
    {
        DacRangeWrite(index, S826_ADR_DAC, chan, range);            // Program output range and 0V setpoint.
        return S826_ERR_OK;
    }

    if ((errcode = sys826_lock(index, MUTEX_BANK)) == S826_ERR_OK)  //* BANKED - Lock board.
    {
        sys826_writereg(index, S826_ADR_INDEX, S826_BANK_DAC);      //* Select dac bank.
        DacRangeWrite(index, S826_ADR_INDIRECT, chan, range);       //* Program output range and 0V setpoint.
        sys826_unlock(index, MUTEX_BANK);                           //* Unlock board
    }
    return errcode;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Program DAC setpoint. This calibration-corrects the setpoint before writing it to dacram.
// Imports:
//  setpoint - unipolar ranges: 0x0000-0xFFFF = 0 to +Vmax, bipolar ranges: 0x0000-0xFFFF = -Vmax to +Vmax.

static void DacDataWrite(uint index, uint baseaddr, uint chan, uint setpoint)
{
    uint range;
    uint cmd;
    
    range = sys826_readreg(index, baseaddr + S826_OS_DACCFG(chan)) & DAC_MASK_RANGE;    // Fetch previously programmed range for this dac channel.
    cmd   = DacCmdSetOutput(chan, DacUserToRaw(index, setpoint, range));                // Convert user setpoint to raw data and format as dac command.
    sys826_writereg(index, baseaddr + S826_OS_DACDATA(chan), cmd);                      // Update dac output.
}

S826_API int S826_CC S826_DacDataWrite(uint board, uint chan, uint setpoint, uint safemode)
{
    uint index;
    int errcode;
    if (board >= S826_MAX_BOARDS)                                   // Error if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)        // Map board to index number, error if board is closed.
        return S826_ERR_BOARDCLOSED;
    if ((chan >= S826_NUM_DAC) || (setpoint > 0xFFFF))
        return S826_ERR_VALUE;

    if (safemode)                                                   // If programming safemode setpoint
        chan += S826_NUM_DAC;                                       //   adjust base register address offset to access safemode values.

    if (flataddr[index])                                            // FLAT
    {
        DacDataWrite(index, S826_ADR_DAC, chan, setpoint);          // Program dac setpoint.
        return S826_ERR_OK;
    }
    
    if ((errcode = sys826_lock(index, MUTEX_BANK)) == S826_ERR_OK)  //* BANKED - Lock board.
    {
        sys826_writereg(index, S826_ADR_INDEX, S826_BANK_DAC);      //* Select dac bank.
        DacDataWrite(index, S826_ADR_INDIRECT, chan, setpoint);     //* Program dac setpoint.
        sys826_unlock(index, MUTEX_BANK);                           //* Unlock board.
    }
    return errcode;
}

///////////////////////////////////////////////////////////////////////
// Return programmed DAC output range and setpoint.

static int DacRead(uint index, uint baseaddr, uint chan, uint *range, uint *setpoint, uint safemode)
{
    uint span;
    uint data;

    if (safemode)                                                   // If reading safemode data
        chan += S826_NUM_DAC;                                       //   adjust base register address offset to access safemode values.

    span = sys826_readreg(index, baseaddr + S826_OS_DACCFG(chan));  // Fetch dac range command and busy status.
    data = sys826_readreg(index, baseaddr + S826_OS_DACDATA(chan)); // Fetch dac setpoint command and busy status.

    *range    = span & DAC_MASK_RANGE;                              // extract range code
    *setpoint = DacRawToUser(index, data, *range);                  // convert native dac setpoint to cal-corrected user value

    if (!safemode)                                                  // If caller is checking runtime settings (vs. safemode),
        if ((span | data) & DAC_MASK_BUSY)                          //   if range or setpoint command has not yet been sent to dac device,
            return S826_ERR_NOTREADY;                               //     indicate value(s) has not yet been sent to device.

    return S826_ERR_OK;
}


S826_API int S826_CC S826_DacRead(uint board, uint chan, uint *range, uint *setpoint, uint safemode)
{
    uint index;
    int errcode;
    if (board >= S826_MAX_BOARDS)                                                       // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                            // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;
    if (chan >= S826_NUM_DAC)
        return S826_ERR_VALUE;

    if (flataddr[index])                                                                // FLAT
        return DacRead(index, S826_ADR_DAC, chan, range, setpoint, safemode);

    if ((errcode = sys826_lock(index, MUTEX_BANK)) == S826_ERR_OK)                      //* BANKED - Lock board.
    {
        sys826_writereg(index, S826_ADR_INDEX, S826_BANK_DAC);                          //* Select dac bank.
        errcode = DacRead(index, S826_ADR_INDIRECT, chan, range, setpoint, safemode);   //* Read range and setpoint.
        sys826_unlock(index, MUTEX_BANK);                                               //* Unlock board.
    }
    return errcode;
}

//////////////////////////////////////////////////////////////////////////
// Read/write raw dac setpoint.
// FOR INTERNAL SENSORAY USE ONLY.

// Program setpoint to uncorrected value.
S826_API int S826_CC S826_DacRawWrite(uint board, uint chan, uint setpoint)
{
    uint cmd;
    uint index;
    int errcode;
    if (board >= S826_MAX_BOARDS)                                               // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                    // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;
    if (chan >= S826_NUM_DAC)
        return S826_ERR_VALUE;

    cmd = DacCmdSetOutput(chan, setpoint << DAC_NUM_GUARDBITS);                 // Construct dac device command.

    if (flataddr[index])                                                        // FLAT
    {
        sys826_writereg(index, S826_ADR_DAC + S826_OS_DACDATA(chan), cmd);          // Write setpoint.
        return S826_ERR_OK;
    }

    if ((errcode = sys826_lock(index, MUTEX_BANK)) == S826_ERR_OK)              // BANKED
    {                                                                               //* Lock board.
        sys826_writereg(index, S826_ADR_INDEX, S826_BANK_DAC);                      //* Select dac bank.
        sys826_writereg(index, S826_ADR_INDIRECT + S826_OS_DACDATA(chan), cmd);     //* Write setpoint.
        sys826_unlock(index, MUTEX_BANK);                                           //* Unlock board.
    }
    return errcode;
}

// Return raw programmed setpoint. FOR INTERNAL SENSORAY USE ONLY.
S826_API int S826_CC S826_DacRawRead(uint board, uint chan, uint *setpoint)
{
    uint index;
    int errcode;
    *setpoint = 0;       // default return value upon error
    if (board >= S826_MAX_BOARDS)                                                               // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                                    // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;
    if (chan >= S826_NUM_DAC)                                                                   // Abort if invalid chan.
        return S826_ERR_VALUE;

    if (flataddr[index])                                                                        // FLAT
    {
        *setpoint = sys826_readreg(index, S826_ADR_DAC + S826_OS_DACDATA(chan)) & 0xFFFF;       // Read setpoint.
        return S826_ERR_OK;                                                                     // done
    }

    if ((errcode = sys826_lock(index, MUTEX_BANK)) == S826_ERR_OK)                              //* BANKED - Lock board.
    {
        sys826_writereg(index, S826_ADR_INDEX, S826_BANK_DAC);                                  //* Select dac bank.
        *setpoint = sys826_readreg(index, S826_ADR_INDIRECT + S826_OS_DACDATA(chan)) & 0xFFFF;  //* Read setpoint.
        sys826_unlock(index, MUTEX_BANK);                                                       //* Unlock board.
    }
    return errcode;                                                                             // done
}

///////////////////////////////////////////////////////////////////////////
// Copy dac calibration scalars (one per range) from eeprom to ram.
// EEPROM LOCK (#) MUST BE SET BEFORE CALLING.

static int DacCalEepromRead_EEL(uint index)
{
    int range;

    for (range = 0; range < DAC_NUM_RANGES; range++)                                            //# Establish default cal values in case eeprom is blank or faulty
        DacRangeCal[index][range] = 1.0;

    if (eeprom_fault[index])                                                                    //# Abort if eeprom problem detected.
        return S826_ERR_NOTREADY;

    if (EepromCmdREAD_EEL(index, S826_EEADR_DACVALID) == S826_EEDAT_CALVALID)                   //# If cal data is stored in eeprom
    {
        for (range = 0; range < DAC_NUM_RANGES; range++)                                        //# For each dac range, read cal const from eeprom and copy to ram:
        {
            double calval = EepromQuadletRead_EEL(index,
                S826_EEADR_DACDATA + 4 * range) / (double)CAL_FIXPT_1R0;                        //#   fixed-pt val: 0x80000000 == +1.000

            DacRangeCal[index][range] = ((calval >= 0.9) && (calval <= 1.0)) ? calval : 1.0;    //# Copy to ram (replace any dubious cal const with default value).
        }
    }

    return S826_ERR_OK;                                                                         //# done
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Read eeprom calibration values for the four dac output ranges. This doesn't affect current dac setpoints or output levels.
// On exit, slope[4] contain cal data.

S826_API int S826_CC S826_DacCalRead(uint board, double slope[4], uint *valid)
{
    uint range;
    uint index;
    int errcode;
    if (board >= S826_MAX_BOARDS)                                                           // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                                // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;
    if (eeprom_fault[index])                                                                // Abort if eeprom fault.
        return S826_ERR_NOTREADY;

    if ((errcode = sys826_lock(index, MUTEX_EEPROM)) == S826_ERR_OK)                        //# Lock board.
    {
        *valid = (EepromCmdREAD_EEL(index, S826_EEADR_DACVALID) == S826_EEDAT_CALVALID);    //# Determine whether eeprom data is valid.
        errcode = DacCalEepromRead_EEL(index);

        for (range = 0; range < DAC_NUM_RANGES; range++)                                    //# Copy cal values to user buffer.
            *slope++  = DacRangeCal[index][range];

        sys826_unlock(index, MUTEX_EEPROM);                                                 //# Unlock board.
    }
    return errcode;                                                                         // done
}

///////////////////////////////////////////////////////////////////////////////////////
// Program the current calibration settings to eeprom for the four dac output ranges.
// On entry, slope[4] contains cal data.

// Helper: validate dac calibration scalars.
static int isDacCalScalarsValid(const double scalars[4])
{
    uint range;
    for (range = 0; range < DAC_NUM_RANGES; range++)   // Reality check for cal values.
    {
        if ((scalars[range] < 0.9) || (scalars[range] > 1.0))
            return FALSE;
    }
    return TRUE;
}

// API function.
S826_API int S826_CC S826_DacCalWrite(uint board, const double scalars[4])
{
    uint range;
    uint index;
    int errcode;
    if (board >= S826_MAX_BOARDS)                                               // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                    // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;
    if (eeprom_fault[index])                                                    // Abort if eeprom problem detected.
        return S826_ERR_NOTREADY;
    if (!isDacCalScalarsValid(scalars))                                         // Abort if scalars invalid.
        return S826_ERR_VALUE;

    if ((errcode = sys826_lock(index, MUTEX_EEPROM)) == S826_ERR_OK)            //# Lock eeprom.
    {
        for (range = 0; range < DAC_NUM_RANGES; range++)
        {
            DacRangeCal[index][range] = scalars[range];                         //# Store new cal value to working storage.
            EepromQuadletWrite_EEL(index, S826_EEADR_DACDATA + 4 * range,       //# Copy new value to eeprom.
                (uint)(scalars[range] * (double)CAL_FIXPT_1R0));
        }

        EepromByteWrite_EEL(index, S826_EEADR_DACVALID, S826_EEDAT_CALVALID);   //# Set the eeprom cal data "valid" indicator.
        sys826_unlock(index, MUTEX_EEPROM);                                     //# Unlock eeprom.
    }
    return errcode;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////  ADC FUNCTIONS  //////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#define SYS_CLOCK               50                          // Local bus clock (MHz).

#define ADC_NUM_SLOTS           32                          // Total number of slots including privileged and app slots.
#define ADC_NUM_APPSLOTS        16                          // First 16 slots available to app; others are reserved for internal use by middleware.
#define ADC_NUM_RANGES          4                           // Number of supported input ranges.
#define ADC_APPSLOT_MASK        0x0000FFFF                  // Application slot mask.
#define ADC_NCHANS_PRIVILEGED   32                          // Total number of chans including privileged and app. 0-15=app, 16=open, 17=0V, 18=-5V, 19=+5V, 20-31=alias(16-19).
#define ADC_MAX_TSETTLE         (0x00FFFFFF / SYS_CLOCK)    // Max hardware timer value is 0x00FFFFFF, which corresponds to 335544.3 microseconds at 50MHz.

// ADCTL register bits.
#define ADCTL_BIT_RUN           29                          // Adc enable
#define ADCTL_BIT_HT            28                          // Triggered mode enable
#define ADCTL_BIT_TP            27                          // Trigger polarity
#define ADCTL_BIT_MT            26                          // Missed trigger

// ADCTL register masks.
#define ADCTL_MASK_RUN          (1 << ADCTL_BIT_RUN)
#define ADCTL_MASK_HT           (1 << ADCTL_BIT_HT)
#define ADCTL_MASK_TP           (1 << ADCTL_BIT_TP)
#define ADCTL_MASK_MT           (1 << ADCTL_BIT_MT)

// Slot configuration constants.
#define ADCCFG_ENABTIMER        0x80000000                  // Enable settling timer.
#define ADCCFG_EXTMUX           0x00000040                  // Select external analog mux (vs. internal cal mux).

// Offset/scalar calibration constant pair.
typedef struct ADC_CALPAIR {    // When cal-correcting raw adc data,
    int     offset;             //   first subtract offset from raw
    double  slope;              //   then multiply by slope
} ADC_CALPAIR;

// Offset/scalar constants associated with adc input ranges. Copied from eeprom at app startup.
// After writing new constants to eeprom, the adc must be restarted to activate new constants.
static ADC_CALPAIR AdcRangeCal[S826_MAX_BOARDS][ADC_NUM_APPSLOTS];

// Offset/scalar constants to use for each slot. These are changed by middleware for a slot whenever a new range is assigned to the slot.
static ADC_CALPAIR AdcSlotCal[S826_MAX_BOARDS][ADC_NUM_APPSLOTS];


//////////////////////////////////////////////////////////////////////////////////////////////////
// Initialize all slot offset/scalar values by copying from eeprom to ram.
// This function polls the eeprom interface while fetching the calibration constants.
// Board is locked while function executes.
// Returns error code.


// # EEPROM LOCK MUST BE SET BEFORE CALLING THIS FUNCTION.
static void AdcCalActivateEx_EEL(uint index, uint baseaddr)
{
    int slot;
    int range;

    for (slot = 0; slot < ADC_NUM_APPSLOTS; slot++)                     //# Repeat for each adc slot:
    {
        range = sys826_readreg(index, baseaddr + slot) & 3;             //#   read slot's programmed range
        AdcSlotCal[index][slot] = AdcRangeCal[index][range];            //#   set slot offset/scalar accordingly
    }
}

// For each slot, read slot's analog input range and set its slope/offset values accordingly.
// # EEPROM LOCK MUST BE SET BEFORE CALLING THIS FUNCTION.
static int AdcCalActivate_EEL(uint index)
{
    int errcode;

    if (flataddr[index])                                                //# FLAT
    {
        AdcCalActivateEx_EEL(index, S826_ADR_ADCCFG);                   //# Read each slot's programmed range and set its offset/scalar accordingly
        return S826_ERR_OK;                                             //# Success.
    }

    if ((errcode = sys826_lock(index, MUTEX_BANK)) == S826_ERR_OK)      //#* BANKED - Lock bank.
    {
        sys826_writereg(index, S826_ADR_INDEX, S826_BANK_ADCCFG);       //#* Select adc configuration bank.
        AdcCalActivateEx_EEL(index, S826_ADR_INDIRECT);                 //#* Read each slot's programmed range and set its offset/scalar accordingly
        sys826_unlock(index, MUTEX_BANK);                               //#* Unlock bank.
    }
    return errcode;                                                     //# done
}

//////////////////////////////////////////////////////////////
// Copy adc range calibration values from eeprom to ram.
// # EEPROM LOCK MUST BE SET BEFORE CALLING THIS FUNCTION.

static int AdcCalEepromRead_EEL(uint index)
{
    int range;

    for (range = 0; range < ADC_NUM_RANGES; range++)                            //# Establish default cal values in case eeprom is blank or faulty.
    {
        AdcRangeCal[index][range].offset = 0;
        AdcRangeCal[index][range].slope  = 0.25;
    }

    if (eeprom_fault[index])                                                    //# Abort if eeprom fault.
        return S826_ERR_NOTREADY;
    if (EepromCmdREAD_EEL(index, S826_EEADR_ADCVALID) != S826_EEDAT_CALVALID)   //# Abort if no cal data stored in eeprom.
        return S826_ERR_OK;

    for (range = 0; range < ADC_NUM_RANGES; range++)                            //# Copy adc cal constants from eeprom to ram for the four adc input ranges:
    {
        AdcRangeCal[index][range].offset = (int)EepromQuadletRead_EEL(index,    //#   offset
            S826_EEADR_ADCDATA + 8 * range);
        AdcRangeCal[index][range].slope  = (double)EepromQuadletRead_EEL(index, //#   scalar is fixed-point value: 0x80000000 == +1.000
            S826_EEADR_ADCDATA + 8 * range + 4) / (double)CAL_FIXPT_1R0;
    }
    return S826_ERR_OK;                                                         //# done
}

///////////////////////////////////////////////////////////////////////////////////
// Read eeprom calibration values for the four adc gain ranges.
// This also copies cal values to working ram and applies them to slots.
// On exit, slope[4] and offset[4] contain cal data.

S826_API int S826_CC S826_AdcCalRead(uint board, double slope[4], int offset[4], uint *valid)
{
    uint range;
    uint index;
    int errcode;
    if (board >= S826_MAX_BOARDS)                                                           // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                                // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;
    if (eeprom_fault[index])                                                                // Abort if eeprom fault.
        return S826_ERR_NOTREADY;
    if ((errcode = sys826_lock(index, MUTEX_EEPROM)) == S826_ERR_OK)                        //# Lock board.
    {
        *valid = (EepromCmdREAD_EEL(index, S826_EEADR_ADCVALID) == S826_EEDAT_CALVALID);    //# Check whether eeprom cal data is valid.

        if ((errcode = AdcCalEepromRead_EEL(index)) == S826_ERR_OK)                         //# Copy adc range calibration constants from eeprom to ram. If no errors:
        {
            for (range = 0; range < ADC_NUM_RANGES; range++)                                //#   Copy range cal values to application buffers.
            {
                *offset++ = AdcRangeCal[index][range].offset;
                *slope++  = AdcRangeCal[index][range].slope;
            }

            AdcCalActivate_EEL(index);                                                      //#   For each slot, read analog input range and set slope/offset values accordingly.
        }

        sys826_unlock(index, MUTEX_EEPROM);                                                 //# Unlock board.
    }
    return errcode;                                                                         // done
}

///////////////////////////////////////////////////////////////////////////////////////
// Program the current calibration settings for the four adc gain ranges.
// Writes cal values to eeprom and activates for subsequent conversions.
// On entry, slope[4] and offset[4] contain cal data.

S826_API int S826_CC S826_AdcCalWrite(uint board, const double slope[4], const int offset[4])
{
    uint range;
    uint index;
    int errcode;
    if (board >= S826_MAX_BOARDS)                                                       // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                            // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;
    if (eeprom_fault[index])
        return S826_ERR_NOTREADY;                                                       // Abort if eeprom fault.

    if ((errcode = sys826_lock(index, MUTEX_EEPROM)) == S826_ERR_OK)                    //# Lock board so no other processes can read cal values while we write new ones.
    {
        for (range = 0; range < ADC_NUM_RANGES; range++)
        {
            AdcRangeCal[index][range].offset = *offset;                                 //# Store new range cal values to working storage.
            AdcRangeCal[index][range].slope  = *slope;

            EepromQuadletWrite_EEL(index, S826_EEADR_ADCDATA + 8 * range, *offset++);   //# Copy new values to eeprom.
            EepromQuadletWrite_EEL(index, S826_EEADR_ADCDATA + 8 * range + 4,
                (uint)(*slope++ * (double)CAL_FIXPT_1R0));
        }

        EepromByteWrite_EEL(index, S826_EEADR_ADCVALID, S826_EEDAT_CALVALID);           //# Set the eeprom cal data "valid" indicator.

        AdcCalActivate_EEL(index);                                                      //# Update all slot cal values. This only affects the current process; other procs must restart to activate new eeprom values.
    
        sys826_unlock(index, MUTEX_EEPROM);                                             //# Unlock board.
    }
    return errcode;                                                                     // done
}

/////////////////////////////////////////////////////////////////////////
// Start/stop adc subsystem.

// Program AdcEnable.
S826_API int S826_CC S826_AdcEnableWrite(uint board, uint enable)
{
    uint index;
    if (board >= S826_MAX_BOARDS)                                                       // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                            // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;

    if (enable)
    {
        sys826_writereg(index, S826_ADR_ADCTL, PORT_BITCLR | ADC_APPSLOT_MASK);         // If enabling, reset any residual overflow indicators first and then enable conversions.
        sys826_writereg(index, S826_ADR_ADCTL, PORT_BITSET | ADCTL_MASK_RUN);
    }
    else
    {
        sys826_writereg(index, S826_ADR_ADCTL, PORT_BITCLR | ADCTL_MASK_RUN);           // If disabling, disable conversions first and then reset any overflow indicators.
        sys826_writereg(index, S826_ADR_ADCTL, PORT_BITCLR | ADC_APPSLOT_MASK);
    }
    return S826_ERR_OK;
}

// Return AdcEnable.
S826_API int S826_CC S826_AdcEnableRead(uint board, uint *enable)
{
    uint index;
    if (board >= S826_MAX_BOARDS)                                                       // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                            // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;

    *enable = ((sys826_readreg(index, S826_ADR_ADCTL) & ADCTL_MASK_RUN) != 0);
    return S826_ERR_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Configure ADC time slot.

// Helper: Build ADC slot configuration quadlet.
static uint BuildAdcConfig(uint chan, uint tsettle, uint gain)
{
    uint cfg = ((chan & 0x1F) << 2) | gain;                                             // External/internal mux, analog channel, bipolar span (0=10V, 1=5V, 2=2V, 3=1V).
    if (tsettle > 0)
        cfg |= (ADCCFG_ENABTIMER | ((tsettle * SYS_CLOCK) << 7));                       // Enable settling timer, settling time in microseconds.
    return cfg;
}

// API function: Write slot configuration.
S826_API int S826_CC S826_AdcSlotConfigWrite(uint board, uint slot, uint chan, uint tsettle, uint range)
{
    uint index;
    int errcode;
    if (board >= S826_MAX_BOARDS)                                                               // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                                    // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;
    if ((slot >= ADC_NUM_APPSLOTS) || (chan >= ADC_NCHANS_PRIVILEGED) || (tsettle > ADC_MAX_TSETTLE) || (range > S826_ADC_GAIN_10))
        return S826_ERR_VALUE;

    if (flataddr[index])                                                                        // FLAT
    {
        sys826_writereg(index, S826_ADR_ADCCFG + slot, BuildAdcConfig(chan, tsettle, range));
        AdcSlotCal[index][slot] = AdcRangeCal[index][range];                                    // set slot's calibration slope/offset according to gain range
        return S826_ERR_OK;
    }

    if ((errcode = sys826_lock(index, MUTEX_BANK)) == S826_ERR_OK)                              //* BANKED - Lock board.
    {
        sys826_writereg(index, S826_ADR_INDEX, S826_BANK_ADCCFG);                               //* Select adc configuration bank.
        sys826_writereg(index, S826_ADR_INDIRECT + slot, BuildAdcConfig(chan, tsettle, range)); //* Write slot configuration.
        sys826_unlock(index, MUTEX_BANK);                                                       //* Unlock board.
        AdcSlotCal[index][slot] = AdcRangeCal[index][range];                                    // set slot's calibration slope/offset according to gain range
    }
    return errcode;
}

// API function: Read slot configuration.
S826_API int S826_CC S826_AdcSlotConfigRead(uint board, uint slot, uint *chan, uint *tsettle, uint *gain)
{
    uint index;
    uint regval = 0;       // default return value upon error
    int errcode;
    if (board >= S826_MAX_BOARDS)                                                       // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                            // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;
    if (slot >= ADC_NUM_APPSLOTS)
        return S826_ERR_VALUE;

    if (flataddr[index])                                                                // FLAT
    {
        regval = sys826_readreg(index, S826_ADR_ADCCFG + slot);                         // Read flat register.
        errcode = S826_ERR_OK;
    }
    else if ((errcode = sys826_lock(index, MUTEX_BANK)) == S826_ERR_OK)                 //* BANKED - Lock board.
    {
        sys826_writereg(index, S826_ADR_INDEX, S826_BANK_ADCCFG);                       //* Select adc configuration bank.
        regval = sys826_readreg(index, S826_ADR_INDIRECT + slot);                       //* Read slot configuration.
        sys826_unlock(index, MUTEX_BANK);                                               //* Unlock board.
    }

    *chan    = (regval >> 2) & 0x1F;                                                    // External/internal mux, analog input channel number.
    *gain    = regval & 3;                                                              // Input span (bipolar): 0=10V, 1=5V, 2=2V, 3=1V.
    *tsettle = ((regval >> 7) & 0x00FFFFFF) / SYS_CLOCK;                                // Settling time in microseconds.
    return errcode;
}

////////////////////////////////////////////////////////////////////////////
// Program/return slotlist.

// Modify slotlist.
S826_API int S826_CC S826_AdcSlotlistWrite(uint board, uint slotlist, uint mode)
{
    uint index;
    if (board >= S826_MAX_BOARDS)                                                       // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                            // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;
    if ((slotlist > 0xFFFF) || (mode > 2))
        return S826_ERR_VALUE;

    sys826_writereg(index, S826_ADR_ADEN, (mode << 30) | slotlist);
    return S826_ERR_OK;
}

// Return slotlist.
S826_API int S826_CC S826_AdcSlotlistRead(uint board, uint *slotlist)
{
    uint index;
    if (board >= S826_MAX_BOARDS)                                                       // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                            // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;

    *slotlist = sys826_readreg(index, S826_ADR_ADEN);
    return S826_ERR_OK;
}

////////////////////////////////////////////////////////////////////////////
// Program/return ADC trigger mode: (enable,trigpol,srcselect[5:0].

static void AdcTrigModeWrite(uint index, uint router_baseaddr, uint trigmode)
{
    sys826_writereg(index, router_baseaddr + S826_OS_ROUTEADCT, trigmode & 0x3F);                               // Program dio routing control.
    sys826_writereg(index, S826_ADR_ADCTL, ADCTL_MASK_TP | ((trigmode & 0x40) ? PORT_BITSET : PORT_BITCLR));    // Program trigger polarity.
    sys826_writereg(index, S826_ADR_ADCTL, ADCTL_MASK_HT | PORT_BITSET);                                        // Enable triggering.
}

// Program trigger mode.
S826_API int S826_CC S826_AdcTrigModeWrite(uint board, uint trigmode)
{
    uint index;
    int errcode;
    if (board >= S826_MAX_BOARDS)                                                   // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                        // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;
    if ((trigmode & 0xFFFFFF00) != 0)                                               // Abort if bad argument.
        return S826_ERR_VALUE;

    if ((trigmode & 0x80) == 0)                                                     // If disabling hardware triggering
    {
        sys826_writereg(index, S826_ADR_ADCTL,  ADCTL_MASK_HT | PORT_BITCLR);       //   disable hardware triggering
        return S826_ERR_OK;
    }

    if (flataddr[index])                                                            // If enabling hardware triggering - FLAT
    {
        AdcTrigModeWrite(index, S826_ADR_ROUTER, trigmode);
        return S826_ERR_OK;
    }

    if ((errcode = sys826_lock(index, MUTEX_BANK)) == S826_ERR_OK)                  //* If enabling hardware triggering - BANKED - Lock board.
    {
        sys826_writereg(index, S826_ADR_INDEX, S826_BANK_ROUTER);                   //* Select router bank.
        AdcTrigModeWrite(index, S826_ADR_INDIRECT, trigmode);                       //* Program trigger mode.
        sys826_unlock(index, MUTEX_BANK);                                           //* Unlock board.
    }
    return errcode;
}

// Return trigger mode.
S826_API int S826_CC S826_AdcTrigModeRead(uint board, uint *trigmode)
{
    uint index;
    uint route = 0;
    uint ctl;
    int errcode;
    if (board >= S826_MAX_BOARDS)                                                   // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                        // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;

    if (flataddr[index])                                                            // FLAT - Read router's DIO select
    {
        route = sys826_readreg(index, S826_ADR_ROUTER + S826_OS_ROUTEADCT);
        errcode = S826_ERR_OK;
    }
    else if ((errcode = sys826_lock(index, MUTEX_BANK)) == S826_ERR_OK)             //* BANKED - Lock board.
    {
        sys826_writereg(index, S826_ADR_INDEX, S826_BANK_ROUTER);                   //* Select router bank.
        route = sys826_readreg(index, S826_ADR_INDIRECT + S826_OS_ROUTEADCT);       //* Read router's DIO select.
        sys826_unlock(index, MUTEX_BANK);                                           //* Unlock board.
    }

    ctl = sys826_readreg(index, S826_ADR_ADCTL);                                    // Read trigger enable and polarity

    *trigmode = ((ctl & ADCTL_MASK_HT) ? 0x80 : 0x00)                               // Combine TrigEnable
             | ((ctl & ADCTL_MASK_TP) ? 0x40 : 0x00)                                //   TrigPolarity
             | (route & 0x3F);                                                      //   and DIO select.

    return errcode;
}

//////////////////////////////////////////////////////////////////////////////
// Read DataAvailable status for application slots 0-15.

S826_API int S826_CC S826_AdcStatusRead(uint board, uint *slotlist)
{
    uint index;
    if (board >= S826_MAX_BOARDS)                                                   // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                        // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;

    *slotlist = sys826_readreg(index, S826_ADR_ADSTS) & 0xFFFF;                     // ValidData (mask app channels)
    return S826_ERR_OK;
}

/////////////////////////////////////////////////////////////
// Cancel blocking waits on arbitrary set of slots.
#ifndef RT_SUPPORT
S826_API int S826_CC S826_AdcWaitCancel(uint board, uint slotlist)
{
    uint index;
    if (board >= S826_MAX_BOARDS)                                                   // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                        // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;

    return sys826_waitcancel_adc(index, slotlist);
}
#endif
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Read digitized values from an arbitrary set of analog input timeslots and copy to application buffer.
// Imports:
//  buf      - Pointer to buffer that will receive ADC data (value range 0xFFF2000-0x0001FFF). Must be large enough to accomodate 16 values.
//  slotlist - Pointer to bit flags, one bit per slot. Set flags indicate slots of interest.
//             Slots 0-15 are used by application.
//             Slots 16-31 are privileged slots, available for internal use (e.g., calibration reference voltages).
//  tmax     - Maximum time, in milliseconds, to wait for ADC data. Non-blocking if 0.
// Exports:
//  slotlist - Bit flags, one bit per slot. Set flags indicate slots that have new data in buf.
// Returns:
//  S826_ERR_OK = success, else = error code


// Helper: Wait for ADC data to arrive on all slots of interest.
// Imports:
//  slotlist - Bit flags, one bit per slot, indicating slots of interest.
// Exports:
//  slotlist - Bit flags, one bit per slot, indicating slots that have data available. Set even if wait timeout occurs.
// Returns error code:
//  S826_ERR_OK         - data available from slots of interest.
//  S826_ERR_NOTREADY   - timed out (non-critical error, so slotlist set on exit).
//  others              - critical error or wait canceled.

static int AdcWaitDav(uint index, uint *slotlist, uint tmax)
{
#ifndef RT_SUPPORT
    uint tstart;
    uint telapsed;
#endif
//    int soi = *slotlist;        // Slots Of Interest.

    int ready_soi = sys826_readreg(index, S826_ADR_ADSTS) & *slotlist;      // Fetch data available flags and mask off uninteresting slots.
    if ((uint) ready_soi == *slotlist)                                             // Done if all slots of interest have been converted
        return S826_ERR_OK;

    if (tmax == 0)                                                          // If waiting is not permitted
    {
        *slotlist = ready_soi;                                              //   Indicate completed slots of interest.
        return S826_ERR_NOTREADY;                                           //   Indicate not all requested data is available.
    }

#ifdef RT_SUPPORT
    // waiting not support RT_SUPPORT is polled
    return S826_ERR_OSSPECIFIC;
#else
    tstart = sys826_readreg(index, S826_ADR_TSTAMP);                        // Remember wait start time.

    while (1)                                                               // Repeat forever:
    {
        int errcode = sys826_wait_adc(index, *slotlist & ~ready_soi, S826_WAIT_ALL, tmax);    // Wait for all unready soi's, or timeout, or wait canceled.

        switch (errcode)
        {
        case S826_ERR_OK:                                                       // WAIT SATISFIED
            return S826_ERR_OK;                                                 // Done.

        case S826_ERR_NOTREADY:                                                 // WAIT TIMED OUT
            telapsed = sys826_readreg(index, S826_ADR_TSTAMP) - tstart;         // Measure elapsed time since wait started.
            if (telapsed >= tmax)                                               // If an actual timeout ...
            {
                ready_soi = sys826_readreg(index, S826_ADR_ADSTS) & *slotlist;  //   Read status one final time
                if ((uint) ready_soi == *slotlist)                                     //   Check in case all soi's completed after the wait, and if so,
                    return S826_ERR_OK;                                         //     cancel timeout error.

                *slotlist = ready_soi;                                          //   Return list of partially completed soi's
                return S826_ERR_NOTREADY;                                       //     along with timeout error.
            }

            tmax -= telapsed;                                                   //   Not an actual timeout (os inaccuracy caused early timeout), so adjust wait time and
            break;                                                              //     do another wait.

        default:                                                                // CRITICAL ERROR or WAIT CANCELED
            *slotlist = 0;                                                      // Indicate no slot data available.
            return errcode;
        }
    }
#endif
}

// Helper: Copy ADC data to buffer.
// Imports:
//  slotlist - slots of interest (privileged and app slots) for which data is available.
//  buf      - buffer that will receive soi data.

static int AdcDataCopy(uint index, uint slotlist, int *data, uint *timestamps, int calcorrect)
{
    int         errcode;
    int         slot;
    uint        baseaddr_data;
    uint        baseaddr_tstamp;
    RREGSPEC    rlist[ADC_NUM_SLOTS];       // adc data
    RREGSPEC    tslist[ADC_NUM_SLOTS];      // timestamps
    uint        nslots = 0;

    if (slotlist == 0)                                      // Abort if no slots specified
        return S826_ERR_OK;

    // Determine base addresses for data and timestamp buffers.
    if (flataddr[index]) {
        baseaddr_data   = S826_ADR_ADCDATA;
        baseaddr_tstamp = S826_ADR_ADCTSTAMP;
    }
    else
        baseaddr_data = baseaddr_tstamp = S826_ADR_INDIRECT;

    // Build a list of slots to be copied. This applies to both privileged and app slots.
    for (slot = 0; slot < ADC_NUM_SLOTS; slot++)        // For each slot ...
    {
        if ((slotlist & (1 << slot)) != 0)                  // If this is a slot of interest and adc data is available ...
        {
            rlist[nslots].offset = baseaddr_data + slot;        // Add slot to adcdata list.
            rlist[nslots].dest   = &data[slot];
            if (timestamps != NULL)
            {
                tslist[nslots].offset = baseaddr_tstamp + slot; // Add slot to timestamp list (even though it may not be used).
                tslist[nslots].dest   = &timestamps[slot];
            }
            nslots++;
        }
    }

    // Copy slot data (burstcount, overflowflag, adcrawdata) and timestamps to app buffers.
    if (flataddr[index])                                                    // FLAT - If slot data of interest is available ...
    {
        sys826_readreg_list(index, rlist, nslots);                          // Read slot data from adc data ram (auto-clears hardware dav flags) and copy to app's data buffer.
        if (timestamps != NULL)
            sys826_readreg_list(index, tslist, nslots);                     // Read timestamps associated with slots of interest and copy to app's timestamps buffer.
        errcode = S826_ERR_OK;
    }
    else if ((errcode = sys826_lock(index, MUTEX_BANK)) == S826_ERR_OK)     //* BANKED - If slot data of interest is available ... Lock board.
    {
        sys826_writereg(index, S826_ADR_INDEX, S826_BANK_ADCDATA);          //* Select adc data bank.
        sys826_readreg_list(index, rlist, nslots);                          //* Read slot data from adc data ram (auto-clears hardware dav flags) and copy to app's data buffer.
        if (timestamps != NULL)
        {
            sys826_writereg(index, S826_ADR_INDEX, S826_BANK_ADCTSTAMP);    //* Select adc timestamp bank.
            sys826_readreg_list(index, tslist, nslots);                     //* Read timestamps associated with slots of interest and copy to app's timestamps buffer.
        }
        sys826_unlock(index, MUTEX_BANK);                                   //* Done accessing adc data/timestamp rams, so unlock board.
    }

    // Cal-correct the raw data (if enabled)
    if ((errcode == S826_ERR_OK) && (calcorrect != 0))                      // If requested to cal-correct adc data values ...
    {
        for (slot = 0; slot < ADC_NUM_APPSLOTS; slot++)                     // For each application slot (but not priveleged slots, which are never cal-corrected) ...
        {
            if ((slotlist & (1 << slot)) != 0)                              // If this is a slot of interest and adc data is available ...
            {
                ADC_CALPAIR *cal = &AdcSlotCal[index][slot];
                int rawadc    = (data[slot] & 0x00020000) ? (data[slot] | 0xFFFC0000) : (data[slot] & 0x0003FFFF);  //   Extract adc data and sign-extend it.
                int corrected = (int)((double)(rawadc - cal->offset) * cal->slope + 0.5);                           //   Correct data: val = (raw - offset) * scalar

                if (corrected < -32768)      corrected = -32768;                        //   Limit corrected data to 16-bit signed value
                else if (corrected > 32767)  corrected = 32767;

                data[slot] = (data[slot] & 0xFF800000) | (corrected & 0x0000FFFF);      //   Reinsert corrected data to app buffer (combine with burst count and overflow flag)
            }
        }
    }

    return errcode;
}

// API top-level helper for reading ADC data. Allows access to all slots (privileged and app slots).
static int AdcRead(uint index, int *data, uint *timestamps, uint *slotlist, int calcorrect, uint tmax)
{
    int errcode;
//    int soi = *slotlist;                // Slots Of Interest (soi) flags. Mask off unauthorized slot flags before calling.

    errcode = AdcWaitDav(index, slotlist, tmax);                                        // Wait for data on all interesting slots.

    if (errcode == S826_ERR_NOTREADY)                                                   // If any data unavailable then we have timed out, but
    {                                                                                   // timeout is non-critical (but higher priority than MissedTrigger or BufferOverflow), so ...
                                                                                        //   ignore NotReady error so partially available data can be copied.
        errcode = AdcDataCopy(index, *slotlist, data, timestamps, calcorrect);          //   copy all available adc data (if any) to buf.
        if (errcode == S826_ERR_OK)                                                     //   restore NotReady error if no errors detected during data copy.
            errcode = S826_ERR_NOTREADY;
    }
    else if (errcode != S826_ERR_OK)                                                    // Else if a critical error was detected (e.g., board closed, wait cancel) then
    {
        *slotlist = 0;                                                                  //   we can't continue, so exit with no data.
    }
    else
    {
        errcode = AdcDataCopy(index, *slotlist, data, timestamps, calcorrect);          // Else all interesting data is available, so copy adc data to app buffer.
        if (errcode == S826_ERR_OK)                                                     // If no errors copying data,
        {
            // If missed trigger and operating in Triggered mode (hardware triggering enabled) ...
            if ((sys826_readreg(index, S826_ADR_ADOVR) & (ADCTL_MASK_HT | ADCTL_MASK_MT)) == (ADCTL_MASK_HT | ADCTL_MASK_MT)) 
            {
                sys826_writereg(index, S826_ADR_ADCTL, ADCTL_MASK_MT | PORT_BITCLR);    // Reset MissedTrigger flag.
                errcode = S826_ERR_MISSEDTRIG;                                          // Indicate missed trigger.
            }
        }
    }

    return errcode;
}

// API function. Since this is exposed to app, access to privileged slots (16-31) is prohibited.
S826_API int S826_CC S826_AdcRead(uint board, int *data, uint *timestamps, uint *slotlist, uint tmax)
{
    uint index;
    if (board >= S826_MAX_BOARDS) {                                                     // Abort if invalid board number.
        *slotlist = 0;
        return S826_ERR_BOARD;
    }
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX) {                          // Map board to index number; abort if board closed.
        *slotlist = 0;
        return S826_ERR_BOARDCLOSED;
    }
    if ((*slotlist & ~ADC_APPSLOT_MASK) != 0) {                                         // Abort if app attempting to access privileged slots.
        *slotlist = 0;
        return S826_ERR_VALUE;
    }

    return AdcRead(index, data, timestamps, slotlist, 1 /* cal correct */, tmax);
}

// API function. INTENDED FOR INTERNAL SENSORAY USE ONLY.
S826_API int S826_CC S826_AdcRawRead(uint board, int *data, uint *timestamps, uint *slotlist, uint tmax)
{
    uint index;
    if (board >= S826_MAX_BOARDS) {                                                     // Abort if invalid board number.
        *slotlist = 0;
        return S826_ERR_BOARD;
    }
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX) {                          // Map board to index number; abort if board closed.
        *slotlist = 0;
        return S826_ERR_BOARDCLOSED;
    }

    return AdcRead(index, data, timestamps, slotlist, 0 /* no cal correct */, tmax);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////  GPIO FUNCTIONS  /////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#define GPIO_MASK       0x00FFFFFF      // isolate quadlet's 24-bit gpio data


//////////////////////////////////////////////////////////////////////////
// Return physical input states.

S826_API int S826_CC S826_DioInputRead(uint board, uint data[2])
{
    uint index;
    if (board >= S826_MAX_BOARDS)                                                   // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                        // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;

    data[0] = sys826_readreg(index, S826_ADR_GPIL);                                 // Read pin states.
    data[1] = sys826_readreg(index, S826_ADR_GPIH);
    return S826_ERR_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Program/return GPIO channel output sources (data bits: '0'=normal, '1'=counter output).

S826_API int S826_CC S826_DioOutputSourceWrite(uint board, const uint data[2])
{
    uint index;
    if (board >= S826_MAX_BOARDS)                                                   // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                        // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;

    sys826_writereg(index, S826_ADR_GPOSRCL, data[0]);                              // Write output source selectors.
    sys826_writereg(index, S826_ADR_GPOSRCH, data[1]);
    return S826_ERR_OK;
}

S826_API int S826_CC S826_DioOutputSourceRead(uint board, uint data[2])
{
    uint index;
    if (board >= S826_MAX_BOARDS)                                                   // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                        // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;

    data[0] = sys826_readreg(index, S826_ADR_GPOSRCL);                              // Read output source selectors.
    data[1] = sys826_readreg(index, S826_ADR_GPOSRCH);
    return S826_ERR_OK;
}

///////////////////////////////////////////////////////////////////////////
// Modify/return rising and falling edge capture enables.

// Modify capture enables.
S826_API int S826_CC S826_DioCapEnablesWrite(uint board, const uint rising[2], const uint falling[2], uint mode)
{
    uint index;
    if (board >= S826_MAX_BOARDS)                                                   // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                        // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;

    mode <<= 30;
    sys826_writereg(index, S826_ADR_GPRISEL, mode | (rising[0]  & GPIO_MASK));      // Write/modify capture enables.
    sys826_writereg(index, S826_ADR_GPRISEH, mode | (rising[1]  & GPIO_MASK));
    sys826_writereg(index, S826_ADR_GPFALLL, mode | (falling[0] & GPIO_MASK));
    sys826_writereg(index, S826_ADR_GPFALLH, mode | (falling[1] & GPIO_MASK));
    return S826_ERR_OK;
}

// Return capture enables.
S826_API int S826_CC S826_DioCapEnablesRead(uint board, uint *rising, uint *falling)
{
    uint index;
    if (board >= S826_MAX_BOARDS)                                                   // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                        // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;

    rising[0]  = sys826_readreg(index, S826_ADR_GPRISEL);                           // Read capture enables.
    rising[1]  = sys826_readreg(index, S826_ADR_GPRISEH);
    falling[0] = sys826_readreg(index, S826_ADR_GPFALLL);
    falling[1] = sys826_readreg(index, S826_ADR_GPFALLH);
    return S826_ERR_OK;
}

//////////////////////////////////////////////////////////////////////////
// Read/write GPIO runmode data output registers.

S826_API int S826_CC S826_DioOutputRead(uint board, uint data[2])
{
    uint index;
    if (board >= S826_MAX_BOARDS)                                                   // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                        // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;

    data[0] = sys826_readreg(index, S826_ADR_GPOL);                                 // Read runmode output data.
    data[1] = sys826_readreg(index, S826_ADR_GPOH);
    return S826_ERR_OK;
}

S826_API int S826_CC S826_DioOutputWrite(uint board, const uint data[2], uint mode)
{
    uint index;
    if (board >= S826_MAX_BOARDS)                                                   // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                        // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;

    mode <<= 30;
    sys826_writereg(index, S826_ADR_GPOL, mode | (data[0] & GPIO_MASK));            // Write/modify runmode output data.
    sys826_writereg(index, S826_ADR_GPOH, mode | (data[1] & GPIO_MASK));
    return S826_ERR_OK;
}

//////////////////////////////////////////////////////////////////////////
// Read/write GPIO safemode data output registers.

S826_API int S826_CC S826_DioSafeRead(uint board, uint data[2])
{
    uint index;
    if (board >= S826_MAX_BOARDS)                                                   // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                        // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;

    data[0] = sys826_readreg(index, S826_ADR_GPSAFEL);                              // Read safemode output data.
    data[1] = sys826_readreg(index, S826_ADR_GPSAFEH);
    return S826_ERR_OK;
}

S826_API int S826_CC S826_DioSafeWrite(uint board, const uint data[2], uint mode)
{
    uint index;
    if (board >= S826_MAX_BOARDS)                                                   // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                        // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;

    mode <<= 30;
    sys826_writereg(index, S826_ADR_GPSAFEL, mode | (data[0] & GPIO_MASK));         // Write/modify safemode output data.
    sys826_writereg(index, S826_ADR_GPSAFEH, mode | (data[1] & GPIO_MASK));
    return S826_ERR_OK;
}

//////////////////////////////////////////////////////////////////////////
// Read/write GPIO safemode enable registers.

S826_API int S826_CC S826_DioSafeEnablesRead(uint board, uint enables[2])
{
    uint index;
    if (board >= S826_MAX_BOARDS)                                                   // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                        // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;

    enables[0] = sys826_readreg(index, S826_ADR_GPSAFENL);                          // Read safemode enables.
    enables[1] = sys826_readreg(index, S826_ADR_GPSAFENH);
    return S826_ERR_OK;
}

S826_API int S826_CC S826_DioSafeEnablesWrite(uint board, const uint enables[2])
{
    uint index;
    if (board >= S826_MAX_BOARDS)                                                   // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                        // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;

    sys826_writereg(index, S826_ADR_GPSAFENL, enables[0]);                          // Write safemode enables.
    sys826_writereg(index, S826_ADR_GPSAFENH, enables[1]);
    return S826_ERR_OK;
}

/////////////////////////////////////////////////////////////
// Cancel blocking waits on arbitrary set of GPIOs.
#ifndef RT_SUPPORT
S826_API int S826_CC S826_DioWaitCancel(uint board, const uint data[2])
{
    uint index;
    if (board >= S826_MAX_BOARDS)                                                   // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                        // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;

    return sys826_waitcancel_dio(index, data);                                      // Cancel wait.
}

///////////////////////////////////////////////////////////////////////////////
// Read capture status and (optionally) wait for capture events.
// Resets capture flags on all channels of interest (coi).

S826_API int S826_CC S826_DioCapRead(uint board, uint chanlist[2], uint waitall, uint tmax)
{
    uint capstatus[2];
    uint index;
    int  errcode = S826_ERR_OK;
    if (board >= S826_MAX_BOARDS)                                                   // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                        // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;

    capstatus[0] = sys826_readreg(index, S826_ADR_GPSTSL);                          // Fetch capture status flags.
    capstatus[1] = sys826_readreg(index, S826_ADR_GPSTSH);

    if (tmax != 0)
    {
        int needwait = (waitall == 0) ?                                                 // Determine whether capture criteria is satisfied:
            (((chanlist[0] &  capstatus[0]) | (chanlist[1] &  capstatus[1])) == 0) :    //   Wait for any: satisfied if any coi has been captured.
            (((chanlist[0] & ~capstatus[0]) | (chanlist[1] & ~capstatus[1])) != 0);     //   Wait for all: satisfied if all coi's have been captured.

        if (needwait)                                                                   // If capture criteria is not satisfied
        {
            errcode = sys826_wait_dio(index, chanlist, (waitall ? S826_WAIT_ALL : S826_WAIT_ANY), tmax);    //   Wait on all coi's even though some may already have captures (in the case of waitall).
            if ((errcode != S826_ERR_OK) && (errcode != S826_ERR_NOTREADY))                                 //   Abort if critical error (e.g., wait cancel)
                return errcode;

            capstatus[0] = sys826_readreg(index, S826_ADR_GPSTSL);                      //   Fetch current capture status again, after waiting.
            capstatus[1] = sys826_readreg(index, S826_ADR_GPSTSH);
        }
    }

    chanlist[0] &= capstatus[0];                                                    // Post flags to app for all coi's that have captures (by resetting coi's that had no captures).
    chanlist[1] &= capstatus[1];

    sys826_writereg(index, S826_ADR_GPSTSL, chanlist[0]);                           // Reset capture flags.
    sys826_writereg(index, S826_ADR_GPSTSH, chanlist[1]);

    return errcode;
}

#endif
////////////////////////////////////////////////////////////////////////////////////////////
// Program/return GPIO debounce filter interval and enables.

S826_API int S826_CC S826_DioFilterWrite(uint board, const uint interval, const uint enables[2])
{
    uint index;
    if (board >= S826_MAX_BOARDS)                                                   // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                        // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;

    sys826_writereg(index, S826_ADR_DIOFILTIM, interval);                           // Write interval.
    sys826_writereg(index, S826_ADR_DIOFILTL, enables[0]);                          // Write enables.
    sys826_writereg(index, S826_ADR_DIOFILTH, enables[1]);
    return S826_ERR_OK;
}

S826_API int S826_CC S826_DioFilterRead(uint board, uint *interval, uint enables[2])
{
    uint index;
    if (board >= S826_MAX_BOARDS)                                                   // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                        // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;

    *interval = sys826_readreg(index, S826_ADR_DIOFILTIM);                          // Read interval.
    enables[0] = sys826_readreg(index, S826_ADR_DIOFILTL);                          // Read enables.
    enables[1] = sys826_readreg(index, S826_ADR_DIOFILTH);
    return S826_ERR_OK;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////  VIRTUAL I/O FUNCTIONS  ////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// Peripheral address map
#define S826_ADR_BGPDATA        (S826_ADR_BGPIO + 0)    // buried general-purpose outputs (normal mode)
#define S826_ADR_BGPSAFE        (S826_ADR_BGPIO + 1)    // buried general-purpose outputs (safe mode)
#define S826_ADR_BGPSAFEN       (S826_ADR_BGPIO + 2)    // buried general-purpose outputs safemode enables

#define BGPO_MASK               0x0000003F              // bit mask for the six buried dios


//////////////////////////////////////////////////////////////////////////
// Read/write buried GPIO runmode data output registers.

S826_API int S826_CC S826_VirtualRead(uint board, uint *data)
{
    uint index;
    if (board >= S826_MAX_BOARDS)                                                   // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                        // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;

    *data = sys826_readreg(index, S826_ADR_BGPDATA);                                // Read runmode output data.
    return S826_ERR_OK;
}

S826_API int S826_CC S826_VirtualWrite(uint board, const uint data, uint mode)
{
    uint index;
    if (board >= S826_MAX_BOARDS)                                                   // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                        // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;

    sys826_writereg(index, S826_ADR_BGPDATA, (mode <<= 30) | (data & BGPO_MASK));   // Write/modify runmode output data.
    return S826_ERR_OK;
}

//////////////////////////////////////////////////////////////////////////
// Read/write GPIO safemode data output registers.

S826_API int S826_CC S826_VirtualSafeRead(uint board, uint *data)
{
    uint index;
    if (board >= S826_MAX_BOARDS)                                                   // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                        // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;

    *data = sys826_readreg(index, S826_ADR_BGPSAFE);                                // Read safemode output data.
    return S826_ERR_OK;
}

S826_API int S826_CC S826_VirtualSafeWrite(uint board, const uint data, uint mode)
{
    uint index;
    if (board >= S826_MAX_BOARDS)                                                   // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                        // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;

    sys826_writereg(index, S826_ADR_BGPSAFE, (mode <<= 30) | (data & BGPO_MASK));   // Write/modify safemode output data.
    return S826_ERR_OK;
}

//////////////////////////////////////////////////////////////////////////
// Read/write GPIO safemode enable registers.

S826_API int S826_CC S826_VirtualSafeEnablesRead(uint board, uint *enables)
{
    uint index;
    if (board >= S826_MAX_BOARDS)                                                   // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                        // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;

    *enables = sys826_readreg(index, S826_ADR_BGPSAFEN);                            // Read safemode enables.
    return S826_ERR_OK;
}

S826_API int S826_CC S826_VirtualSafeEnablesWrite(uint board, const uint enables)
{
    uint index;
    if (board >= S826_MAX_BOARDS)                                                   // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                        // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;

    sys826_writereg(index, S826_ADR_BGPSAFEN, enables & BGPO_MASK);                 // Write/modify safemode enables.
    return S826_ERR_OK;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////  WATCHDOG FUNCTIONS  ///////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// Bit masks
#define S826_WDCFG_EN           0x00000020                  // CFG bit mask for watchdog EN (enable)
#define S826_WDCFG_MASK         0x0000005F                  // CFG bit mask for all configuration bits except watchdog EN (enable)
#define S826_WDCTRL_T0          0x00000001                  // status bit in WDCTRL: stage0 timeout

///////////////////////////////////////////////////////////////////////////////
// Return/program watchdog configuration settings.
// These affect all timer intervals and all configuration bits except EN.

// Helper functions.
static void WatchdogConfigRead(uint index, uint baseaddr, uint *config, uint timers[5])
{
    timers[0] = sys826_readreg(index, baseaddr + S826_OS_WDDELAY0);                 // stage0 interval (in 50 MHz clocks)
    timers[1] = sys826_readreg(index, baseaddr + S826_OS_WDDELAY1);                 // stage1 interval (in 50 MHz clocks)
    timers[2] = sys826_readreg(index, baseaddr + S826_OS_WDDELAY2);                 // stage2 interval (in 50 MHz clocks)
    timers[3] = sys826_readreg(index, baseaddr + S826_OS_WDPWIDTH);                 // ResetOut pulse width (in 50 MHz clocks)
    timers[4] = sys826_readreg(index, baseaddr + S826_OS_WDPGAP);                   // ResetOut pulse gap (in 50 MHz clocks)
    *config   = sys826_readreg(index, baseaddr + S826_OS_WDCFG) & S826_WDCFG_MASK;  // config: GSN, EN, SEN, NIE, PEN, OP, OEN (w/set-clr-wr). Mask off EN bit (which is accessed via enable read/write functions).
}

static void WatchdogConfigWrite(uint index, uint baseaddr, uint config, uint timers[5])
{
    sys826_writereg(index, baseaddr + S826_OS_WDDELAY0, timers[0]);                                     // stage0 delay (in 50 MHz clocks)
    sys826_writereg(index, baseaddr + S826_OS_WDDELAY1, timers[1]);                                     // stage1 delay (in 50 MHz clocks)
    sys826_writereg(index, baseaddr + S826_OS_WDDELAY2, timers[2]);                                     // stage2 delay (in 50 MHz clocks)
    sys826_writereg(index, baseaddr + S826_OS_WDPWIDTH, timers[3]);                                     // ResetOut pulse width (in 50 MHz clocks)
    sys826_writereg(index, baseaddr + S826_OS_WDPGAP,   timers[4]);                                     // ResetOut pulse gap (in 50 MHz clocks)
    sys826_writereg(index, baseaddr + S826_OS_WDCFG,    PORT_BITSET | (config & S826_WDCFG_MASK));      // configuration bits set: GSN, SEN, NIE, PEN, OP, OEN (don't change EN)
    sys826_writereg(index, baseaddr + S826_OS_WDCFG,    PORT_BITCLR | ((~config) & S826_WDCFG_MASK));   // configuration bits clr: GSN, SEN, NIE, PEN, OP, OEN (don't change EN)
}

// API functions
S826_API int S826_CC S826_WatchdogConfigRead(uint board, uint *config, uint timers[5])
{
    uint index;
    int errcode;
    if (board >= S826_MAX_BOARDS)                                               // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                    // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;

    // Read watchdog configuration and timing control registers.
    if (flataddr[index])
    {
        WatchdogConfigRead(index, S826_ADR_WDOG, config, timers);               // FLAT - Read watchdog configuration registers.
        return S826_ERR_OK;
    }
    if ((errcode = sys826_lock(index, MUTEX_BANK)) == S826_ERR_OK)              //* BANKED - Lock board.
    {
        sys826_writereg(index, S826_ADR_INDEX, S826_BANK_WDOG);                 //* Select watchdog bank.
        WatchdogConfigRead(index, S826_ADR_INDIRECT, config, timers);           //* Read watchdog configuration.
        sys826_unlock(index, MUTEX_BANK);                                       //* Unlock board.
    }
    return errcode;
}

S826_API int S826_CC S826_WatchdogConfigWrite(uint board, uint config, uint timers[5])
{
    uint index;
    int errcode;
    if (board >= S826_MAX_BOARDS)                                               // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                    // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;

    // Write watchdog configuration and timing control registers.
    if (flataddr[index])
    {
        WatchdogConfigWrite(index, S826_ADR_WDOG, config, timers);              // FLAT - Write watchdog configuration.
        return S826_ERR_OK;
    }
    if ((errcode = sys826_lock(index, MUTEX_BANK)) == S826_ERR_OK)              //* BANKED - Lock board.
    {
        sys826_writereg(index, S826_ADR_INDEX, S826_BANK_WDOG);                 //* Select watchdog bank.
        WatchdogConfigWrite(index, S826_ADR_INDIRECT, config, timers);          //* Write watchdog configuration registers.
        sys826_unlock(index, MUTEX_BANK);                                       //* Unlock board
    }
    return errcode;
}

/////////////////////////////////////////////////////////////
// Return/program watchdog enable.

S826_API int S826_CC S826_WatchdogEnableRead(uint board, uint *enable)
{
    uint index;
    uint regval = 0;       // default return value upon error
    int errcode;
    if (board >= S826_MAX_BOARDS)                                               // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                    // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;

    // Read watchdog configuration register.
    if (flataddr[index])
    {
        regval = sys826_readreg(index, S826_ADR_WDOG + S826_OS_WDCFG);
        errcode = S826_ERR_OK;
    }
    else if ((errcode = sys826_lock(index, MUTEX_BANK)) == S826_ERR_OK)         //* Lock board.
    {
        sys826_writereg(index, S826_ADR_INDEX, S826_BANK_WDOG);                 //* Select watchdog bank.
        regval = sys826_readreg(index, S826_ADR_INDIRECT + S826_OS_WDCFG);      //* Read watchdog enable.
        sys826_unlock(index, MUTEX_BANK);                                       //* Unlock board.
    }

    *enable = ((regval & S826_WDCFG_EN) != 0);
    return errcode;
}

S826_API int S826_CC S826_WatchdogEnableWrite(uint board, uint enable)
{
    uint index;
    uint wval = S826_WDCFG_EN | (enable ? PORT_BITSET : PORT_BITCLR);
    int errcode;
    if (board >= S826_MAX_BOARDS)                                               // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                    // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;

    if (flataddr[index])
    {
        sys826_writereg(index, S826_ADR_WDOG + S826_OS_WDCFG, wval);            // FLAT - enable/disable watchdog
        return S826_ERR_OK;
    }

    if ((errcode = sys826_lock(index, MUTEX_BANK)) == S826_ERR_OK)              //* BANKED - Lock board.
    {
        sys826_writereg(index, S826_ADR_INDEX, S826_BANK_WDOG);                 //* Select watchdog bank
        sys826_writereg(index, S826_ADR_INDIRECT + S826_OS_WDCFG, wval);        //* Write watchdog enable.
        sys826_unlock(index, MUTEX_BANK);                                       //* Unlock board.
    }
    return errcode;
}

/////////////////////////////////////////////////////////////
// Return watchdog status.

S826_API int S826_CC S826_WatchdogStatusRead(uint board, uint *status)
{
    uint index;
    int errcode;
    if (board >= S826_MAX_BOARDS)                                               // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                    // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;

    if (flataddr[index])
    {
        *status = sys826_readreg(index, S826_ADR_WDOG + S826_OS_WDCTRL);        // Read watchdog status register.
        return S826_ERR_OK;
    }

    if ((errcode = sys826_lock(index, MUTEX_BANK)) == S826_ERR_OK)              //* Lock board.
    {
        sys826_writereg(index, S826_ADR_INDEX, S826_BANK_WDOG);                 //* Select watchdog bank.
        *status = sys826_readreg(index, S826_ADR_INDIRECT + S826_OS_WDCTRL);    //* Read watchdog status.
        sys826_unlock(index, MUTEX_BANK);                                       //* Unlock board.
    }
    return errcode;
}

/////////////////////////////////////////////////////////////
// Kick watchdog. Data value must be 0x5A55AA5A.

S826_API int S826_CC S826_WatchdogKick(uint board, uint data)
{
    uint index;
    int errcode;
    if (board >= S826_MAX_BOARDS)                                               // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                    // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;

    if (flataddr[index])                                                        // FLAT
    {
        sys826_writereg(index, S826_ADR_WDOG + S826_OS_WDCTRL, data);
        return S826_ERR_OK;
    }

    if ((errcode = sys826_lock(index, MUTEX_BANK)) == S826_ERR_OK)              //* BANKED - Lock board.
    {
        sys826_writereg(index, S826_ADR_INDEX, S826_BANK_WDOG);                 //* Select watchdog bank.
        sys826_writereg(index, S826_ADR_INDIRECT + S826_OS_WDCTRL, data);       //* Kick watchdog.
        sys826_unlock(index, MUTEX_BANK);                                       //* Unlock board.
    }

    return errcode;
}

#ifndef RT_SUPPORT
/////////////////////////////////////////////////////////////
// Wait for watchdog stage0 event.

S826_API int S826_CC S826_WatchdogEventWait(uint board, uint tmax)
{
    uint index;
    uint status;
    int errcode;
    if (board >= S826_MAX_BOARDS)                                               // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                    // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;

    // Read watchdog event status.
    if (flataddr[index])                                                        // FLAT: Read watchdog status.
        status = sys826_readreg(index, S826_ADR_WDOG + S826_OS_WDCTRL);
    else if ((errcode = sys826_lock(index, MUTEX_BANK)) != S826_ERR_OK)         //* BANKED: Lock board.
        return errcode;                                                         // Exit if lock failed.
    else
    {
        sys826_writereg(index, S826_ADR_INDEX, S826_BANK_WDOG);                 //* Select watchdog bank.
        status = sys826_readreg(index, S826_ADR_INDIRECT + S826_OS_WDCTRL);     //* Read watchdog status.
        sys826_unlock(index, MUTEX_BANK);                                       //* Unlock board.
    }

    // Process event status.
    if (status & S826_WDCTRL_T0)                                                // If stage0 event
        return S826_ERR_OK;                                                     //   indicate watchdog event and exit
    if (tmax == 0)                                                              // If non-blocking call
        return S826_ERR_NOTREADY;                                               //   indicate wait timeout and exit

    return sys826_wait_watchdog(index, tmax);                                   // Block until stage0 event or wait cancel.
}

/////////////////////////////////////////////////////////////
// Cancel blocking waits on arbitrary set of GPIOs.

S826_API int S826_CC S826_WatchdogWaitCancel(uint board)
{
    uint index;
    if (board >= S826_MAX_BOARDS)                                               // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                    // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;

    return sys826_waitcancel_watchdog(index);                                   // Cancel wait.
}
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////  COUNTER FUNCTIONS  ////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// Status bits in CTRSTS register.
#define CTRSTS_RUN          0x20000000      // channel enabled
#define CTRSTS_ST           0x08000000      // level-sensitive "sticky" trigger
#define CTRSTS_DAV          0x00000080      // snapshot data available
#define CTRSTS_ZERO         0x00000040      // counter contains zero
#define CTRSTS_QUADERR      0x00000008      // registered quadrature error
#define CTRSTS_FIFOOVERFLOW 0x00000100      // counter snapshot fifo overflow
#define CTRSTS_UNDERFLOW    0x00000002      // registered counter underflow
#define CTRSTS_OVERFLOW     0x00000001      // registered counter overflow

// Snapshot reason.
#define CTRSTS_SHIFT_SR     18              // nshift to right justify
#define CTRSTS_MASK_SR      0x000001FF      // reason mask (right justified)

// Self-resetting command bits in CTRCMD register.
#define CTRCMD_SNAPSHOT     0x80000000      // force counter snapshot
#define CTRCMD_PRELOAD      0x40000000      // force counter preload
#define CTRCMD_RUN          0x20000000      // enable channel
#define CTRCMD_HALT         0x10000000      // disable channel
#define CTRCMD_ST           0x08000000      // assert level-sensitive "sticky" trigger
#define CTRCMD_RT           0x04000000      // negate level-sensitive "sticky" trigger
#define CTRCMD_FIFOOVERFLOW 0x00000100      // counter snapshot fifo overflow
#define CTRCMD_UNDERFLOW    0x00000002      // reset CTRSTS_UNDERFLOW
#define CTRCMD_OVERFLOW     0x00000001      // reset CTRSTS_OVERFLOW

// Mode register field positions.
#define CTRMODE_SHIFT_NR    23
#define CTRMODE_SHIFT_UD    22
#define CTRMODE_SHIFT_OM    18
#define CTRMODE_SHIFT_OP    17
#define CTRMODE_SHIFT_TP    14
#define CTRMODE_SHIFT_TS    11
#define CTRMODE_SHIFT_TE    9
#define CTRMODE_SHIFT_TD    7
#define CTRMODE_SHIFT_K     4
#define CTRMODE_SHIFT_XS    0

// Mode register field masks.
#define CTRMODE_MASK_NR     ( 1 << CTRMODE_SHIFT_NR)
#define CTRMODE_MASK_UD     ( 1 << CTRMODE_SHIFT_UD)
#define CTRMODE_MASK_OM     ( 7 << CTRMODE_SHIFT_M )
#define CTRMODE_MASK_OP     ( 1 << CTRMODE_SHIFT_OP)
#define CTRMODE_MASK_TP     ( 3 << CTRMODE_SHIFT_TP)
#define CTRMODE_MASK_TS     ( 3 << CTRMODE_SHIFT_TS)
#define CTRMODE_MASK_TE     ( 1 << CTRMODE_SHIFT_TE)
#define CTRMODE_MASK_TD     ( 3 << CTRMODE_SHIFT_TD)
#define CTRMODE_MASK_K      ( 7 << CTRMODE_SHIFT_K )
#define CTRMODE_MASK_XS     (15 << CTRMODE_SHIFT_XS)

// Mode register field enumerations -----------------

                                    // IP (ExtIn polarity):
#define CTRMODE_IP_NORMAL       0   //  Normal.
#define CTRMODE_IP_INVERTED     1   //  Inverted.
                                    // IM (ExtIn mode):
#define CTRMODE_IM_NOTUSED      0   //  ExtIn not used.
#define CTRMODE_IM_COUNTEN      1   //  Use as count permissive.
#define CTRMODE_IM_LOADEN       2   //  Use as preload permissive.
                                    // NR (Retrigger disable):
#define CTRMODE_NR_RETRIG       0   //  Retriggering enabled.
#define CTRMODE_NR_NORETRIG     1   //  Retriggering disabled.
                                    // UD (Normal/reverse count direction):
#define CTRMODE_UD_NORMAL       0   //  Normal.
#define CTRMODE_UD_INVERTED     1   //  Inverted.
                                    // OM (ExtOut mode):
#define CTRMODE_OM_MATCH        0   //  Pulsed upon match0 or match1. Used in Encoder mode.
#define CTRMODE_OM_PRELOAD1     1   //  Active preload register selector. Used in PWM mode.
#define CTRMODE_OM_NOTZERO      2   //  Active when counts != 0. Used in PulseGen mode.
#define CTRMODE_OM_NONE         3   //  Never active. Pulse Generator mode.
                                    // OP (ExtOut polarity):
#define CTRMODE_OP_NORMAL       0   //  Normal.
#define CTRMODE_OP_INVERTED     1   //  Inverted.
                                    // TS (Snapshot trigger):
#define CTRMODE_TS_SOFTONLY     0   //  Disabled (SoftTrigger only).
#define CTRMODE_TS_IXRISE       1   //  Latch on index leading edge.
#define CTRMODE_TS_IXFALL       2   //  Latch on index leading edge.
                                    // TE (Count enable trigger):
#define CTRMODE_TE_ONCONFIG     0   //  Enable upon configuration (no trigger needed).
#define CTRMODE_TE_IXRISE       1   //  Enable on index leading edge.
                                    // TD (Count disable trigger): 
#define CTRMODE_TD_NEVER        0   //  Never disabled by trigger.
#define CTRMODE_TD_IXFALL       1   //  Disable on index trailing edge.
#define CTRMODE_TD_ZERCOUNT     2   //  Disable when zero counts reached.
                                    // K (Clock mode):
#define CTRMODE_K_MONO_AP       0   //   Single-phase, clock on RiseA.
#define CTRMODE_K_MONO_AN       1   //   Single-phase, clock on FallA.
#define CTRMODE_K_MONO_1MHZ     2   //   Use internal 1 MHz clock.
#define CTRMODE_K_MONO_25MHZ    3   //   Use internal 25 MHz clock.
#define CTRMODE_K_QUAD_X1_AP    4   //   Quadrature x1, clock on RiseA.
#define CTRMODE_K_QUAD_X1_AN    5   //   Quadrature x1, clock on FallA.
#define CTRMODE_K_QUAD_X2       6   //   Quadrature x2, clock on BothA.
#define CTRMODE_K_QUAD_X4       7   //   Quadrature x4, clock on all A/B edges.
                                    // XS (Index source): 
#define CTRMODE_XS_INT_1MHZ     15  //  Internal tick generator.
#define CTRMODE_XS_INT_100KHZ   14
#define CTRMODE_XS_INT_10KHZ    13
#define CTRMODE_XS_INT_1KHZ     12
#define CTRMODE_XS_INT_100HZ    11
#define CTRMODE_XS_INT_10HZ     10
#define CTRMODE_XS_INT_1HZ      9
#define CTRMODE_XS_INT_R1HZ     8
#define CTRMODE_XS_EXT_INVERT   1   //  External index input, inverted polarity.
#define CTRMODE_XS_EXT_NORMAL   0   //  External index input, normal polarity.

//////////////////////////////////////////////////////
// Helper functions: Read/write counter register.

static int CounterRegWrite(uint board, uint chan, uint regos, uint regval)
{
    uint index;
    int errcode;
    if (board >= S826_MAX_BOARDS)                                       // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)            // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;
    if (chan >= S826_NUM_COUNT)                                         // Abort if bad argument.
        return S826_ERR_VALUE;
    
    if (flataddr[index])
    {
        sys826_writereg(index, S826_ADR_CTR(chan) + regos, regval);     // FLAT - Write register.
        return S826_ERR_OK;
    }

    if ((errcode = sys826_lock(index, MUTEX_BANK)) == S826_ERR_OK)      //* BANKED - Lock board.
    {
        sys826_writereg(index, S826_ADR_INDEX, S826_BANK_CTR(chan));    //* Select counter bank.
        sys826_writereg(index, S826_ADR_INDIRECT + regos, regval);      //* Write counter register.
        sys826_unlock(index, MUTEX_BANK);                               //* Unlock board.
    }
    return errcode;
}

static int CounterRegRead(uint board, uint chan, uint regos, uint *regval)
{
    uint index;
    int errcode;
    if (board >= S826_MAX_BOARDS)                                       // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)            // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;
    if (chan >= S826_NUM_COUNT)                                         // Abort if bad argument.
        return S826_ERR_VALUE;

    if (flataddr[index])
    {
        *regval = sys826_readreg(index, S826_ADR_CTR(chan) + regos);    // FLAT - Read flat register.
        return S826_ERR_OK;
    }

    if ((errcode = sys826_lock(index, MUTEX_BANK)) == S826_ERR_OK)      //* BANKED - Lock board.
    {
        sys826_writereg(index, S826_ADR_INDEX, S826_BANK_CTR(chan));    //* Select counter bank.
        *regval = sys826_readreg(index, S826_ADR_INDIRECT + regos);     //* Read counter register.
        sys826_unlock(index, MUTEX_BANK);                               //* Unlock board.
    }
    return errcode;
}

/////////////////////////////////////////////////////////////
// Cancel blocking wait on a counter channel.
#ifndef RT_SUPPORT
S826_API int S826_CC S826_CounterWaitCancel(uint board, uint chan)
{
    uint index;
    if (board >= S826_MAX_BOARDS)                                       // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)            // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;
    if (chan >= S826_NUM_COUNT)                                         // Abort if bad argument.
        return S826_ERR_VALUE;

    return sys826_waitcancel_counter(index, chan);
}
#endif
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Read counter snapshot.

// Helper function. In banked mode, BOARD MUST BE LOCKED WITH COUNTER MAPPED INTO INDIRECT ACCESS WINDOW.
// Returns error code: CTRCMD_FIFOOVERFLOW if snapshot fifo overflowed, else S826_ERR_OK.
static int ReadSnapshot(uint index, uint baseaddr, uint *counts, uint *tstamp, uint *reason)
{
    uint status;

    if (counts != NULL)
        *counts = sys826_readreg(index, baseaddr + S826_OS_COUNTS);                 // Latch snapshot and read counts, clear dav.
    else
        sys826_readreg(index, baseaddr + S826_OS_COUNTS);                           // Must latch snapshot and clear dav even if not returning counts.

    if (tstamp != NULL)
        *tstamp = sys826_readreg(index, baseaddr + S826_OS_CTRTSTAMP);              // Read timestamp

    status = sys826_readreg(index, baseaddr + S826_OS_CTRSTS);                      // Read counter status
    if (reason != NULL)
        *reason = (status >> CTRSTS_SHIFT_SR) & CTRSTS_MASK_SR;                     // Extract reason flags

    if ((status & CTRSTS_FIFOOVERFLOW) == 0)                                        // Return OK if snapshot fifo not overflowed
        return S826_ERR_OK;

    sys826_writereg(index, baseaddr + S826_OS_CTRSTS, CTRCMD_FIFOOVERFLOW);         // Else clear overflow status and return warning
    return S826_ERR_FIFOOVERFLOW;
}

// API function.
S826_API int S826_CC S826_CounterSnapshotRead(uint board, uint chan, uint *counts, uint *tstamp, uint *reason, uint tmax)
{
    uint index;
    int errcode;
    if (board >= S826_MAX_BOARDS)                                                           // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                                // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;
    if (chan >= S826_NUM_COUNT)                                                             // Abort if bad argument.
        return S826_ERR_VALUE;

    if (flataddr[index])                                                                    // FLAT:
    {
        uint baseaddr = S826_ADR_CTR(chan);                                                 // Cache counter registers base address
        if (sys826_readreg(index, baseaddr + S826_OS_CTRSTS) & CTRSTS_DAV)                  // If snapshot available
            return ReadSnapshot(index, baseaddr, counts, tstamp, reason);                   //   read snapshot and copy to app buffers; exit
        if (tmax == 0)                                                                      // If non-blocking call
            return S826_ERR_NOTREADY;                                                       //   exit unsatisfied

#ifdef RT_SUPPORT
		// no wait supported for RT LINUXCNC (polled)
		return S826_ERR_OSSPECIFIC;
#else
        if ((errcode = sys826_wait_counter(index, 1 << chan, tmax)) == S826_ERR_OK)         // Block until snapshot or error (timeout or wait cancel). If not timeout or cancelled
            return ReadSnapshot(index, baseaddr, counts, tstamp, reason);                   //   read snapshot and copy to app buffers; exit
        return errcode;                                                                     // done
#endif
    }

    if ((errcode = sys826_lock(index, MUTEX_BANK)) == S826_ERR_OK)                              //* BANKED - Lock board.
    {
        sys826_writereg(index, S826_ADR_INDEX, S826_BANK_CTR(chan));                            //* Select counter bank.
        if (sys826_readreg(index, S826_ADR_INDIRECT + S826_OS_CTRSTS) & CTRSTS_DAV)             //* If snapshot available
        {
            errcode = ReadSnapshot(index, S826_ADR_INDIRECT, counts, tstamp, reason);           //*   read snapshot and copy to app buffers
            sys826_unlock(index, MUTEX_BANK);                                                   //*   unlock board
            return errcode;                                                                     //    exit
        }

        if (tmax == 0)                                                                          //* If non-blocking call
        {
            sys826_unlock(index, MUTEX_BANK);                                                   //*   unlock board
            return S826_ERR_NOTREADY;                                                           //    exit unsatisfied
        }
        else                                                                                    //* Else block until satisfied or error:
        {
#ifdef RT_SUPPORT
	    	// no wait supported for RT LINUXCNC (polled)
	    	return S826_ERR_OSSPECIFIC;
#else

            sys826_unlock(index, MUTEX_BANK);                                                   //*   Unlock board before blocking.
            if ((errcode = sys826_wait_counter(index, 1 << chan, tmax)) == S826_ERR_OK)         //    Block until snapshot or error (timeout or wait cancel). If not timed out or cancelled ...
            {
                if ((errcode = sys826_lock(index, MUTEX_BANK)) == S826_ERR_OK)                  //*     Lock board.
                {
                    sys826_writereg(index, S826_ADR_INDEX, S826_BANK_CTR(chan));                //*     Select counter bank.
                    errcode = ReadSnapshot(index, S826_ADR_INDIRECT, counts, tstamp, reason);   //*     Read snapshot and copy to app buffers.
                    sys826_unlock(index, MUTEX_BANK);                                           //*     Unlock board
                }
            }
#endif
        }
    }
    return errcode;                                                                         // done
}

//////////////////////////////////////////////////////////////////////////////////////
// Read instantaneous counts.

S826_API int S826_CC S826_CounterRead(uint board, uint chan, uint *counts)
{
    return CounterRegRead(board, chan, S826_OS_QCOUNTS,counts);
}

//////////////////////////////////////////////////////////////////////////////////////
// Read/write compare register.
// Imports:
//  regid - 1=Compare1, 0=Compare0

S826_API int S826_CC S826_CounterCompareWrite(uint board, uint chan, uint regid, uint counts)
{
    return CounterRegWrite(board, chan, S826_OS_CMP0 + (regid != 0), counts);
}

S826_API int S826_CC S826_CounterCompareRead(uint board, uint chan, uint regid, uint *counts)
{
    return CounterRegRead(board, chan, S826_OS_CMP0 + (regid != 0), counts);
}

//////////////////////////////////////////////////////////////////////////////////////
// Read/write snapshot control register.

S826_API int S826_CC S826_CounterSnapshotConfigWrite(uint board, uint chan, uint cfg, uint mode)
{
    return CounterRegWrite(board, chan, S826_OS_SSCTL, (mode << 30) | (cfg & 0x3FFFFFFF));
}

S826_API int S826_CC S826_CounterSnapshotConfigRead(uint board, uint chan, uint *cfg)
{
    return CounterRegRead(board, chan, S826_OS_SSCTL, cfg);
}

////////////////////////////////////////////////////////////////
// Write/read preload register.

S826_API int S826_CC S826_CounterPreloadWrite(uint board, uint chan, uint reg, uint counts)
{
    return CounterRegWrite(board, chan, S826_OS_PRELOAD0 + (reg != 0), counts);
}

S826_API int S826_CC S826_CounterPreloadRead(uint board, uint chan, uint reg, uint *counts)
{
    return CounterRegRead(board, chan, S826_OS_PRELOAD0 + (reg != 0), counts);
}

//////////////////////////////////////////////////////////////////////////////////
// Soft preload (copy Preload0 to counter core).

S826_API int S826_CC S826_CounterPreload(uint board, uint chan, uint level, uint sticky)
{
    uint cmdmask;

    if (!sticky)    cmdmask = CTRCMD_PRELOAD | CTRCMD_RT;           // If not a sticky trigger, strobe the preload enable and cancel sticky if asserted.
    else if (level) cmdmask = CTRCMD_ST;                            // Else if "sticky active", assert the sticky preload enable.
    else            cmdmask = CTRCMD_RT;                            // Else it's "sticky inactive", so negate the preload enable.

    return CounterRegWrite(board, chan, S826_OS_CTRCMD, cmdmask);
}

////////////////////////////////////////////////
// Soft snapshot.

S826_API int S826_CC S826_CounterSnapshot(uint board, uint chan)
{
    return CounterRegWrite(board, chan, S826_OS_CTRCMD, CTRCMD_SNAPSHOT);
}

/////////////////////////////////////////////////////
// Read/write counter channel's ExtIn routing.

S826_API int S826_CC S826_CounterExtInRoutingWrite(uint board, uint ctrchan, uint diochan)
{
    uint index;
    int errcode;
    if (board >= S826_MAX_BOARDS)                                                           // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                                // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;
    if (ctrchan >= S826_NUM_COUNT)                                                          // Abort if bad argument.
        return S826_ERR_VALUE;

    if (flataddr[index])                                                                    // FLAT
    {
        sys826_writereg(index, S826_ADR_ROUTER + S826_OS_ROUTECEN(ctrchan), diochan);
        return S826_ERR_OK;
    }

    if ((errcode = sys826_lock(index, MUTEX_BANK)) == S826_ERR_OK)                          //* BANKED - Lock board.
    {
        sys826_writereg(index, S826_ADR_INDEX, S826_BANK_ROUTER);                           //* Select router bank.
        sys826_writereg(index, S826_ADR_INDIRECT + S826_OS_ROUTECEN(ctrchan), diochan);     //* Write routing control register.
        sys826_unlock(index, MUTEX_BANK);                                                   //* Unlock board.
    }
    return errcode;                                                                         // done
}

S826_API int S826_CC S826_CounterExtInRoutingRead(uint board, uint ctrchan, uint *diochan)
{
    uint index;
    int errcode;
    *diochan = 0;       // default return value upon error
    if (board >= S826_MAX_BOARDS)                                       // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)            // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;
    if (ctrchan >= S826_NUM_COUNT)                                      // Abort if bad argument
        return S826_ERR_VALUE;

    if (flataddr[index])
    {
        *diochan = sys826_readreg(index, S826_ADR_ROUTER + ctrchan);    // FLAT - Read register.
        return S826_ERR_OK;
    }
    
    if ((errcode = sys826_lock(index, MUTEX_BANK)) == S826_ERR_OK)      //* BANKED - Lock board.
    {
        sys826_writereg(index, S826_ADR_INDEX, S826_BANK_ROUTER);       //* Select router bank.
        *diochan = sys826_readreg(index, S826_ADR_INDIRECT + ctrchan);  //* Read routing control register.
        sys826_unlock(index, MUTEX_BANK);                               //* Unlock board.
    }
    return errcode;                                                     // done
}

////////////////////////////////////////////////////////////////////////////////////////////
// Read/write raw counter mode.

S826_API int S826_CC S826_CounterModeWrite(uint board, uint chan, uint mode)
{
    // D.A. 5/6/2014.  If invalid mode is sent to this register, the 826 will crash
    // An example is writing 0x4e2CDAF to mode.  After writing this, the board no longer
    // responds normally.  We need to check all bit values here.
    int td = (mode >> 7) & 0x03;
    int te = (mode >> 9) & 0x03;
    int im = (mode >> 28) & 0x03;
    if (td >= 3)
        return S826_ERR_VALUE;
    if (te >= 3)
        return S826_ERR_VALUE;
    if (im >= 3)
        return S826_ERR_VALUE;

    return CounterRegWrite(board, chan, S826_OS_CTRMODE, mode);
}

S826_API int S826_CC S826_CounterModeRead(uint board, uint chan, uint *mode)
{
    return CounterRegRead(board, chan, S826_OS_CTRMODE, mode);
}

/////////////////////////////////////////////////////////////////////////////
// Write counter run control.

S826_API int S826_CC S826_CounterStateWrite(uint board, uint chan, uint run)
{
    return CounterRegWrite(board, chan, S826_OS_CTRCMD, run ? CTRCMD_RUN : CTRCMD_HALT);
}

////////////////////////////////////////////////////////////
// Read counter status.

S826_API int S826_CC S826_CounterStatusRead(uint board, uint chan, uint *status)
{
    uint ctrsts;
    int errcode = CounterRegRead(board, chan, S826_OS_CTRSTS, &ctrsts);
    *status = ctrsts & (CTRSTS_RUN | CTRSTS_ST | CTRSTS_QUADERR);
    return errcode;
}

//////////////////////////////////////////////////////////////////////////////////////
// Read/write counter debounce filter control register.

S826_API int S826_CC S826_CounterFilterWrite(uint board, uint chan, uint cfg)
{
    return CounterRegWrite(board, chan, S826_OS_CTRFILT, cfg);
}

S826_API int S826_CC S826_CounterFilterRead(uint board, uint chan, uint *cfg)
{
    return CounterRegRead(board, chan, S826_OS_CTRFILT, cfg);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////  SYSTEM FUNCTIONS  /////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////
// Initialize system storage.

static void InitSystem(void)
{
    int i;
    for (i = 0; i < S826_MAX_BOARDS; i++)
    {
        bdindex[i]  = VACANT_BOARDINDEX;
        flataddr[i] = FALSE;
    }
    sys826_init();
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// Read API, driver and board version info.

// Helper function: read board number and pwb and fpga versions
static int BoardInfoRead(uint index, uint *bdnum, uint *bdrev, uint *fpgarev)
{
    uint config;
    uint rev;
                                                                        // Read info registers:
    config      = sys826_readreg(index, S826_ADR_CONFIG);               //   fpga build, pwb revision, board number
    rev         = sys826_readreg(index, S826_ADR_REV);                  //   fpga major/minor version (added in fpga version 0.0.69)

    if ((int)config == ~0)                                                   // If illegal value returned (couldn't read local bus)
        return S826_ERR_LOCALBUS;                                       //   Return error.

                                                                        // Extract and return info items:
    *bdnum      = (config >> 8) & 0x0F;                                 //   board number
    *bdrev      = (config >> 18) & 0x1F;                                //   pwb revision
    *fpgarev    = ((rev & 0xFFFF) << 16) | ((config >> 24) & 0xFF);     //   fpga rev: (major/8, minor/8, build/16)
    return S826_ERR_OK;
}

// API function
S826_API int S826_CC S826_VersionRead(uint board, uint *api, uint *driver, uint *bdrev, uint *fpgarev)
{
    uint index;
    uint bdnum;     // ignored
    int errcode;
    if (board >= S826_MAX_BOARDS)                                                               // Abort if invalid board number.
        return S826_ERR_BOARD;
    if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                                    // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;

    if ((errcode = sys826_readver(index, driver)) == S826_ERR_OK)                               // Get driver version,
    {
        *api = ((S826_VERSION_MAJOR << 24) | (S826_VERSION_MINOR << 16) | S826_VERSION_BUILD);  //   middleware version, and
        BoardInfoRead(index, &bdnum, bdrev, fpgarev);                                           //   board and fpga versions (board number is ignored)
    }
    return errcode;
}

//////////////////////////////////////////////////////////////////
// Close system. Closes all open boards.

S826_API int S826_CC S826_SystemClose()
{
    InitSystem();
    sys826_close();
    return S826_ERR_OK;
}

#ifndef RT_SUPPORT
//////////////////////////////////////////////////////////////////////////
// Open system. Discovers and opens all installed 826 boards.
// Returns:
//   0 or positive = flags for detected 826 boards, 1 bit per board. Bit position == board's switch setting.
//   negative      = failed

S826_API int S826_CC S826_SystemOpen(void)
{
    int     boardflags = 0;
    int     index;              // enumerated board number
    int     nboards;
    int     errcode;
    uint    board;              // logical board number (set via dip switches on board)
    uint    garev;
    uint    bdrev;              // ignored

    InitSystem();                                           // Init boardnum-to-indexnum map.

    nboards = sys826_open();                                // Detect and open all 826 boards. This returns board count (>= 0) or error code (< 0).
    if (nboards < 0)
        return nboards;                                     // Exit if error.

    for (index = 0; index < nboards; index++)               // For each found board's index number ...
    {
        // Get board attributes.
        errcode = BoardInfoRead(index, &board, &bdrev, &garev); // Read board info.
        if (errcode != S826_ERR_OK)                             // If board fault detected
        {
            S826_SystemClose();                                 //   Close all boards.
            return errcode;                                     //   Return error.
        }

        if (bdindex[board] != VACANT_BOARDINDEX)                // If a previously registered board has the same switch settings (i.e., duplicate board number)
        {
            S826_SystemClose();                                 //   Close all boards.
            return S826_ERR_DUPADDR;                            //   Return error.
        }

        bdindex[board] = index;                                 // Populate board access info.
        boardflags |= (1 << board);                             // Set BoardFound flag for this board.
        if (garev >= GA_BASEREV_FLATADDR)                       // If fpga version supports flat address space, indicate api should use flat addressing.
            flataddr[index] = TRUE;

        // Board initialization.
        eeprom_fault[index] = 1;                                // Assume eeprom fault until determined otherwise.
        if (sys826_lock(index, MUTEX_EEPROM) == S826_ERR_OK)    //# Lock board.
        {
            if (EepromDetect_EEL(index))                        //# If eeprom detected
                eeprom_fault[index] = 0;                        //#   indicate eeprom detected and seemingly functional.
            DacCalEepromRead_EEL(index);                        //# Copy dac range calibration constants from eeprom (or defaults if no eeprom) to ram.
            AdcCalEepromRead_EEL(index);                        //# Copy adc range calibration constants from eeprom (or defaults if no eeprom) to ram.
            AdcCalActivate_EEL(index);                          //# For each adc timeslot, read slot's analog input range and set its slope/offset values accordingly.
            sys826_unlock(index, MUTEX_EEPROM);                 //# Unlock board.
        }
    }

    return boardflags;
}
#else

S826_API int S826_CC S826_SystemOpen(int index, uint *brd_idx)
{
    int     boardflags = 0;
    int     errcode;
    uint    board;              // logical board number (set via dip switches on board)
    uint    garev;
    uint    bdrev;              // ignored

    if (index == 0) {
		// first board (of whatever board "NUM" or id) opened
		InitSystem();                                           // Init boardnum-to-indexnum map.
    }

    // Get board attributes.
    errcode = BoardInfoRead(index, &board, &bdrev, &garev); // Read board info.
    if (errcode != S826_ERR_OK)                             // If board fault detected
    {
		S826_SystemClose();                                 //   Close all boards.
		return errcode;                                     //   Return error.
    }
    
    if (bdindex[board] != VACANT_BOARDINDEX)                // If a previously registered board has the same switch settings (i.e., duplicate board number)
    {
		S826_SystemClose();                                 //   Close all boards.
		return S826_ERR_DUPADDR;                            //   Return error.
    }

    bdindex[board] = index;                                 // Populate board access info.
    boardflags |= (1 << board);                             // Set BoardFound flag for this board.
    if (garev >= GA_BASEREV_FLATADDR)                       // If fpga version supports flat address space, indicate api should use flat addressing.
		flataddr[index] = TRUE;
    
    // Board initialization.
    eeprom_fault[index] = 1;                                // Assume eeprom fault until determined otherwise.
    if (sys826_lock(index, MUTEX_EEPROM) == S826_ERR_OK)    //# Lock board.
    {
		if (EepromDetect_EEL(index))                        //# If eeprom detected
			eeprom_fault[index] = 0;                        //#   indicate eeprom detected and seemingly functional.
		DacCalEepromRead_EEL(index);                        //# Copy dac range calibration constants from eeprom (or defaults if no eeprom) to ram.
		AdcCalEepromRead_EEL(index);                        //# Copy adc range calibration constants from eeprom (or defaults if no eeprom) to ram.
		AdcCalActivate_EEL(index);                          //# For each adc timeslot, read slot's analog input range and set its slope/offset values accordingly.
		sys826_unlock(index, MUTEX_EEPROM);                 //# Unlock board.
    }
    *brd_idx = board;
    return 0;
}

#endif

/////////////////////////////////////////////////////////////////////////////////////////////////
// Read/write COMCMD port

S826_API int S826_CC S826_SafeControlRead(uint board, uint *settings)
{
    uint index;
    if (board >= S826_MAX_BOARDS)                                                       // Abort if invalid board number.
        return S826_ERR_BOARD;
    else if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                       // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;

    *settings = sys826_readreg(index, S826_ADR_CONFIG) & COMCMD_MASK;                   // read COMCMD port; extract user-accessible control flags.
    return S826_ERR_OK;
}

S826_API int S826_CC S826_SafeControlWrite(uint board, uint settings, uint mode)
{
    uint index;
    if (board >= S826_MAX_BOARDS)                                                       // Abort if invalid board number.
        return S826_ERR_BOARD;
    else if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                       // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;
    if (mode > 2)
        return S826_ERR_VALUE;

    sys826_writereg(index, S826_ADR_CONFIG, (mode << 30) | (settings & COMCMD_MASK));   // program COMCMD register
    return S826_ERR_OK;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Read/write safemode-related write enable

S826_API int S826_CC S826_SafeWrenRead(uint board, uint *enable)
{
    uint index;
    if (board >= S826_MAX_BOARDS)                                                       // Abort if invalid board number.
        return S826_ERR_BOARD;
    else if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                       // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;

    *enable = sys826_readreg(index, S826_ADR_SAFEN);
    return S826_ERR_OK;
}

S826_API int S826_CC S826_SafeWrenWrite(uint board, uint data)
{
    uint index;
    if (board >= S826_MAX_BOARDS)                                                       // Abort if invalid board number.
        return S826_ERR_BOARD;
    else if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                       // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;

    sys826_writereg(index, S826_ADR_SAFEN, data);                                       // Write savemode data writeEnable: 2=enable, 1=disable, else=no change
    return S826_ERR_OK;
}

//////////////////////////////////////////////////////////////////////////////
// Read current timestamp.

S826_API int S826_CC S826_TimestampRead(uint board, uint *timestamp)
{
    uint index;
    if (board >= S826_MAX_BOARDS)                                                       // Abort if invalid board number.
        return S826_ERR_BOARD;
    else if ((index = bdindex[board]) == (uint)VACANT_BOARDINDEX)                       // Map board to index number; abort if board closed.
        return S826_ERR_BOARDCLOSED;

    *timestamp = sys826_readreg(index, S826_ADR_TSTAMP);
    return S826_ERR_OK;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////  FOR SENSORAY INTERNAL TESTING ONLY  //////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


S826_API void S826_CC S826_WriteReg(uint board, uint offset, uint val)
{
    sys826_writereg(bdindex[board], offset, val);
}

S826_API uint S826_CC S826_ReadReg(uint board, uint offset)
{
    return sys826_readreg(bdindex[board], offset);
}

S826_API int S826_CC S826_WriteBridge(uint board, uint lcs, uint offset, uint value, uint rindex, uint rcode)
{
    uint index = bdindex[board];
    if (!lcs)
        return -1;
    return sys826_writeLCS(index, offset, value, rindex, rcode);
}

S826_API int S826_CC S826_ReadBridge(uint board, uint lcs, uint offset, uint *value, uint rindex, uint rcode)
{
    uint index = bdindex[board];
    if (!lcs)
        return -1;
    return sys826_readLCS(index, offset, value, rindex, rcode);
}

