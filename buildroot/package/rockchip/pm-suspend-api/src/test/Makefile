%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS) -I ../

TARGET_BINS = app_test1 app_test2 sleep_test

all:$(TARGET_BINS)

app_test1: app_test1.o
	$(CC) -o $@ $^ $(CFLAGS) -lsuspend_api -ldbus-1 -L../ 

app_test2: app_test2.o 
	$(CC) -o $@ $^ $(CFLAGS) -lsuspend_api -ldbus-1 -L../

sleep_test: sleep_test.o 
	$(CC) -o $@ $^ $(CFLAGS) -lsuspend_api -ldbus-1 -L../

.PHONY: clean

clean:
	rm -f *.o *~ $(TARGET_BINS)

.PHONY: install

install:
	cp -f  app_test1 $(TARGET_DIR)/usr/bin/
	cp -f  app_test2 $(TARGET_DIR)/usr/bin/
	cp -f  sleep_test $(TARGET_DIR)/usr/bin/
	cp -f  app_test_dbus.conf $(TARGET_DIR)/etc/dbus-1/system.d/

.PHONY: uninstall

uninstall:
	rm -f $(TARGET_DIR)/usr/bin/app_test1
	rm -f $(TARGET_DIR)/usr/bin/app_test2
	rm -f $(TARGET_DIR)/usr/bin/sleep_test

