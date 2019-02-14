/* 
 * usage: udpclient <host> <port>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <time.h> 
#include <math.h> 

#define DATA_SIZE 8192
#define BUFFER_SIZE (DATA_SIZE)

double getTime() {
    struct timespec tspec;
    struct tm *tim;
    clock_gettime(CLOCK_REALTIME, &tspec);
    tim = gmtime(&tspec.tv_sec);
    double t = tim->tm_hour*3600.0 + tim->tm_min*60.0 + tim->tm_sec +
        (tspec.tv_nsec/1.0e+9);
    t = tspec.tv_sec + (tspec.tv_nsec/1.0e+9);
    return t;
}

int main(int argc, char **argv) {
    int sockfd, portno, n;
    char buf[BUFFER_SIZE];
    struct timeval tv;
    socklen_t serverlen;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    char *hostname;
    char * ptr;
    int count = 0, status = 0;
    double t;
    int sec, msec;
    double f_fpga = 256.0*1E6;
    double accum_len = pow(2, 19);
    double data_rate = f_fpga/accum_len;

    // get args
    if (argc != 3) {
        fprintf(stderr,"usage: %s <hostname> <port>\n", argv[0]);
        exit(0);
    }
    hostname = argv[1];
    portno = atoi(argv[2]);

    // init socket
    if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        exit(-1);
    }
    tv.tv_sec = 1;
    tv.tv_usec = 100000;
    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0) {
        perror("setsockopt snd");
    }

    // init server address
    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", hostname);
        exit(0);
    }
    // what is this below
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);
    serverlen = sizeof(serveraddr);

    // init data
    int data_len = BUFFER_SIZE/sizeof(int);
    int data[data_len]; // <-- why int?
    for(int j=0; j < data_len; j++) {
        data[j] = j; // <-- store integers
    }

    // go
    printf("udpsend to %s %d\n", hostname, portno);
    printf("%d %d %lf %lf\n", BUFFER_SIZE, DATA_SIZE, data_rate, 1/data_rate);

    while(1) {

        // get time
        t = getTime();
        sec = (int)t;
        msec = (int)((t-sec)*f_fpga);

        // put data in buffer
        for(int j=0; j<data_len; j++) {
            ptr = (char *) &data[j];
            for(int i=0; i<4; i++) {
                buf[j*4+i] = ptr[3-i];
            }
        }

        // put time in buffer
        ptr = (char *) &sec;
        for(int i=0; i<4; i++) {
            buf[BUFFER_SIZE-16+i] = ptr[3-i];
        }
        ptr = (char *) &msec;
        for(int i=0; i<4; i++) {
            buf[BUFFER_SIZE-12+i] = ptr[3-i];
        }

        // put count in buffer
        ptr = (char *) &count;
        for(int i=0; i<4; i++) {
            buf[BUFFER_SIZE-8+i] = ptr[3-i];
        }

        // put status in buffer
        ptr = (char *) &status;
        for(int i=0; i<4; i++) {
            buf[BUFFER_SIZE-4+i] = ptr[3-i];
        }

        // send
        if((n = sendto(sockfd, buf, BUFFER_SIZE, 0, (struct sockaddr *)&serveraddr, serverlen)) < 0) {
            perror("sendto");
        }

        // increment count
        count++;

        // try to simulate 488Hz
        usleep(1000000*1./data_rate/2.);
    }

    close(sockfd);
}
