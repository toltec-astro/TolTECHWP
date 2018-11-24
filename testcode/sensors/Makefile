all: housekeeping.o housekeeping

housekeeping.o: housekeeping.c
	gcc -D_LINUX -Wall -Wextra -DOSTYPE_LINUX -c -no-pie -I sdk_826_linux_3.3.11/demo housekeeping.c 

housekeeping: housekeeping.o
	gcc -D_LINUX housekeeping.o -no-pie -o housekeeping -lm -L sdk_826_linux_3.3.11/demo -l826_64
