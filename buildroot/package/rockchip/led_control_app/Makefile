PROJECT_DIR := $(shell pwd)
PROM	= led_control_app
#CXX	?= ../../../output/host/usr/bin/aarch64-rockchip-linux-gnu-g++

CXXFLAGS ?= -fPIC -O3 -I$(PROJECT_DIR) -lpthread
OBJ = InfoLed.o  main.o

$(PROM): $(OBJ)	
	$(CXX) -o $(PROM) $(OBJ) $(CXXFLAGS)
%.o: %.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS)
clean:
	@rm -rf $(OBJ) $(PROM)

install:
	sudo install -D -m 755 led_control_app -t /usr/bin/
