all: readout.o readout

readout.o: readout.c
	gcc -D_LINUX -Wall -Wextra -DOSTYPE_LINUX -c -no-pie -I sdk_826_linux_3.3.11/demo readout.c 

readout: readout.o
	gcc -D_LINUX readout.o -no-pie -o readout -lm -L sdk_826_linux_3.3.11/demo -l826_64
