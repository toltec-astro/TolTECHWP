//////////////////////////////////////////////////////////////////////
// File      : 826drv.c
// Function  : Driver call wrapper for Sensoray 826 for Windows
// Copyright : Copyright Sensoray Company Inc. 2010-2012
//////////////////////////////////////////////////////////////////////

// All functions in this module are visible only to the middleware.
// NEVER EXPOSE THESE FUNCTIONS TO THE USER APPLICATION.


#define _CRT_SECURE_NO_DEPRECATE
#define _CRT_SECURE_NO_WARNINGS     // turn off warnings for sprintf.

#include <windows.h>
#include <WinIoCtl.h>
#include <stdio.h>
#include <stdlib.h>
#include <setupapi.h>
#include <initguid.h>

#include "s826ioctl.h"
#include "platform.h"
#include "826drv.h"
#include "826api.h"


// Constants -----------------------------------------------------

#define VACANT_BOARDINDEX   (~0)UL

// Windows-specific error codes.
#define S826_ERR_GETCLASSDEVS                       (S826_ERR_OSSPECIFIC - 0)
#define S826_ERR_ENUMDEVICEINTERFACES               (S826_ERR_OSSPECIFIC - 1)
#define S826_ERR_GETDEVICEINTERFACEDETAILSIZE       (S826_ERR_OSSPECIFIC - 2)
#define S826_ERR_GETDEVICEINTERFACEDETAILS          (S826_ERR_OSSPECIFIC - 3)



// Types ---------------------------------------------------------

typedef unsigned int uint;

typedef struct IXINFO
{
    HANDLE  hboard;             // Board handle. Set to NULL if board is unregistered.
    HANDLE  hmutex[3];          // Mutex handles. Board lock, eeprom lock, bank lock.
    uint    *baseadr;           // Pointer to board's register space. Set to NULL if board is unregistered.
};

typedef struct IXINFO IXINFO;


// Storage --------------------------------------------------------

static IXINFO   ixinfo[S826_MAX_BOARDS];    // Info for registered boards, maps index to: hboard, hmutex[], baseadr.


int OurDeviceIoControl(HANDLE hDevice, DWORD dwIoControlCode,
                        LPVOID lpInBuffer, DWORD nInBufferSize,
                        LPVOID lpOutBuffer, DWORD nOutBufferSize,
                        LPDWORD lpBytesReturned, int timeout)
{
    HANDLE hevent;
    OVERLAPPED ovlap;
    int rc;

    // Windows timeout is ms, not microseconds
    if (timeout > 0)
        timeout = (timeout / 1000) + 1; // + 1 because otherwise it won't wait at all.  We need at least 1ms


    memset(&ovlap, 0, sizeof(OVERLAPPED));
    hevent = CreateEvent(NULL, FALSE, FALSE, NULL);
    ovlap.hEvent = hevent;
    rc = DeviceIoControl(hDevice, dwIoControlCode, 
                         lpInBuffer, nInBufferSize, 
                         lpOutBuffer, nOutBufferSize,
                         lpBytesReturned, &ovlap);
    if (rc != 0) {
        CloseHandle(hevent);
        return S826_ERR_DRIVER;
    }

    // Need this because GetOverlappedResultEx (with the timeout builtin)
    // is only available for Windows 8.
    if (WaitForSingleObject(hevent, timeout) == WAIT_TIMEOUT) {
        CloseHandle(hevent);
        return S826_ERR_NOTREADY;    
    }

    if (GetOverlappedResult(hDevice, &ovlap, lpBytesReturned, FALSE) == 0) {
        CloseHandle(hevent);
        return S826_ERR_DRIVER;
    }
    CloseHandle(hevent);
    return 0;
}


////////////////////////////////////////////////////////////////////////
// Get driver version.

int sys826_readver(uint index, uint *version)
{
    ULONG               nBytes;
    struct s826_version ver;

    if (OurDeviceIoControl(ixinfo[index].hboard, S826_IOC_VERSION, &ver, sizeof (struct s826_version), &ver, sizeof (struct s826_version), &nBytes, 0) != 0)
        return S826_ERR_DRIVER;

    *version = ver.version;
    return S826_ERR_OK;
}

//////////////////////////////////////////////////////////////////////
// Direct register read/write.
// Caller is responsible for ensuring legal index number and board is open.

uint sys826_readreg(uint index, uint offset)
{
    return *(volatile uint *)(ixinfo[index].baseadr + offset);
}

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

// Create mutex. Returns mutex handle if successful, else 0.
static HANDLE create_lock(uint index, MUTEX_ID mutid)
{
    char mutexname[12];
    sprintf(mutexname, "s826bd%d_%d", index, mutid);
    return CreateMutex(NULL, 0, mutexname);    // Attempt to create a mutux for the board.
}

// Lock board. Returns S826_ERR_OK if successful, else S826_ERR_CANCELLED | S826_ERR_BOARDCLOSED.
int sys826_lock(uint index, MUTEX_ID mutid)
{
    HANDLE h = ixinfo[index].hmutex[mutid];
    if (h != NULL)
        return (WaitForSingleObject(h, INFINITE) == WAIT_OBJECT_0) ? S826_ERR_OK : S826_ERR_CANCELLED;
    else
        return S826_ERR_BOARDCLOSED;
}

// Unlock board.
void sys826_unlock(uint index, MUTEX_ID mutid)
{
    HANDLE h = ixinfo[index].hmutex[mutid];
    if (h != NULL)
        ReleaseMutex(h);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// Print OS-specific diagnostic error messages.

void sys826_print_errmsg(int errcode, uint index)
{
    switch (errcode)
    {
    case S826_ERR_OK:                           break; // success
    case S826_ERR_GETCLASSDEVS:                 printf("SetupDiGetClassDevs failed: %x\n", GetLastError()); break;
    case S826_ERR_GETDEVICEINTERFACEDETAILSIZE: printf("SetupDiGetDeviceInterfaceDetail failed %d\n", GetLastError()); break;
    case S826_ERR_GETDEVICEINTERFACEDETAILS:    printf("Error in SetupDiGetDeviceInterfaceDetail\n"); break;
    default:                                    printf("Unknown error.\n"); break;
    }
}

////////////////////////////////////////////////////////////
// Public system init.

void sys826_init(void)
{
    // not used in windows
}

////////////////////////////////////////////////////////////
// Initialize system storage.
// This must be called when a process attaches to the DLL.

static void sys_init(void)
{
    int i;
    int m;
    for (i = 0; i < S826_MAX_BOARDS; i++)   // no board info available yet
    {
        ixinfo[i].hboard  = NULL;
        ixinfo[i].baseadr = NULL;
        for (m = 0; m < NMUTEXES; m++)
            ixinfo[i].hmutex[m] = NULL;
    }
}


////////////////////////////////////////////////////////////////////////////
// Close all boards.

void sys826_close(void)
{
    uint i;
    uint m;
    IXINFO *pinfo = ixinfo;

    for (i = 0; i < S826_MAX_BOARDS; i++, pinfo++)
    {
        // Note: handles are set to null in sys_init below
        if (pinfo->hboard != NULL)
            CloseHandle(pinfo->hboard);         // Close board.

        for (m = 0; m < NMUTEXES; m++)          // Destroy board's mutexes.
        {
            if (pinfo->hmutex[m] != NULL)
                CloseHandle(pinfo->hmutex[m]);
        }
    }

    sys_init();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Open board using driver GUID instead of hardcoded DOS symbolic link(CreateFile); recommended method in Windows for newer drivers.
// Imports:
//  index - driver-enumerated board number.
// Returns board handle if board exists, else INVALID_HANDLE_VALUE.

static HANDLE OpenBoard(uint index, int *errcode)
{
    HDEVINFO                            hardwareDeviceInfo;
    SP_DEVICE_INTERFACE_DATA            deviceInterfaceData;
    PSP_DEVICE_INTERFACE_DETAIL_DATA    deviceInterfaceDetailData;
    ULONG                               requiredLength  = 0;
    HANDLE                              rtnval          = INVALID_HANDLE_VALUE;  // default.

    // Open a handle to the device interface information set of all 826s: no enumerator (global), only devices present (function class devices).
    hardwareDeviceInfo = SetupDiGetClassDevs((LPGUID)&GUID_S826_INTERFACE, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	if (hardwareDeviceInfo == INVALID_HANDLE_VALUE)
        *errcode = S826_ERR_GETCLASSDEVS;
    else
    {
        deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
	    if (SetupDiEnumDeviceInterfaces(hardwareDeviceInfo, 0, (LPGUID)&GUID_S826_INTERFACE, index, &deviceInterfaceData))   // If more boards (don't care about specific PDOs) ...
        {
            // Allocate a function class device data structure to receive the information about this particular device.
            // First find out required length of the buffer: probing so no output buf & buflen=0, not interested in the specific dev-node.
            SetupDiGetDeviceInterfaceDetail(hardwareDeviceInfo, &deviceInterfaceData, NULL, 0, &requiredLength, NULL);
	        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
                *errcode = S826_ERR_GETDEVICEINTERFACEDETAILSIZE;
            else if ((deviceInterfaceDetailData = malloc(requiredLength)) == NULL)
                *errcode = S826_ERR_MALLOC;
            else
            {
                deviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);
                if (! SetupDiGetDeviceInterfaceDetail(hardwareDeviceInfo, &deviceInterfaceData, deviceInterfaceDetailData, requiredLength, &requiredLength, NULL))
                    *errcode = S826_ERR_GETDEVICEINTERFACEDETAILS;
                else
                    rtnval = CreateFile(deviceInterfaceDetailData->DevicePath, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, NULL);  // return board handle

                free(deviceInterfaceDetailData);
            }
        }

	    SetupDiDestroyDeviceInfoList(hardwareDeviceInfo);   // Close the handle to the device interface information set.
    }

    return rtnval;
}

///////////////////////////////////////////////////////////////////////
// Get board's base memory address.

static int GetBoardAddress(HANDLE hboard, PVOID *addr)
{
    struct s826_getmem  param;
    ULONG               nBytes;
    int                 errnum = S826_ERR_OK;

    // Get board's base memory address.
    param.idx = 1;
    if (OurDeviceIoControl(hboard, S826_IOC_GETMEM, &param, sizeof (struct s826_getmem), &param, sizeof (struct s826_getmem), &nBytes, 0) != 0)
        errnum = S826_ERR_DRIVER;
	else if ((PVOID)param.mmap_bar == NULL)              // Abort if we failed to get board's memory address.
        errnum = S826_ERR_MEMORYMAP;
    else
        *addr = (uint *)param.mmap_bar;    // Cache board's memory address.

    return errnum;
}

////////////////////////////////////////////////////////////////////////////////////////
// Open all boards. This must be called once to open access to all 826 boards.
// Returns:
//  if success - board count (positive value or zero).
//  if failed  - error code (negative value).

int sys826_open()
{
    uint    b;
    uint    m;
    int     nboards = 0;            // no boards detected at any board index
    int     errnum  = S826_ERR_OK;  // no errors so far

    for (b = 0; b < S826_MAX_BOARDS; b++)    // For each driver-enumerated board (in range 0 to boardcount-1) ...
    {
        // Open board and get its handle.
        ixinfo[b].hboard = OpenBoard(b, &errnum);
        if (ixinfo[b].hboard == INVALID_HANDLE_VALUE)   // Done if no more boards detected
            break;

        // Get board's base memory address.
        errnum = GetBoardAddress(ixinfo[b].hboard, &ixinfo[b].baseadr);
        if (errnum != S826_ERR_OK)
            break;

        // Create board mutexes.
        for (m = 0; m < NMUTEXES; m++)
        {
            ixinfo[b].hmutex[m] = create_lock(b, m);    // Attempt to create mutex.
            if (ixinfo[b].hmutex[m] == NULL)
            {
                errnum = S826_ERR_CREATEMUTEX;          // Abort if failed.
                break;
            }
        }

        if (errnum != S826_ERR_OK)
            break;

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
    DWORD nBytes;
    struct s826_waitcount p;
    int rc; 
    p.usecs     = timeout;      // microseconds
    p.mask      = intlist;      // interrupt flag(s) to wait for
    p.bWaitAny  = 1;            // wait all
    rc = OurDeviceIoControl(ixinfo[index].hboard, S826_IOC_WAITCOUNT, &p, sizeof (struct s826_waitcount), &p, sizeof (struct s826_waitcount), &nBytes, timeout);
    if (rc == 0)
        rc = p.rc;
    return rc;
}

///////////////////////////////////////////////////////////////////////////////////////////
// Block until any/all specified DIO channels have acquired data.
// Caller is responsible for ensuring legal board number.

int sys826_wait_dio(uint index, uint *intlist, uint waitany, uint timeout)
{
    DWORD nBytes;
    struct s826_waitdio p;
    int rc;
  
    p.usecs    = timeout;               // microseconds
    p.mask[0]  = intlist[0];            // interrupt flag(s) to wait for
	p.mask[1]  = intlist[1];
	p.bWaitAny = (waitany != 0) ? 1 : 0;
    rc = OurDeviceIoControl(ixinfo[index].hboard, S826_IOC_WAITDIO, &p, sizeof (struct s826_waitdio), &p, sizeof (struct s826_waitdio), &nBytes, timeout);
    if (rc == 0)
        rc = p.rc;
    return rc;
}

///////////////////////////////////////////////////////////////////////////////////////////
// Block until any/all specified ADC channels have acquired data.
// Caller is responsible for ensuring legal board number.

int sys826_wait_adc(uint index, uint intlist, uint waitany, uint timeout)
{
    DWORD nBytes;
    struct s826_waitadc p;
    int rc;
    p.usecs     = timeout;      // microseconds
    p.mask      = intlist;      // interrupt flag(s) to wait for
	p.bWaitAny  = (waitany != 0) ? 1 : 0;
    rc = OurDeviceIoControl(ixinfo[index].hboard, S826_IOC_WAITADC, &p, sizeof (struct s826_waitadc), &p, sizeof (struct s826_waitadc), &nBytes, timeout);
    if (rc == 0)
        rc = p.rc;
    return rc;
}

///////////////////////////////////////////////////////////////////////////////////////////
// Block until watchdog state0 timeout.
// Caller is responsible for ensuring legal board number.

int sys826_wait_watchdog(uint index, uint timeout)
{
    DWORD nBytes;
    struct s826_waitwd p;
    int rc;
    p.usecs = timeout;      // microseconds
    rc = OurDeviceIoControl(ixinfo[index].hboard, S826_IOC_WAITWD, &p, sizeof (struct s826_waitwd), &p, sizeof (struct s826_waitwd), &nBytes, timeout);
    if (rc == 0)
        rc = p.rc;
    return rc;
}

///////////////////////////////////////////////////////////////////////////////////////
// Cancel blocking calls.

int sys826_waitcancel_adc(uint index, uint slotlist)
{
    DWORD nBytes;
    struct s826_cancel_waitadc p;
    BOOL success;
    p.mask = slotlist;
    success = OurDeviceIoControl(ixinfo[index].hboard, S826_IOC_CANCEL_WAITADC, &p, sizeof(p), &p, sizeof(p), &nBytes, 1);
    return success? S826_ERR_OK : S826_ERR_DRIVER;
}

int sys826_waitcancel_counter(uint index, uint chan)
{
    DWORD nBytes;
    struct s826_cancel_waitcount p;
    BOOL success;
    p.mask = (1 << chan);
    success = OurDeviceIoControl(ixinfo[index].hboard, S826_IOC_CANCEL_WAITCOUNT, &p, sizeof(p), &p, sizeof(p), &nBytes, 1);
    return success? S826_ERR_OK : S826_ERR_DRIVER;
}

int  sys826_waitcancel_dio(uint index, const uint chanlist[2])
{
    DWORD nBytes;
    struct s826_cancel_waitdio p;
    BOOL success;
    p.mask[0] = chanlist[0];
    p.mask[1] = chanlist[1];
    success = OurDeviceIoControl(ixinfo[index].hboard, S826_IOC_CANCEL_WAITDIO, &p, sizeof(p), &p, sizeof(p), &nBytes, 1);
    return success? S826_ERR_OK : S826_ERR_DRIVER;
}

int sys826_waitcancel_watchdog(uint index)
{
    DWORD nBytes;
    struct s826_cancel_waitwd p;
    BOOL success;
    memset(&p, 0, sizeof(p));
    success = OurDeviceIoControl(ixinfo[index].hboard, S826_IOC_CANCEL_WAITWD, &p, sizeof(p), &p, sizeof(p), &nBytes, 1);
    return success? S826_ERR_OK : S826_ERR_DRIVER;
}

////////////////////////////////////////////////////////
// MCR/LCS access -- for internal Sensoray use only.


int sys826_writeLCS(uint index, uint offset, uint value, uint rindex, uint rcode)
{
    DWORD nBytes;
    struct s826_pci_reg p;
    BOOL success;
    p.offset = offset;
    p.value  = value;
    p.reserved[rindex] = rcode;
    success = OurDeviceIoControl(ixinfo[index].hboard, S826_IOC_SET_LCS, &p, sizeof(p), &p, sizeof(p), &nBytes, 0);
    return success? S826_ERR_OK : S826_ERR_DRIVER;
}

int sys826_readLCS(uint index, uint offset, uint *value, uint rindex, uint rcode)
{
    DWORD nBytes;
    struct s826_pci_reg p;
    BOOL success;
    p.offset = offset;
    p.reserved[rindex] = rcode;
    success = OurDeviceIoControl(ixinfo[index].hboard, S826_IOC_GET_LCS, &p, sizeof(p), &p, sizeof(p), &nBytes, 0);
    *value = p.value;
    return success? S826_ERR_OK : S826_ERR_DRIVER;
}

////////////////////////////////////////////////////////
// DLL entry point.

BOOL APIENTRY DllMain(HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
	{
		case DLL_PROCESS_ATTACH:
            sys_init();         // init private middleware storage
			break;
		case DLL_THREAD_ATTACH:
		case DLL_THREAD_DETACH:
		case DLL_PROCESS_DETACH:
			break;
    }
    return TRUE;
}
