// Copyright Sensoray Company Inc. 2012

#ifndef S826CONST_12345_H
#define S826CONST_12345_H

// Constants /////////////////////////////////////////////////////////////////////////////////////////////////////


// Direct address space -------------------

// Miscellaneous
#define S826_ADR_INDEX          0x00    // RW index / signature
#define S826_ADR_COMCTL         0x01    // RW counter common control
#define S826_ADR_COMCMD         0x02    // -W common command
#define S826_ADR_CONFIG         0x02    // R- config

#ifdef S826_DRIVER_BUILD
#define S826_ADR_INTEN          0x03    // interrupt enable *** controlled by driver
#endif

#define S826_ADR_INTSTS         0x04    // interrupt status
#define S826_ADR_EEPROM         0x05    // eeprom interface
#define S826_ADR_RAMPAGE        0x06    // ram page selector

// GPIO
#define S826_ADR_GPOL           0x0A    
#define S826_ADR_GPOH           0x0B    
#define S826_ADR_GPIL	        0x0C
#define S826_ADR_GPIH	        0x0D
#define S826_ADR_GP0SRCL        0x0E
#define S826_ADR_GP0SRCH        0x0F
#define S826_ADR_GPRISEL        0x10
#define S826_ADR_GPRISEH        0x11
#define S826_ADR_GPFALLL        0x12
#define S826_ADR_GPFALLH        0x13
#define S826_ADR_CAPEL	        0x14	// DIN capture enable bit channels 0...23
#define S826_ADR_CAPEH	        0x15	// DIN capture enable bit channels 24...47
#define S826_ADR_GPSTSL         0x16     // DIN capture polarity/status 0...31
#define S826_ADR_GPSTSH         0x17     // DIN capture polarity/status 32...47

#ifdef S826_DRIVER_BUILD
#define S826_ADR_CAPIENL        0x18    // DIN capture status 0...31 *** controlled by driver
#define S826_ADR_CAPIENH        0x19    // DIN capture status 32...47 *** controlled by driver
#endif

// ADC
#define S826_ADR_ADSTS          0x1A    // ADC status
#define S826_ADR_ADCTL          0x1B    // -W ADC control/overflow reset
#define S826_ADR_ADOVR          0x1B    // R- ADC overflow status
#ifdef S826_DRIVER_BUILD
#define S826_ADR_ADINTEN        0x1C    // ADC channel interrupt enables *** controlled by driver
#endif
#define S826_ADR_ADEN           0x1D    // ADC channel scan enables

// Indirect bank selects ------------------------------------

//                         0x00-0x05    // Counter channels 0-5
#define S826_INDEX_DAC          0x08    // ADC data ram
#define S826_INDEX_ADCDATA      0x09    // ADC data ram
#define S826_INDEX_ADCCFG       0x0A    // ADC configuration ram
#define S826_INDEX_ROUTER       0x0B    // DIO routing matrix controller
#define S826_INDEX_GPRAM	    0x0C    // General-purpose ram

// Indirect address spaces --------------------

// Counters
#define S826_IND_COUNTS         0x20    // snapshot counts
#define S826_IND_TSTAMP         0x21    // snapshot timestamp
#define S826_IND_PRELOAD0       0x22    // preload register 0
#define S826_IND_PRELOAD1       0x23    // preload register 1
#define S826_IND_CMPA		    0x24	// compare A register
#define S826_IND_CMPB		    0x25	// compare B register
#define S826_IND_CMPEN	        0x26    // compare enables
#define S826_IND_SETCMPEN	    0x27    // bit set compare enables
#define S826_IND_CLRCMPEN	    0x28    // bit clear compare enables
#define S826_IND_CTRMODE        0x29    // mode control register
#define S826_IND_CTRCMD         0x2A    // -W counter command
#define S826_IND_CTRSTS         0x2A    // R- counter status register

// DAC data, DAC config, ADC data, ADC config, general-purpose RAM
#define S826_IND_DACDATA(i)     ((i) | 0x20)
#define S826_IND_DACCFG(i)      ((i) | 0x28)
#define S826_IND_ADCDATA(i)     ((i) | 0x20)
#define S826_IND_ADCCFG(i)      ((i) | 0x20)
#define S826_IND_GPRAM(i)       ((i) | 0x20)

// Signal router
#define S826_IND_ROUTECEN0      0x20
#define S826_IND_ROUTECEN1      0x21
#define S826_IND_ROUTECEN2      0x22
#define S826_IND_ROUTECEN3      0x23
#define S826_IND_ROUTECEN4      0x24
#define S826_IND_ROUTECEN5      0x25
#define S826_IND_ROUTEADCT      0x28

#ifdef S826_DRIVER_BUILD

#define S826_MASK_INTEN_CT        0x3F  //Counter mask for INTEN and INTSTS
#define S826_MASK_INTEN_BD        0x40  //global board interrupt enable
#define S826_MASK_INTSTS_DIO      0x80  //DIO INTSTS
#define S826_MASK_INTSTS_ADC      0x40  //ADC INTSTS
#define S826_MASK_INTEN_WD        0x100 //mask for WD (INTEN and INTSTS)

#endif



#endif
