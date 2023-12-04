PROJECT_DIR := $(shell pwd)

LIB_NAME = libupdateengine.so
BOOTCONTROL_BIN_NAME = rkboot_control
TEST_BIN_NAME = update_engine

all: $(LIB_NAME) $(BOOTCONTROL_BIN_NAME) $(TEST_BIN_NAME)
.PHONY : all

OBJ_BOOTCONTROL = bootcontrol_main.o

OBJ_RKBOOTCONTROL = rkboot_control.o

OBJ_UPDATE = log.o \
    partition.o \
    rkimage.o \
    tomcat.o \
    md5sum.o \
    download.o \
    deviceInfo.o \
    update.o

OBJ_TEST = test_main.o

#CFLAGS += -lcrypto -lssl -fPIC -std=c++11 -lcurl -lpthread

$(BOOTCONTROL_BIN_NAME): $(OBJ_BOOTCONTROL) $(OBJ_RKBOOTCONTROL)
	$(CXX) -o $(BOOTCONTROL_BIN_NAME) $(OBJ_BOOTCONTROL) $(OBJ_RKBOOTCONTROL)

$(TEST_BIN_NAME): $(OBJ_UPDATE) $(OBJ_TEST) $(OBJ_RKBOOTCONTROL)
	$(CXX) -o $(TEST_BIN_NAME) $(OBJ_UPDATE) $(OBJ_TEST) $(OBJ_RKBOOTCONTROL) $(CFLAGS)

$(LIB_NAME): $(OBJ_UPDATE) $(OBJ_RKBOOTCONTROL)
	$(CXX) -shared -o $(LIB_NAME) $(OBJ_UPDATE) $(OBJ_RKBOOTCONTROL)

%.o: %.cpp
	$(CXX) -c $(CFLAGS) -fPIC $< -o $@

clean:
	rm -rf $(OBJ) $(PROM)

install:
	sudo install -D -m 755 update_engine -t /usr/bin/
