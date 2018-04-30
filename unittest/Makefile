vpath %.c ../bbgproj2

INCLUDE_DIRS = -I../bbgproj2
INCLUDE_DIRS += -I$(CMOCKA_DIR)/include

CFLAGS	= $(INCLUDE_DIRS)

LDFLAGS	= -L$(CMOCKA_DIR)/build/src -Wl,-rpath=$(CMOCKA_DIR)/build/src 
LDFLAGS += -lcmocka -lrt

TARGET = unittests.elf

SOURCES = comm.c sync_fileio.c myusrled.c server_socket.c logger.c ipc_messq.c
TEST_SRC = unittests.c

SRC_OBJ = $(patsubst %.c,obj/%.o,$(SOURCES)) 
TEST_OBJ = $(patsubst %.c,obj/%.o,$(TEST_SRC)) 
OBJ_DIR = obj

CMOCKA_TAR = cmocka-1.1.1.tar.gz
CMOCKA_DIR = cmocka

all: $(CMOCKA_DIR) $(OBJ_DIR) $(TARGET)
	./$(TARGET)

$(SRC_OBJ): | $(OBJ_DIR)

$(OBJ_DIR):
	@mkdir -p $@

$(OBJ_DIR)/%.o : %.c
	@echo $< 
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET):  $(SRC_OBJ) $(TEST_OBJ)
	$(CC) -o $@ $^ $(LDFLAGS)

$(CMOCKA_DIR):
	mkdir -p $@
	tar -xf $(CMOCKA_TAR) -C $@ --strip-components 1
	mkdir -p $@/build
	@(cd $@/build && cmake ../ && make -s)

clean:
	rm -rf $(TARGET) $(OBJ_DIR) $(CMOCKA_DIR)