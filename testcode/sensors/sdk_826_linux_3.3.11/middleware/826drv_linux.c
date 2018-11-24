/* Copyright Sensoray Company Inc. 2010-2012
 * Driver call wrapper for Sensoray 826 for Linux
*/
// All functions in this module are visible only to the middleware.
// NEVER EXPOSE THESE FUNCTIONS TO THE USER APPLICATION.

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sched.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/file.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include "826drv.h"
#include "826api.h"
#include "s826ioctl_linux.h"
#include "platform.h"


#define DEV_NAME_		"/dev/s826_"

// Constants -----------------------------------------------------

#define VACANT_BOARDINDEX   (~0)UL

// Windows-specific error codes.
#define S826_ERR_GETCLASSDEVS                       (S826_ERR_OSSPECIFIC - 0)
#define S826_ERR_ENUMDEVICEINTERFACES               (S826_ERR_OSSPECIFIC - 1)
#define S826_ERR_GETDEVICEINTERFACEDETAILSIZE       (S826_ERR_OSSPECIFIC - 2)
#define S826_ERR_GETDEVICEINTERFACEDETAILS          (S826_ERR_OSSPECIFIC - 3)


// Types ---------------------------------------------------------

typedef unsigned int uint;

typedef struct
{
    int     hboard;             // Board handle. Set to NULL if board is unregistered.
    int hmutex;    // Mutex handle. Used to guard the INDEX register while accessing indirect register banks.
    uint    *baseadr;           // Pointer to board's register space. Set to NULL if board is unregistered.
} IXINFO;

typedef unsigned long DWORD;

// Storage --------------------------------------------------------

static IXINFO   ixinfo[S826_MAX_BOARDS];    // Info for registered boards, maps index to: hboard, hmutex, baseadr.

////////////////////////////////////////////////////////////////////////
// Get driver version.

int sys826_readver(uint index, uint *version)
{
    struct s826_version p;
    int rc;
    rc = ioctl(ixinfo[index].hboard, S826_IOC_VERSION, &p);
    *version = 0;
    if (rc == 0)
        *version = p.version;
    else
        return S826_ERR_DRIVER;
    return 0;
}

//////////////////////////////////////////////////////////////////////
// Direct register read/write.
// Caller is responsible for ensuring legal index number and board is open.

uint sys826_readreg(uint index, uint offset)
{
    return *(volatile uint *)(ixinfo[index].baseadr + offset);
};


void sys826_writereg(uint index, uint offset, uint val)
{
      *(volatile uint *)(ixinfo[index].baseadr + offset) = val;
}

void sys826_readreg_list(uint index, RREGSPEC *rlist, uint nreads)
{
    do
    {
        *(rlist->dest) = *(volatile uint *)(ixinfo[index].baseadr + rlist->offset);
        rlist++;
    }
    while (--nreads);
}

void sys826_writereg_list(uint index, WREGSPEC *wlist, uint nwrites)
{
    do
    {
        *(volatile uint *)(ixinfo[index].baseadr + wlist->offset) = wlist->val;
        wlist++;
    }
    while (--nwrites);
}

/////////////////////////////////////////////////////////////////////////////
// Mutex functions.
// Caller is responsible for ensuring legal index number.

// Create board lock. Returns mutex handle if successful, else 0.
static int sys826_create_lock(uint index)
{
    return 0; // using flock with file handle
}

// Lock board. Returns S826_ERR_OK if successful, else S826_ERR_CANCELLED | S826_ERR_BOARDCLOSED.
int sys826_lock(uint index, MUTEX_ID m)
{
    int rc;
    rc = flock(ixinfo[index].hboard, LOCK_SH);
    return (rc == 0) ? S826_ERR_OK : S826_ERR_CANCELLED;
}

// Unlock board.
void sys826_unlock(uint index, MUTEX_ID m)
{
    (void) flock(ixinfo[index].hboard, LOCK_UN);
    return;
}




////////////////////////////////////////////////////////////
// Initialize system storage.
// This must be called when a process attaches to the DLL.
void sys826_init(void)
{
    int i;
    for (i = 0; i < S826_MAX_BOARDS; i++) {
        ixinfo[i].hboard = -1;
        ixinfo[i].hmutex = -1;
        ixinfo[i].baseadr = NULL;
    }
}

////////////////////////////////////////////////////////////////////////////
// Close all boards.

void sys826_close(void)
{
    uint i;
    IXINFO *pinfo = ixinfo;
    for (i = 0; i < S826_MAX_BOARDS; i++, pinfo++) {
        if (pinfo->hboard != -1) {
            close(pinfo->hboard);        // Close board.
            pinfo->hboard = -1;
        }
        if (pinfo->hmutex != -1) {
            pinfo->hmutex = -1;
        }
    }
}

// open the board
static int OpenBoard(uint index, int *errcode)
{
    char devname[256];
    int hdev;
    if (index > S826_MAX_BOARDS) {
        *errcode = S826_ERR_VALUE;
        return -1;
    }
    sprintf(devname, DEV_NAME_"%01d", index);
    hdev = open(devname, O_RDWR);
    return hdev;
}

///////////////////////////////////////////////////////////////////////
// Get board's base memory address.

static int GetBoardAddress(int hboard, void **addr)
{
    struct s826_getmem p;
    int rc;
    void *mval;
    p.idx = 1;
    rc = ioctl(hboard, S826_IOC_GETMEM, &p);
    if (rc == 0) {
        mval = mmap(NULL, 256, PROT_READ | PROT_WRITE, MAP_SHARED, hboard,
                    (__off_t) p.bar);
        if (mval == NULL) 
            return S826_ERR_MEMORYMAP;
        else
            *addr = mval;
        return S826_ERR_OK;
    }
    return S826_ERR_DRIVER;
}


////////////////////////////////////////////////////////////////////////////////////////
// Open all boards. This must be called once to open access to all 826 boards.
// Returns:
//  if success - board count (positive value or zero).
//  if failed  - error code (negative value).

int sys826_open()
{
    uint    i;
    int     nboards = 0;            // no boards detected at any board index
    int     errnum  = S826_ERR_OK;  // no errors so far

    for (i = 0; i < S826_MAX_BOARDS; i++) {   // For each driver-enumerated board (in range 0 to boardcount-1) ...
        // Open board and get its handle.
        ixinfo[i].hboard = OpenBoard(i, &errnum);
        if (ixinfo[i].hboard == -1)   // Done if no more boards detected
            break;

        // Get board's base memory address.
        errnum = GetBoardAddress(ixinfo[i].hboard, (void **) &ixinfo[i].baseadr);
        if (errnum != S826_ERR_OK)
            break;

        // Create board mutex.
        ixinfo[i].hmutex = sys826_create_lock(i);       // Attempt to create a mutex.
        if (ixinfo[i].hmutex == -1) {
            errnum = S826_ERR_CREATEMUTEX;              // Abort if failed to create mutex.
            break;
        }

        nboards++;                                      // Bump registered board count.
    }

    if (errnum == S826_ERR_OK)                          // Exit with board count if no errors.
        return nboards;

    sys826_close();     // Close all open boards.
    return errnum;      // Return error code.
}



///////////////////////////////////////////////////////////////////////////////////
// Wait for counter interrupt or timeout, whichever comes first.
// Caller is responsible for ensuring legal board number.

int sys826_wait_counter(uint index, uint intlist, uint timeout)
{
    int rc;
    struct s826_waitcount p;
    p.usecs     = timeout;      // microseconds
    p.mask      = intlist;      // interrupt flag(s) to wait for
	p.bWaitAny  = 0;
    rc = ioctl(ixinfo[index].hboard, S826_IOC_WAITCOUNT, &p);
    if (rc == 0)
        return p.rc;
    else
        return rc;
}


///////////////////////////////////////////////////////////////////////////////////////////
// Block until any/all specified DIO channels have acquired data.
// Caller is responsible for ensuring legal board number.

int sys826_wait_dio(uint index, uint *intlist, uint waitany, uint timeout)
{
    int rc;
    struct s826_waitdio p;
    p.usecs     = timeout;      // microseconds
    p.mask[0]   = intlist[0];    // interrupt flag(s) to wait for
    p.mask[1]   = intlist[1];    // interrupt flag(s) to wait for
	p.bWaitAny  = (waitany != 0) ? 1 : 0;
    rc = ioctl(ixinfo[index].hboard, S826_IOC_WAITDIO, &p);
    if (rc == 0)
        return p.rc;
    else
        return rc;
}


///////////////////////////////////////////////////////////////////////////////////////////
// Block until any/all specified ADC channels have acquired data.
// Caller is responsible for ensuring legal board number.

int sys826_wait_adc(uint index, uint intlist, uint waitany, uint timeout)
{
    int rc;
    struct s826_waitadc p;
    p.usecs = timeout;      // microseconds
    p.mask = intlist;    // interrupt flag(s) to wait for
	p.bWaitAny = (waitany != 0) ? 1 : 0;
    rc = ioctl(ixinfo[index].hboard, S826_IOC_WAITADC, &p);
    if (rc == 0)
        return p.rc;
    else
        return rc;
}

int  sys826_wait_watchdog(uint index, uint timeout)
{
    int rc;
    struct s826_waitwd p;
    p.usecs = timeout;      // microseconds
    rc = ioctl(ixinfo[index].hboard, S826_IOC_WAITWD, &p);
    if (rc == 0)
        return p.rc;
    else
        return rc;
}

///////////////////////////////////////////////////////////////////////////////////////
// Cancel blocking calls.


int  sys826_waitcancel_adc(uint index, uint slotlist)
{
    int rc;
    struct s826_cancel_waitadc p;
    p.mask = slotlist;
    rc = ioctl(ixinfo[index].hboard, S826_IOC_CANCEL_WAITADC, &p);
    return (rc == 0) ? S826_ERR_OK : S826_ERR_DRIVER;
}

int  sys826_waitcancel_counter  (uint index, uint chan)
{
    int rc;
    struct s826_cancel_waitcount p;
    p.mask = (1 << chan);
    rc = ioctl(ixinfo[index].hboard, S826_IOC_CANCEL_WAITCOUNT, &p);
    return (rc == 0) ? S826_ERR_OK : S826_ERR_DRIVER;
}

int  sys826_waitcancel_dio(uint index, const uint chanlist[2])
{
    int rc;
    struct s826_cancel_waitdio p;
    p.mask[0] = chanlist[0];
    p.mask[1] = chanlist[1];
    rc = ioctl(ixinfo[index].hboard, S826_IOC_CANCEL_WAITDIO, &p);
    return (rc == 0) ? S826_ERR_OK : S826_ERR_DRIVER;
}

int  sys826_waitcancel_watchdog(uint index)
{
    int rc;
    struct s826_cancel_waitwd p;
    rc = ioctl(ixinfo[index].hboard, S826_IOC_CANCEL_WAITWD, &p);
    return (rc == 0) ? S826_ERR_OK : S826_ERR_DRIVER;
}

////////////////////////////////////////////////////////
// LCS access -- for internal Sensoray use only.
// Note: Functions have not been tested

int sys826_writeLCS(uint index, uint offset, uint value, uint rindex, uint rcode)
{
    int rc;
    struct s826_pci_reg p;
    p.offset = offset;
    p.value = value;
    p.reserved[rindex] = rcode;
    rc = ioctl(ixinfo[index].hboard, S826_IOC_SET_LCS, &p);
    return (rc == 0) ? S826_ERR_OK : S826_ERR_DRIVER;
}

int sys826_readLCS(uint index, uint offset, uint *value, uint rindex, uint rcode)
{
    int rc;
    struct s826_pci_reg p;
    p.offset = offset;
    p.reserved[rindex] = rcode;
    rc = ioctl(ixinfo[index].hboard, S826_IOC_GET_LCS, &p);
    *value = p.value;
    return (rc == 0) ? S826_ERR_OK : S826_ERR_DRIVER;
}
