PROJECT_DIR := $(shell pwd)
CC = gcc
LIB_NAME = bootcontrol 
OBJ = boot_control_avb.o 

CFLAGS += -I/home/hkh/projects/Linux/rk3308/external/avb/

$(LIB_NAME): $(OBJ)
	$(CC) -o $(LIB_NAME) $(OBJ) $(CFLAGS)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -rf $(OBJ) $(LIB_NAME)

install:
	sudo install -D -m 755 bootcontrol -t /usr/bin/
