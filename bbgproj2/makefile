SOURCES = main.c i2c_wrapper.c tempsense.c temp_ops.c light_ops.c lightsense.c logger/logger.c logger/sync_fileio.c ipc_messq.c remote_socket_server.c myusrled.c
SOURCES_SOCK = socketclient.c i2c_wrapper.c tempsense.c temp_ops.c light_ops.c lightsense.c logger/logger.c logger/sync_fileio.c ipc_messq.c myusrled.c 
OBJS = $(SOURCES:.c=.o)
OBJS_SOCK =$(SOURCES_SOCK:.c=.o)
IMP = $(SOURCES:.c=.i)
IMP_SOCK = $(SOURCES_SOCK:.c=.i)
INCLUDES = main.h

CC = arm-linux-gnueabihf-gcc
DEBUG = -pthread -lrt #-g -Wall -Werror -O0
CPPFLAGS =
LDFLAGS = -lm -Wl,-Map,project1.map
CFLAGS = -c
LFLAGS = -S

THIRD_PARTY_DIR = 3rd-party
CMOCKA_INCLUDE_DIR = $(THIRD_PARTY_DIR)/build-Debug/include
CMOCKA_LIBRARY = $(THIRD_PARTY_DIR)/build-Debug/lib/libcmocka.a
SUBDIRS = 3rd-party
ifdef SUBDIRS
.PHONY : $(SUBDIRS)
$(SUBDIRS) :
	@if [ -d $@ ]; then \
		$(MAKE) --no-print-directory --directory=$@ \
			CC=$(CC) CFLAGS="$(CFLAGS)" $(MAKECMDGOALS); \
	fi
endif

# Declare names of test objects
TEST_SRCS = unittests.c
TEST_OBJS = $(TEST_SRCS:%.c=%.o)
TEST_EXE = unittests.elf

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
	$(CC) $(DEBUG) $(OBJS) $(LDFLAGS) -o project1.elf -lrt
	size project1.elf $(OBJS)

.PHONY: buildc
buildc: $(OBJS_SOCK)
	$(CC) $(DEBUG) $(OBJS_SOCK) $(LDFLAGS) $(INCLUDES) -o client.elf -lrt
	size client.elf $(OBJS_SOCK)


.PHONY: clean
clean:
	-rm *.i *.o *.map *.d project1.elf a.out
	-rm logger/*.o logger/*.d project1.elf a.out

cmocka : $(SUBDIRS)

# Build and execute unit test
unittests : $(TEST_EXE)
	./$(TEST_EXE)

# Generate unit test object
$(TEST_EXE) : $(TEST_SRCS) $(LIB)
	$(CC) $(CFLAGS) -o $@ $^ $(CMOCKA_LIBRARY) -lm -I$(INCLUDES)

# Rebuild library
$(LIB) : $(LIB_OBJS)
	$(AR) $(ARFLAGS) $@ $^
