// Define needed for reading version
#define DOTREV(N) ((N) >> 24) & 0xFF, ((N) >> 16) & 0xFF, (N) & 0xFFFF

#include "826api.h"
#include <stdio.h>
#include <unistd.h>

int main(){

  int id;  // board ID (determined by switch settings on board)

  // Open the 826 API and list all boards ---------------
  int flags = S826_SystemOpen();
  if (flags < 0)
    printf("S826_SystemOpen returned error code %d", flags);
  else if (flags == 0)
    printf("No boards were detected");
  else {
    printf("Boards were detected with these IDs:");
    for (id = 0; id < 16; id++) {
      if (flags & (1 << id)) printf(" %d", id);
    }
  }
  printf("\n");
  // Set board id to 0
  id = 0;
  // Version read
  uint api, drv, bd, fpga;
  int errcode = S826_VersionRead(id, &api, &drv, &bd, &fpga); // Read version info.
  if (errcode == S826_ERR_OK) {
    // If no errors then
    printf("API version %d.%d.%d\n", DOTREV(api));
    // display info.
    printf("Driver version %d\n", DOTREV(drv));
    printf("Board version %d\n", bd);
    printf("FPGA version %d\n", DOTREV(fpga));
  } else
    printf(" S826_VersionRead returned error code %d", errcode);
  // Read time stamp
  uint t0, t1;
  S826_TimestampRead(id, &t0);
  sleep(1);
  S826_TimestampRead(id, &t1);
  printf("Slept from %d to %d, diff is %d\n", t0, t1, t1-t0);
  // Blink all channels
  uint dios[] = { // Specify DIOs that are to be turned on (driven to 0 V):
    0x00FFFFFF,   // DIO 0-23
    0x00FFFFFF    // DIO 24-47
  };
  int chan = 1; // second channel
  uint data[2] = {0,0}; // data to read
  flags = S826_DioOutputWrite(id, dios, S826_BITWRITE); // Turn on all DIOs. (i.e. all to GND)
  if (flags < 0) printf("S826_DioOutputWrite returned error code %d", flags);
  sleep(2);
  flags = S826_DioInputRead(id, data);
  if (flags < 0) printf("S826_DioOutputWrite returned error code %d", flags);
  printf("Digital read data = %d, %d\n", data[0], data[1]);
  flags = S826_DioOutputWrite(id, dios, S826_BITCLR); // Turn off all DIOs. (i.e. all to 5V with 10k pullup)
  if (flags < 0) printf("S826_DioOutputWrite returned error code %d", flags);
  sleep(2);
  // Blink one channel
  dios[0] = 1 << chan;
  dios[1] = 0;
  flags = S826_DioOutputWrite(id, dios, S826_BITSET); // Turn on all DIOs. (i.e. all to GND)
  if (flags < 0) printf("S826_DioOutputWrite returned error code %d", flags);
  sleep(2);
  flags = S826_DioInputRead(id, data);
  if (flags < 0) printf("S826_DioOutputWrite returned error code %d", flags);
  printf("Digital read data = %d, %d\n", data[0], data[1]);
  flags = S826_DioOutputWrite(id, dios, S826_BITCLR); // Turn off all DIOs. (i.e. all to 5V with 10k pullup)
  if (flags < 0) printf("S826_DioOutputWrite returned error code %d", flags);
  
  // Close system
  S826_SystemClose();
}

// gcc -D_LINUX -Wall -Wextra -DOSTYPE_LINUX -c  -I /home/polboss/toltec/sdk_826_linux_3.3.10/demo tst826.c

// gcc -D_LINUX tst826.o -o tst826 -lm -L /home/polboss/toltec/sdk_826_linux_3.3.10/demo -l826_64
