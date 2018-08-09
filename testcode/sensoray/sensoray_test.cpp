//
//  sensoray_test.cpp
//  
//
//  Created by Eric Van Camp on 8/9/18.
//

#include "stdio.h"
#include "826api.h"
int main() {
  int id;     // board ID
  int flags = S826_SystemOpen();
  if (flags < 0)  {
    printf("S826_systemOpen returned error code %d", flags);
  }  else if (flags == 0)  {
    printf("No boards were detected");
  }  else  {
    printf("boards were detected with these IDs:");
    for (id = 0; id < 16; id++) {
      if (flags & (1 << id))  {
        printf(" %d", id);
      }
    }
  }
}

int S826_SystemOpen(void)  {
    
}
