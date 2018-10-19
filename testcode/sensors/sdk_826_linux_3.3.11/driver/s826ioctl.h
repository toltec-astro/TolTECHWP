#ifndef S826IO_DEFINED_12345
#define S826IO_DEFINED_12345
#include <linux/types.h>
struct s826_getmem {
	__u64 bar;
	int idx;
};

struct s826_regparm {
	unsigned int off; // offset
	unsigned int val; // value
};


struct s826_waitwd {
	int usecs; // microseconds to wait
	int rc;
	int reserved[4];
};

struct s826_waitcount {
	int usecs; // microseconds to wait
	int mask;
	int bWaitAny;
	int rc;
	int reserved[3];
};

struct s826_waitdio {
	int usecs; // microseconds to wait
	int mask[2];
	int bWaitAny;
	int rc;
	int reserved[3];
};

struct s826_waitadc {
	int usecs; // microseconds to wait
	int mask;
	int bWaitAny;
	int rc;
	int reserved[3];
};

struct s826_cancel_waitwd {
	int reserved[4]; // unused, future use
} s826_cancel_waitwd_t;


struct s826_cancel_waitcount {
	int mask; //  mask of events to cancel
	int reserved[3]; // unused, future use
} s826_cancel_waitcount_t;


struct s826_cancel_waitdio {
	int mask[2]; // mask of DIO waits to cancel
	int reserved[3]; // unused, future use
} s826_cancel_waitdio_t;


struct s826_cancel_waitadc {
	int mask; // mask of ADC waits to cancel
	int reserved[3]; // unused, future use
} s826_cancel_waitadc_t;

struct s826_version {
	int version;
	int reserved[3];
} s826_version_t;

struct s826_pci_reg {
    int offset;
    int value;
    int reserved[2]; // 
} s826_pci_reg_t;


#define S826_IOC_READREG          (0x2000)
#define S826_IOC_WRITEREG         (0x2001)
#define S826_IOC_GETMEM             (0x2003)
#define S826_IOC_WAITCOUNT          (0x2004)
#define S826_IOC_WAITADC            (0x2005)
#define S826_IOC_WAITDIO            (0x2006)
#define S826_IOC_CANCEL_WAITCOUNT   (0x2007)
#define S826_IOC_CANCEL_WAITADC     (0x2008)
#define S826_IOC_CANCEL_WAITDIO     (0x2009)
#define S826_IOC_VERSION            (0x200a)
#define S826_IOC_WAITWD             (0x200b)
#define S826_IOC_CANCEL_WAITWD      (0x200c)
// internal use only
// offset can not exceed or be equal to LCS size, which is 512 bytes
// offset must always be multiple of DWORD size (4 bytes) and value written will be DWORD
// (if writing 1 byte only, read the register first and do a bitmask on it)
// Table 20-3 in the manual
#define S826_IOC_SET_LCS            (0x200d)      // write to local config space space
#define S826_IOC_GET_LCS            (0x200e)      // read from local config space space


#endif

