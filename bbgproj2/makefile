SOURCES = main.c logger.c sync_fileio.c ipc_messq.c myusrled.c server_socket.c
SOURCES_SOCK = 
OBJS = $(SOURCES:.c=.o)
OBJS_SOCK =
IMP = $(SOURCES:.c=.i)
IMP_SOCK = $(SOURCES_SOCK:.c=.i)
INCLUDES = main.h

CC = arm-linux-gnueabihf-gcc
#CC = gcc
DEBUG = -pthread -lrt #-g -Wall -Werror -O0
CPPFLAGS =
LDFLAGS = -lm -Wl,-Map,project2.map
CFLAGS = -c
LFLAGS = -S

%.o:%.c
	$(CC) $(DEBUG) $(CPPFLAGS) $(CFLAGS) -MMD $^ -o $@


%.i:%.c
	$(CC) $(DEBUG) -E $(CPPFLAGS) $^ -o $@

%.asm:%.c

	$(CC) $(DEBUG) $(CPPFLAGS) $(CFLAGS) -S $< -o $@


.PHONY: compile-all
compile-all: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS)  -o $@

.PHONY: build
build: $(OBJS)
	$(CC) $(DEBUG) $(OBJS) $(LDFLAGS) -o project2.elf -lrt
	size project2.elf $(OBJS)

.PHONY: buildc
buildc: $(OBJS_SOCK)
	$(CC) $(DEBUG) $(OBJS_SOCK) $(LDFLAGS) $(INCLUDES) -o client.elf -lrt
	size client.elf $(OBJS_SOCK)


.PHONY: clean
clean:
	-rm *.i *.o *.map *.d *.elf a.out
