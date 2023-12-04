RM = rm -f
INCLUDE = .
CFLAGS = -g -Wall #-ansi
CFLAGS += -DSIMPLE_CONFIG_PBC_SUPPORT

TARGET = rtw_simple_config

all: $(TARGET)
$(TARGET): main.o libsc.a
	$(CC) $(CFLAGS) -o $(TARGET) main.o libsc.a -lpthread -lrt

main.o: sc_common.h
	$(CC) $(CFLAGS) -I$(INCLUDE) $(CFLAGS) -c main.c

clean:
	$(RM) $(TARGET) *.o

