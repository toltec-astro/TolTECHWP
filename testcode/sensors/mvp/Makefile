all: onesensor.o onesensor

onesensor.o: onesensor.c
	gcc -D_LINUX -Wall -Wextra -DOSTYPE_LINUX -c -no-pie -I ../sdk_826_linux_3.3.11/demo onesensor.c 

onesensor: onesensor.o
	gcc -D_LINUX onesensor.o -no-pie -o onesensor -lm -L ../sdk_826_linux_3.3.11/demo -l826_64
