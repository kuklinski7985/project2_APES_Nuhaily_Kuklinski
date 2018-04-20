SOURCES = mainserver.c server_socket.c
SOURCES_1= mainclient.c client_socket.c 
OBJS = $(SOURCES:.c=.o)
OBJS_1 =$(SOURCES_1:.c=.o)
IMP = $(SOURCES:.c=.i)
IMP_1 = $(SOURCES_1:.c=.i)
#INCLUDES = main.h

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
	$(CC) $(DEBUG) $(OBJS) $(LDFLAGS) -o server.elf
	size server.elf $(OBJS)

	$(CC) $(DEBUG) $(OBJS_1) $(LDFLAGS) $(INCLUDES) -o client.elf
	size client.elf $(OBJS_1)

.PHONY: clean
clean:
	-rm *.i *.o *.map *.d *.elf a.out

