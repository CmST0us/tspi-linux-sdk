PROJECT_DIR := $(shell pwd)
PROM = rkupdate
OBJ =CRC.o \
	MD5Checksum.o \
	RKAndroidDevice.o \
	RKBoot.o \
	RKComm.o \
	RKDevice.o \
	RKImage.o \
	RKLog.o \
	Upgrade.o \
	main.o

$(PROM): $(OBJ)
	$(CXX) -o $(PROM) $(OBJ) $(CFLAGS)

%.o: %.cpp
	$(CXX) -c $< -o $@ $(CFLAGS)

clean:
	rm -rf $(OBJ) $(PROM)

install:
	sudo install -D -m 755 recovery -t /usr/bin/
