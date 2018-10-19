///////////////////////////////////////////////////////////////////////////////////////////
// File      : 826drv.h
// Function  : Private include for 826 middleware
// Copyright : (C) Sensoray Company Inc. 2012
///////////////////////////////////////////////////////////////////////////////////////////

#ifndef _INC_826DRV_H_
#define _INC_826DRV_H_

typedef enum MUTEX_ID {     // Mutex identifiers:
    MUTEX_EEPROM    = 0,    //   Lock eeprom interface.
    MUTEX_BANK      = 1,    //   Lock banked registers.
    MUTEXCOUNT      = 2     // Number of mutexes per board -- THIS MUST BE LAST
} MUTEX_ID;
#define NMUTEXES    ((int)MUTEXCOUNT)

typedef struct WREGSPEC {
    uint offset;
    uint val;
} WREGSPEC;

typedef struct RREGSPEC {
    uint offset;
    uint *dest;
} RREGSPEC;


// Function prototypes for driver wrappers -----------------------------

// Functions in 826drv.c
void sys826_init                (void);
int  sys826_open                (void);
void sys826_close               (void);
int  sys826_isboardopen         (uint index);
uint sys826_readreg             (uint index, uint offset);
void sys826_readreg_list        (uint index, RREGSPEC *rlist, uint nreads);
void sys826_writereg            (uint index, uint offset, uint val);
void sys826_writereg_list       (uint index, WREGSPEC *wlist, uint nwrites);
int  sys826_lock                (uint index, MUTEX_ID mutid);
void sys826_unlock              (uint index, MUTEX_ID mutid);


int  sys826_readver             (uint index, uint *version);

int  sys826_readLCS             (uint index, uint offset, uint *value, uint rindex, uint rcode);
int  sys826_writeLCS            (uint index, uint offset, uint value, uint rindex, uint rcode);

#ifndef RT_SUPPORT
int  sys826_cancel_wait         (uint index);
int  sys826_wait_adc            (uint index, uint intlist, uint waitany, uint timeout);
int  sys826_wait_counter        (uint index, uint intlist, uint timeout);
int  sys826_wait_dio            (uint index, uint intlist[2], uint waitany, uint timeout);
int  sys826_wait_watchdog       (uint index, uint timeout);
int  sys826_waitcancel_adc      (uint index, uint slotlist);
int  sys826_waitcancel_counter  (uint index, uint chan);
int  sys826_waitcancel_dio      (uint index, const uint chanlist[2]);
int  sys826_waitcancel_watchdog (uint index);
#endif

#endif // ifndef _INC_826DRV_H_
