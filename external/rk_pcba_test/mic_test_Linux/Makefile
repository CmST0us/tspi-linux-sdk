#CC = gcc
CROSS = /home/yyz/audio/rk3308/buildroot/output/rockchip_rk3308_pcba_32bit/host/bin/arm-rockchip-linux-gnueabihf-
CC = $(CROSS)gcc
CXX = $(CROSS)g++

#MW_DIR=$(PWD)
LIBS += -lpthread 
MW_DIR=./
TARGET = libmictest.so
INCLUDE = -I./
#
CFLAGS += -fPIC -DTINA_LINUX -DWITHOUT_ALOG=1 -lpthread -O3 -fno-strict-aliasing -fprefetch-loop-arrays -mfloat-abi=hard -mfpu=neon -marm -march=armv7-a
#-mfpu=neon -marm -march=armv7-a 
CXXFLAGS += -fPIC -DTINA_LINUX -DWITHOUT_ALOG=1 -lpthread -O3 -fno-strict-aliasing -fprefetch-loop-arrays -mfloat-abi=hard -mfpu=neon -marm -march=armv7-a
#-mfpu=neon -marm -march=armv7-a 
LDFLAGS = -fPIC -shared
#找到所有编译目录下的头文件--
SAMDIR=$(MW_DIR)

SAMINCH=$(shell find $(SAMDIR) -name "*.h*")

#找到了，大家可以看到Makefile是可以使用shell命令的，现在结果存储在SAMINCH。

TMP_DIR=$(dir $(SAMINCH))

#dir是个函数，就是列出SAMINCH目录前缀（/*.h之前的字串）的结果。

SAMINCDIR=$(sort $(TMP_DIR))

#sort也是个函数，这是为了去除重复的目录名。

INCLUDE += $(foreach temp, $(SAMINCDIR), -I$(temp))

#只是列出所有匹配的目录结果， 加到-I选项之后，用空格隔开。


#好了，轮到源程序了。

CPP_FILES=$(shell find $(SAMDIR)  -name '*.cpp')
C_FILES =$(shell find $(SAMDIR) -name '*.c')

OBJCPP=$(CPP_FILES:%.cpp=%.o)
OBJC  =$(C_FILES:%.c=%.o)

#建立规则
#$(warning objcpp is $(OBJCPP) objc is $(OBJC) )
#$(warning cflags is $(CFLAGS))
#下面实际的编译过程加上：
$(TARGET): $(OBJCPP) $(OBJC)
#	$(CXX) $(LDFLAGS)  -o $(TARGET)
	$(CXX) $(LDFLAGS) $(OBJCPP) $(OBJC) -o $@  $(LIBS)

$(OBJCPP): %.o: %.cpp
	$(CXX) -c $< $(CXXFLAGS) $(INCLUDE)  -o $@ 

$(OBJC): %.o: %.c
	$(CC) -c $(CFLAGS) $(INCLUDE) $< -o $@ 

#control: clean $(APP)
#	$(CPP) -m32 -o $@ $(INCLUDE) $(APP)

clean:
	rm -fr $(OBJCPP) $(OBJC)
#	rm -f control

