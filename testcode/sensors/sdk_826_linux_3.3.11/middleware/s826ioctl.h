/* private.h 
 * Copyright Sensoray Company Inc. 2010
 * generic WDK device driver for Sensoray 826
 * uses KMDF for ease of implementing power management 
 * Contains shared driver/userspace ioctls
 */ 
#ifndef _S826IOCTL_H
#define _S826IOCTL_H

/*Note: IOC_XXX_DIRECT should only be used for very large amounts of data.  
		IOC_BUFFERED is MUCH faster for short commands */

#include <initguid.h>

// Device Interface GUID
// {76ED6AA2-31D7-4681-BCD8-43C6C1609070}
DEFINE_GUID(GUID_S826_INTERFACE, 0x76ed6aa2, 0x31d7, 0x4681, 0xbc, 0xd8, 0x43, 0xc6, 0xc1, 0x60, 0x90, 0x70);

struct s826_getmem {
	DWORDLONG mmap_bar; // mmap'd user space base address register
	// Under 32 bit Windows, mmap_base will be limited to the lowest 32 bits
	// Under 64 bit Windows, mmap_base could be 32 bit or 64 bit depending on the calling process
	//                       mmap_base is guaranted to be 32 bits if the process is 32 bits.
	DWORD idx; /* use idx = 1 for Local Address Space 0*/
	/* idx = 0, local PCI configuration space, for debug driver build only */
};


// extended wait interrupt
struct s826_waitcount {
	DWORD usecs; //timeout (microseconds)
	DWORD mask; // on input sets the mask of events.
	DWORD bWaitAny; // wait for any (or all default)
	DWORD rc; // return code
	DWORD reserved[3]; // unused future use
} s826_waitcount_t;

struct s826_waitcount_q {
	DWORD chan;
    DWORD rc;
	DWORD reserved[6]; // unused future use
} s826_waitcount_q_t;

//DIO wait interrupt
struct s826_waitdio {
	DWORD usecs; //timeout (microseconds)
	DWORD mask[2]; // mask of interrupts to wait for.
	DWORD bWaitAny;  // wait for any (or all default)
	DWORD rc; // return code
	DWORD reserved[3]; // unused, future use
} s826_waitdio_t;

//ADC wait interrupt
struct s826_waitadc {
	DWORD usecs; //timeout (microseconds)
	DWORD mask; // mask of interrupts to wait for.
	DWORD bWaitAny;  // wait for any (or all default)
	DWORD rc; // return code
	DWORD reserved[3];
} s826_waitadc_t;

// wait watchdog
struct s826_waitwd {
	DWORD usecs; //timeout (microseconds)
	DWORD rc; // return code
	DWORD reserved[4]; // unused future use
} s826_waitwd_t;



struct s826_cancel_waitcount {
	DWORD mask; //  mask of events to cancel
	DWORD reserved[3]; // unused, future use
} s826_cancel_waitcount_t;


struct s826_cancel_waitdio {
	DWORD mask[2]; // mask of DIO waits to cancel
	DWORD reserved[3]; // unused, future use
} s826_cancel_waitdio_t;


struct s826_cancel_waitadc {
	DWORD mask; // mask of ADC waits to cancel
	DWORD reserved[3]; // unused, future use
} s826_cancel_waitadc_t;


struct s826_cancel_waitwd {
	DWORD reserved[4]; // unused, future use
} s826_cancel_waitwd_t;

struct s826_version {
	DWORD version; //
	DWORD reserved[3];
};

struct s826_pci_reg {
    DWORD offset;
    DWORD value;
    DWORD reserved[2]; // 
};

// Note: Do not define new ioctl's with  IOCTL_INDEX + 3, IOCTL_INDEX + 2  (old obsolete ioctls)

#define IOCTL_INDEX                     0x0000
#define S826_IOCTL(N)                   CTL_CODE(FILE_DEVICE_UNKNOWN, IOCTL_INDEX + (N), METHOD_BUFFERED, FILE_ANY_ACCESS)



#define S826_IOC_GETMEM               S826_IOCTL(7)       // maps BAR of 826 to user space
#define S826_IOC_WAITCOUNT            S826_IOCTL(8)       // wait for interrupt counter
#define S826_IOC_WAITDIO              S826_IOCTL(9)       // wait for interrupt DIO
#define S826_IOC_WAITADC              S826_IOCTL(14)      // wait for ADC DIO
#define S826_IOC_VERSION              S826_IOCTL(15)

#define S826_IOC_CANCEL_WAITCOUNT     S826_IOCTL(16)      // cancel wait interrupt counter
#define S826_IOC_CANCEL_WAITDIO       S826_IOCTL(17)      // cancel wait DIO
#define S826_IOC_CANCEL_WAITADC       S826_IOCTL(18)      // cancel wait ADC
#define S826_IOC_WAITWD               S826_IOCTL(19)      // wait WD (watchdog)
#define S826_IOC_CANCEL_WAITWD        S826_IOCTL(20)      // cancel wait WD (watchdog)

// internal use only
// offset can not exceed or be equal to LCS size, which is 512 bytes
// offset must always be multiple of DWORD size (4 bytes) and value written will be DWORD
// (if writing 1 byte only, read the register first and do a bitmask on it)
// Table 20-3 in the manual
#define S826_IOC_SET_LCS              S826_IOCTL(21)      // write to local config space space
#define S826_IOC_GET_LCS              S826_IOCTL(22)      // read from local config space space



// PCI configuration (Local, NOT the bridge chip)
// This is Table 20-2 in the manual
#define S826_IOC_SET_LPCI           S826_IOCTL(25)      // write to PCI config space space
#define S826_IOC_GET_LPCI           S826_IOCTL(26)      // read from PCI config space space


// future use
#define S826_IOC_WAITCOUNT_Q        S826_IOCTL(55)     // wait for interrupt counter q

#endif
