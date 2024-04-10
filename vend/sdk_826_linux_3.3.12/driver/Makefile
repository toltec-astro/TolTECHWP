KDIR = /lib/modules/$(shell uname -r)/build


space :=
space +=

ifeq ($(KERNELRELEASE),)
  SUBDIR = $(shell pwd)
  ifeq ($(space),$(findstring $(space),$(SUBDIR)))
    $(error Current path contains spaces! Please fix. ***)
  endif
else
  SUBDIR = $(SUBDIRS)
endif

all: modules

obj-m := s826.o

EXTRA_CFLAGS += -Wno-unused -DS826_DRIVER_BUILD


modules:
	echo $(.INCLUDE_DIRS)
	$(MAKE) -C $(KDIR) M=$(SUBDIR) $@

clean:
	$(MAKE) -C $(KDIR) M=$(SUBDIR) $@

install: modules_install udev_install unload
	sudo modprobe s826

load:
	sudo modprobe s826

unload:
	sudo modprobe -r -q s826


root_check:
	@id -u | grep "^0" || { \
		echo "Try again as root or use 'sudo make install'"; false; \
	}

udev_install:
	sudo cp 10-local-826.rules /etc/udev/rules.d/10-local-826.rules


modules_install: root_check
	sudo $(MAKE) -C $(KDIR) M=$(SUBDIR) $@
	sudo depmod $(shell uname -r)

uninstall: root_check
	sudo modprobe -q -r s826
	sudo rm -vf /lib/modules/$(shell uname -r)/extra/s826.ko
	sodu depmod $(shell uname -r)
