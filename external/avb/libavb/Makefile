PROJECT_DIR := $(shell pwd)
CC = gcc
LIB_NAME = libavb.so 
OBJ = avb_chain_partition_descriptor.o\
    avb_cmdline.o\
    avb_crc32.o\
    avb_crypto.o\
    avb_descriptor.o\
    avb_footer.o\
    avb_hash_descriptor.o\
    avb_hashtree_descriptor.o\
    avb_kernel_cmdline_descriptor.o\
    avb_property_descriptor.o\
    avb_rsa.o\
    avb_sha256.o\
    avb_sha512.o\
    avb_slot_verify.o\
    avb_sysdeps_posix.o\
    avb_util.o\
    avb_vbmeta_image.o\
    avb_version.o

$(LIB_NAME): $(OBJ)
	$(CC) -shared -o $(LIB_NAME) $(OBJ)

%.o: %.c
	$(CC) -c -fPIC $< -o $@

clean:
	rm -rf $(OBJ) $(LIB_NAME)

install:
	sudo install -D -m 755 libavb.so -t /usr/bin/
