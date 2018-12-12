#include "../vend/ini/ini.h"
#include "../vend/ini/sdk_826_linux_3.3.11/driver/826api.h"
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdlib.h> 

int main(int argc, char **argv){
    
    if( argc < 3 ) {
        printf("Use: readout datadtime sleepdtime duration\n");
        printf("     datadtime = intervals how often data is read (us)\n");
        printf("     sleepdtime = how long computer sleeps between reads (us)\n");
        printf("     duration = how long experiment is (seconds)\n");
        exit(0);
    }    
}