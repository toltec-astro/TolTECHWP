
ifeq ($(shell uname -m),x86_64)
LIBBITS = _64
else
LIBBITS =
endif

LBITS := $(shell getconf LONG_BIT)

EXTRA_CFLAGS += -Wno-unused


all: modules s826demo


modules:
	$(MAKE) -C driver $@

clean:
	$(MAKE) -C driver $@
	$(MAKE) -C demo $@

install:
	$(MAKE) -C driver $@

uninstall:
	$(MAKE) -C driver $@

lib:
	$(MAKE) -C middleware $@

lib_install:
	$(MAKE) -C middleware $@






s826demo:
	$(MAKE) -C demo $@
