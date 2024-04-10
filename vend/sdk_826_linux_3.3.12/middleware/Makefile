CC=gcc
CFLAGS+=-D_LINUX -Wno-unused-parameter
CFLAGS_SO+=-D_LINUX -fPIC -Wno-unused-parameter



all: lib826.a lib826_64.a lib826.so lib826_64.so
lib: lib826.a lib826_64.a lib826.so lib826_64.so

lib_install:
	cp lib826.so /usr/local/lib/lib826.so
	cp lib826_64.so /usr/local/lib/lib826_64.so

clean:
	rm -rf *.o *.a *.so

lib826.so: 826lib_so.o 826drv_so.o platform_so.o
	$(CC) $(CFLAGS_SO) -m32 -shared -o lib826.so 826lib_so.o 826drv_so.o platform_so.o

lib826_64.so: 826lib_64_so.o 826drv_64_so.o platform_64_so.o
	$(CC) $(CFLAGS_SO) -m64 -shared -o lib826_64.so 826lib_64_so.o 826drv_64_so.o platform_64_so.o

lib826.a: 826lib.o 826drv.o platform.o
	$(AR) r lib826.a 826lib.o 826drv.o platform.o

lib826_64.a: 826lib_64.o 826drv_64.o platform_64.o
	$(AR) r lib826_64.a 826lib_64.o 826drv_64.o platform_64.o

826lib.o: 826lib.c 826api.h
	$(CC) $(CFLAGS) -m32 -c 826lib.c -o 826lib.o

826lib_64.o: 826lib.c 826api.h
	$(CC) $(CFLAGS) -m64 -c 826lib.c -o 826lib_64.o

826drv.o: 826drv_linux.c 826drv.h s826ioctl_linux.h
	$(CC) $(CFLAGS) -m32 -c 826drv_linux.c -o 826drv.o

826drv_64.o: 826drv_linux.c 826drv.h s826ioctl_linux.h
	$(CC) $(CFLAGS) -m64 -c 826drv_linux.c -o 826drv_64.o

platform.o: platform_linux.c platform.h
	$(CC) $(CFLAGS) -c -m32 platform_linux.c -o platform.o

platform_64.o: platform_linux.c platform.h
	$(CC) $(CFLAGS) -c -m64 platform_linux.c -o platform_64.o




826lib_so.o: 826lib.c 826api.h
	$(CC) $(CFLAGS_SO) -m32 -Wall -Wextra -c 826lib.c -o 826lib_so.o

826lib_64_so.o: 826lib.c 826api.h
	$(CC) $(CFLAGS_SO) -m64 -Wall -Wextra -c 826lib.c -o 826lib_64_so.o

826drv_so.o: 826drv_linux.c 826drv.h s826ioctl_linux.h
	$(CC) $(CFLAGS_SO) -m32 -Wall -Wextra -c 826drv_linux.c -o 826drv_so.o

826drv_64_so.o: 826drv_linux.c 826drv.h s826ioctl_linux.h
	$(CC) $(CFLAGS_SO) -m64 -Wall -Wextra -c 826drv_linux.c -o 826drv_64_so.o

platform_so.o: platform_linux.c platform.h
	$(CC) $(CFLAGS_SO) -Wall -Wextra -c -m32 platform_linux.c -o platform_so.o

platform_64_so.o: platform_linux.c platform.h
	$(CC) $(CFLAGS_SO) -Wall -Wextra -c -m64 platform_linux.c -o platform_64_so.o


