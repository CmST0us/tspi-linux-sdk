PROJECT_DIR := $(shell pwd)
CC = gcc
LIB_NAME = libavb_ab.so 
OBJ = avb_ab_flow.o 

CFLAGS += -I../
$(LIB_NAME): $(OBJ)
	$(CC) -shared -o $(LIB_NAME) $(OBJ) $(CFLAGS)

%.o: %.c
	$(CC) -c -fPIC $(CFLAGS) $< -o $@

clean:
	rm -rf $(OBJ) $(LIB_NAME)

install:
	sudo install -D -m 755 libavb_ab.so -t /usr/bin/
