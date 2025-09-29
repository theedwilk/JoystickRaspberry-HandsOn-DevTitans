CC = gcc
CFLAGS = -Wall -g
test: main.o joy_driver.o
	   $(CC) $(CFLAGS) main.o joy_driver.o -g -o test

%.o: %.c *.h
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -f *.o test *.ko
