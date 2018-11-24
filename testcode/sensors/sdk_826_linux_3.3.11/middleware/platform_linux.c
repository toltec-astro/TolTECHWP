// Copyright Sensoray Company Inc. 2011
// platform specific code
// critical section and sleep threads
//   ini file reading
#include <string.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <linux/types.h>
#include <stdint.h>
#include "assert.h"
#include "platform.h"


/** sleep for x milliseconds
 *  @param time time in milliseconds to sleep the thread
 *  @return none
 */
void thread_sleep_msec( int time)
{
	usleep( time*1000);
	return;
}

/** returns elapsed_time from starttime in ms 
 *  @param starttime time to compare from
 *  @return time difference
 */
uint64_t elapsed_time( uint64_t starttime) 
{
	struct timeval tv;
	uint64_t diff;
	gettimeofday(&tv,NULL);
    //printf("start time %lld %lld\n", starttime,(tv.tv_sec*1000) + (tv.tv_usec/1000) );
	diff = (tv.tv_sec *1000) + (tv.tv_usec/1000) - starttime;
    //    printf("diff %d \n",diff);//tm.tms_stime, tm.tms_utime);//starttime); // diff);
	return diff;

}

/** retrive current time
 *  @ret
*/
uint64_t current_time()
{
    uint64_t tval;
	struct timeval tv;
    gettimeofday(&tv,NULL);
    tval = (tv.tv_sec * 1000) + (tv.tv_usec/1000);

	return tval;
}
