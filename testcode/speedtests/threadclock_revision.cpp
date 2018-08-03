/*  THREADCLOCK

    A program to experiment with the time a thread sleeps.

    to compile:
        g++ -o threadclock threadclock.cpp

 */

//**** Setup
#include<iostream>
#include<stdio.h>
#include<sys/time.h>
#include<time.h>

//#define N 30
#define DUS 30

//**** Main
int main(int argc, char** argv){
  //**** Gives start time for code  (Central time)
  time_t rawtime;
  struct tm * ptm;
  time ( &rawtime );
  ptm = gmtime ( &rawtime );
  printf("Start Time: %2d:%02d\n", (ptm->tm_hour-5)%24, ptm->tm_min);
  
  int N; // Number of counts to do
  struct timeval tval;
  struct timespec treq;
  int i,j,t0;
  int n01=0, n03=0, n1=0, n3=0, n10=0, n30=0, n100=0;
  long tmin=100000,tmax=0,tdiff,tsum=0;

  //**** Get command line arguments
  //for(i=0;i<argc;i++){
  //  printf("%d %s\n", i, argv[i]);
  //}
  if(argc>1){
    N = atoi(argv[1])*1000000/DUS;
  } else {
    printf("Usage: threadclock seconds\n  Where seconds is the duration the program should run\n");
    exit(1);
  }
  //int tvals[N];
  //**** Simple tests
  //printf("Hello World\n");
  //clock_gettime(CLOCK_MONOTONIC_RAW, &tspec); // does not work on mac
  gettimeofday(&tval, NULL);
  printf("sec = %d    usec = %d\n", int(tval.tv_sec), tval.tv_usec);
  t0=tval.tv_usec;
  //**** Record N time events
  treq.tv_sec = 0;
  treq.tv_nsec=DUS*1000;
  int old_tval = tval.tv_usec, new_tval;
  for(i=0;i<N;i++){
    gettimeofday(&tval, NULL);
    //**** get timestamp for reading
    new_tval = tval.tv_usec;
    //**** compare this timestamp to the one from the last loop
    tdiff = new_tval - old_tval;
      //**** This does the statistics -- Note: stats are cumulative
    if(tdiff<0) tdiff+=1000000;
    tsum+=tdiff;
    if(tdiff>100) n01++;
    if(tdiff>300) n03++;
    if(tdiff>1000) n1++;
    if(tdiff>3000) n3++;
    if(tdiff>10000) n10++;
    if(tdiff>30000) n30++;
    if(tdiff>100000) n100++;
    if(tmin>tdiff) tmin=tdiff;
    if(tmax<tdiff) tmax=tdiff;
    //**** Reassign new timestamp to be the old timestamp for the next loop
    old_tval = new_tval;
    //**** This below is to account for the loop around at 999,999usec and loop after set time
    tdiff = 0;
    while(tdiff < DUS && old_tval < tval.tv_usec+1){
      tdiff = tval.tv_usec - old_tval;
      gettimeofday(&tval, NULL);
    }
  }
  printf("Average = %.1fus  Min = %dus  Max = %dus  Tot = %.1fs\n",
	 float(tsum)/float(N), int(tmin), int(tmax),
	 float(tsum)/float(1000000));
  printf("%d >0.1ms   %d >0.3ms   %d >1ms   %d >3ms   %d >10ms   %d >30ms   %d >100ms\n",
	 n01, n03, n1, n3, n10, n30, n100);
  //**** Return
  return 0;
}
