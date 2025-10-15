CC = gcc
CFLAGS = -Wall -g
test: main.o joy_driver.o
	   $(CC) $(CFLAGS) main.o joy_driver.o -g -o test

%.o: %.c *.h
	$(CC) $(CFLAGS) -c $< -o $@
module:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

joy_driver_module.o: joy_driver_module.c

clean:
	rm -f *.o test *.ko

# Kernel module build rules
obj-m += joy_driver_module.o
