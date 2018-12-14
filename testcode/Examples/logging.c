/* This contains several functions for passing log messages from C back to
 * python.  The user should only need to call one of the functions:
 * logDebug(), logInfo(), logWarning(), logError(), logCritical()
 *
 * Derived from simpleclient.c by Marc Berthoud
 */

#include "hawc.h"
#include<sys/socket.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<string.h>

int logportnum;
char *loghost;

void setLogPort(char *host, int port) {
  loghost = host;
  logportnum = port;
}

int logbase(char *level, char *source, char *message){
   /* This is the base logging function.  The user should not need to call this
    * directly. */

   int bytesSent;     /* bytes sent by send() */
   int socket_desc;   /* socket descriptor */
   char buffer[1024]; /* buffer for level + message length */
   struct sockaddr_in address; /* socket address */
   int num;           /* will keep track of number of characters allowed for
                         message =~ 1024 - len(level) -len(source) */

   /* Check for uninitialized port */
   if (!logportnum)
     setLogPort("127.0.0.1", 50747);
   
   /* Open Connection */
   /* make socket */
   socket_desc = socket(PF_INET,SOCK_STREAM,0);
   if (socket_desc < 0) /* failure to open socket */
      return(1);

   /* make receiving address */
   address.sin_family = AF_INET;
   address.sin_port   = htons(logportnum);
   inet_pton(AF_INET,loghost,&address.sin_addr); /* write to local host */
   memset(&(address.sin_zero),'\0',0); /* set zero rest of struct */
   /*printf("Remote Address=%s Port=%d\n",inet_ntoa(address.sin_addr),
     ntohs(address.sin_port));*/

   /* connect */
   if (connect(socket_desc,(struct sockaddr *)&address,sizeof(address)) != 0)
      return(1); /* Connection Failed */

   /* Construct message */
   strcpy(buffer,level);
   strcat(buffer,"\thawc.pipe.step.libhawc."); /* step.libhawc. is root for all sources */
   num = 1022 - strlen(buffer); /* -2 for one tab and a \0 in length */
   strncat(buffer,source,num);
   strcat(buffer,"\t");
   num = 1023 - strlen(buffer); /* -1 for \0 in length */
   strncat(buffer,message,num);

   /* Send the message */
   bytesSent = send(socket_desc,buffer,strlen(buffer),0);
   /* printf("Sent %d bytes\n",bytesSent); */

   /* Close Connection */
   close(socket_desc);
   return 0;
}

void logDebug(char *source, char *message){
   /* send a message at debug level */
   int err;

   err = logbase("DEBUG", source, message);
}

void logInfo(char *source, char *message){
   /* send a message at info level */
   int err;

   err = logbase("INFO", source, message);
}

void logWarning(char *source, char *message){
   /* send a message at warning level */
   int err;

   err = logbase("WARN", source, message);
}

void logError(char *source, char *message){
   /* send a message at error level */
   int err;

   err = logbase("ERR", source, message);
}

void logCritical(char *source, char *message){
   /* send a message at critical level */
   int err;

   err = logbase("CRIT", source, message);
}
