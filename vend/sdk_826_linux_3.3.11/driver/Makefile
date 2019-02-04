KDIR = /lib/modules/$(shell uname -r)/build

ifeq ($(KERNELRELEASE),)
SUBDIR = $(shell pwd)
else 
SUBDIR = $(SUBDIRS)
endif

all: modules

obj-m := s826.o

EXTRA_CFLAGS += -Wno-unused -DS826_DRIVER_BUILD

modules:
	$(MAKE) -C $(KDIR) SUBDIRS=$(SUBDIR) $@

clean:
	$(MAKE) -C $(KDIR) SUBDIRS=$(SUBDIR) $@

install: modules_install udev_install unload
	modprobe s826

load:
	modprobe s826

unload:
	modprobe -r -q s826


root_check:
	@id -u | grep "^0" || { \
		echo "Try again as root or use 'sudo make install'"; false; \
	}

udev_install:
	sudo cp 10-local-826.rules /etc/udev/rules.d/10-local-826.rules

modules_install: root_check
	$(MAKE) -C $(KDIR) SUBDIRS=$(SUBDIR) $@
	depmod $(shell uname -r)

uninstall: root_check
	-modprobe -q -r s826
	rm -vf /lib/modules/$(shell uname -r)/extra/s826.ko
	depmod $(shell uname -r)
