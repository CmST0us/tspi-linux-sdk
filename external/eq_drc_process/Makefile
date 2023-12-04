#common makefile header
PROJECT_DIR := $(shell pwd)
PROM    = eq_drc_process
CXXFLAGS ?= -fPIC -O3 -I$(PROJECT_DIR) -lpthread -lasound -ldl
OBJ =  main.o Rk_wake_lock.o Rk_socket_app.o
$(PROM): $(OBJ)
	$(CXX) -o $(PROM) $(OBJ) $(CXXFLAGS)
%.o: %.cpp
	$(CXX) -c $< -o $@ $(CXXFLAGS)
clean:
	@rm -rf $(OBJ) $(PROM)
install:
	sudo install -D -m 755 eq_drc_process -t /usr/bin/
