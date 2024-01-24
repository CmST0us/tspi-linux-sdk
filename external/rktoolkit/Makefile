PROJECT_DIR := $(shell pwd)

.PHONY : all
all: io update vendor_storage sample_vendor_lib minimad

io: 
	$(CC) -c $(PROJECT_DIR)/io.c
	$(CC) -o $(PROJECT_DIR)/io $(PROJECT_DIR)/io.o
update: 
	$(CC) -c $(PROJECT_DIR)/update_recv/update_recv.c -I$(PROJECT_DIR)/update_recv
	$(CC) -c $(PROJECT_DIR)/update.c -I$(PROJECT_DIR)
	$(CC) -o $(PROJECT_DIR)/update $(PROJECT_DIR)/update.o $(PROJECT_DIR)/update_recv.o
vendor_storage: 
	$(CC) -c $(PROJECT_DIR)/vendor_storage.c
	$(CC) -o $(PROJECT_DIR)/vendor_storage $(PROJECT_DIR)/vendor_storage.o
sample_vendor_lib: 
	$(CC) -o $(PROJECT_DIR)/libvendor_storage.o -c $(PROJECT_DIR)/vendor_storage.c -DBUILD_LIB_VENDOR_STORAGE
	$(CC) -c $(PROJECT_DIR)/sample/sample_vendor_lib.c -I$(PROJECT_DIR)
	$(CC) -o $(PROJECT_DIR)/sample_vendor_lib $(PROJECT_DIR)/sample_vendor_lib.o $(PROJECT_DIR)/libvendor_storage.o
minimad:
	$(CC) -c $(PROJECT_DIR)/minimad.c
	$(CC) -o $(PROJECT_DIR)/minimad $(PROJECT_DIR)/minimad.o -lmad

clean:
	rm -rf $(PROJECT_DIR)/io \
	$(PROJECT_DIR)/update \
	$(PROJECT_DIR)/vendor_storage \
	$(PROJECT_DIR)/sample_vendor_lib \
	$(PROJECT_DIR)/minimad \
	$(PROJECT_DIR)/io.o \
	$(PROJECT_DIR)/libvendor_storage.o \
	$(PROJECT_DIR)/sample_vendor_lib.o \
	$(PROJECT_DIR)/update.o \
	$(PROJECT_DIR)/update_recv.o \
	$(PROJECT_DIR)/vendor_storage.o \
	$(PROJECT_DIR)/minimad.o

install:
	mkdir -p $(DESTDIR)/usr/bin
	install -D -m 755 $(PROJECT_DIR)/io $(DESTDIR)/usr/bin/
	install -D -m 755 $(PROJECT_DIR)/update $(DESTDIR)/usr/bin/
	install -D -m 755 $(PROJECT_DIR)/vendor_storage $(DESTDIR)/usr/bin/
	install -D -m 755 $(PROJECT_DIR)/sample_vendor_lib $(DESTDIR)/usr/bin/
	install -D -m 755 $(PROJECT_DIR)/minimad $(DESTDIR)/usr/bin/

