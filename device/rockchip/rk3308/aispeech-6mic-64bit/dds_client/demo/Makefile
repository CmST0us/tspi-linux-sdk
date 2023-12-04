LOCAL_MODULE 		     := demo_main
LOCAL_SRC_FILES 		 += main.c
LOCAL_SRC_DIRS 		     += json
LOCAL_SRC_DIRS 		     += button 

LOCAL_CFLAGS 	:= -rdynamic -g -O0 -Wall -DMG_ENABLE_THREADS  -Wno-unused-variable -fPIC
LOCAL_CFLAGS 	+= -I.. -Ijson/ -Ibutton -Ibusserver/mongoose -Ibusserver/src
LOCAL_CFLAGS 	+= -DRK3308_BOARD_V11
LOCAL_CFLAGS 	+= -DPDM_MIC # for v11
#

LOCAL_LDFLAGS += -Wl,-rpath,../
LOCAL_LDFLAGS += -L../ -ldds_client

LOCAL_LDFLAGS += -Wl,-rpath,../libs/
#LOCAL_LDFLAGS += -L../libs/ -lduilite_normal
#LOCAL_LDFLAGS += -L../libs/ -lduilite_fespl
LOCAL_LDFLAGS += -L../libs/ -lduilite_fespa
LOCAL_LDFLAGS += -L../libs/ -lauth
LOCAL_LDFLAGS += -L../libs/ -ldds
LOCAL_LDFLAGS += -L../libs/ -laudio_play
LOCAL_LDFLAGS += -lpthread
LOCAL_LDFLAGS += -lasound


LOCAL_CXXFLAGS 	:= LOCAL_CFLAGS -rdynamic 

CC = ../../../../../../buildroot/output/rockchip_rk3308_release/host/usr/bin/aarch64-linux-gcc

CFLAGS += $(LOCAL_CFLAGS)

demo_main:  main.o music.o json/cJSON.o button/button_api.o busserver/src/busserver.o busserver/mongoose/mongoose.c
	$(CC) -o $@ $^ $(CFLAGS) $(LOCAL_LDFLAGS) -lm

clean:
	rm -f demo_main
	rm -f *.o
