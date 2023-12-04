PROJECT_DIR := $(shell pwd)
CC = gcc
CXX = g++
LIB_NAME = libavb_user.so 
OBJ = avb_ops_user.o\
    avb_user_verification.o\
    avb_user_verity.o

CFLAGS += -I/home/hkh/projects/Linux/rk3308/external/avb/

$(LIB_NAME): $(OBJ)
	$(CC) -shared -o $(LIB_NAME) $(OBJ) $(CFLAGS)

%.o: %.cpp
	$(CXX) -c -fPIC $(CFLAGS) $< -o $@

%.o: %.c
	$(CC) -c -fPIC $(CFLAGS) $< -o $@

clean:
	rm -rf $(OBJ) $(LIB_NAME)

install:
	sudo install -D -m 755 libavb_user.so -t /usr/bin/
