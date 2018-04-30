SOURCES = mainclient.c client_socket.c 
OBJS = $(SOURCES:.c=.o)
IMP = $(SOURCES:.c=.i)

INCLUDES = ../

#CC = arm-linux-gnueabihf-gcc
CC = gcc
DEBUG = -pthread -lrt #-g -Wall -Werror -O0
CPPFLAGS =
LDFLAGS = -lm -Wl,-Map,project1.map
CFLAGS = -c
LFLAGS = -S

.PHONY: compile-all
compile-all: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS)  -o $@

.PHONY: build
build: $(OBJS) $(OBJS_1)
	$(CC) $(DEBUG) $(OBJS) $(LDFLAGS) $(INCLUDES) -o client.elf
	size client.elf $(OBJS)

.PHONY: clean
clean:
	-rm *.i *.o *.map *.d *.elf a.out

