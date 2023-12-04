PROJECT_DIR := $(shell pwd)
PROM	= npu_powerctrl_combine
#CXX	?= ../../../output/host/usr/bin/aarch64-rockchip-linux-gnu-g++

CXXFLAGS ?= -fPIC -O3 -I$(PROJECT_DIR) -lpthread
OBJ = npu_powerctrl_combine.o  main.o

$(PROM): $(OBJ)
	$(CXX) -o $(PROM) $(OBJ) $(CXXFLAGS)
%.o: %.c
	$(CXX) -c $< -o $@ $(CXXFLAGS)
clean:
	@rm -rf $(OBJ) $(PROM)

install:
	sudo install -D -m 755 npu_powerctrl_combine -t /usr/bin/npu_powerctrl
