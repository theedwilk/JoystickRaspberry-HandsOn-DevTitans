# Kbuild part of the Makefile
# This specifies the kernel module to build
obj-m += joy_driver_module.o

# Convenience targets for local compilation
ifeq ($(KERNELRELEASE),)

KDIR ?= /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

all:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

.PHONY: all clean

endif
