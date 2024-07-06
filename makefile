# Compiler
CC = gcc
CCFLAGS = -O2
INCLUDES = -Isrc/utils -Isrc/postgresql -Isrc/redis -Isrc/drawer
# Libs
STD_LIBS = -lstdc++ -lm
POSTGRESQL_LIBS = -lpqxx -lpq
RAY_LIBS = -lraylib -lGL -lpthread -ldl -lrt -lX11
# Directories
BIN_DIR = bin
SRC_DIR = src
EXEC_DIR = exec
SOURCES := $(shell find src -name '*.cpp')
TARGETS := $(addprefix bin/, $(SOURCES:.cpp=.o))
DIRECTORIES := $(sort $(dir $(TARGETS)))
# Code Objects
TOWER_OBJS = $(filter bin/src/tower/%.o, $(TARGETS))
DRONE_OBJS = $(filter bin/src/drone/%.o, $(TARGETS))
REDIS_OBJS = $(filter bin/src/redis/%.o, $(TARGETS))
UTILITY_OBJS = $(filter bin/src/utils/%.o, $(TARGETS))
POSTGRESQL_OBJS = $(filter bin/src/postgresql/%.o, $(TARGETS))
MONITOR_OBJS = $(filter bin/src/monitor/%.o, $(TARGETS))
TESTER_OBJS = $(filter bin/src/tester/%.o, $(TARGETS))
DRAWER_OBJS = $(filter bin/src/drawer/%.o, $(TARGETS))

$(shell mkdir -p $(DIRECTORIES))

PROGS := $(BIN_DIR)/tower $(BIN_DIR)/drone $(BIN_DIR)/log_monitor $(BIN_DIR)/tester $(BIN_DIR)/tower_gui

all: $(PROGS)

bin/%.o: %.cpp 
	$(CC) $(CCFLAGS) $(INCLUDES) -c $< -o $@

$(BIN_DIR)/tower: $(TOWER_OBJS) $(REDIS_OBJS) $(UTILITY_OBJS) $(POSTGRESQL_OBJS) $(DRAWER_OBJS)
	$(CC) $(CCFLAGS) -o $@ $^ $(STD_LIBS) $(POSTGRESQL_LIBS)

$(BIN_DIR)/tower_gui: $(TOWER_OBJS) $(REDIS_OBJS) $(UTILITY_OBJS) $(POSTGRESQL_OBJS) bin/src/drawer/drawer.o
	$(CC) $(CCFLAGS) -o $@ $^ $(STD_LIBS) $(POSTGRESQL_LIBS) $(RAY_LIBS) -DGUI

$(BIN_DIR)/drone: $(DRONE_OBJS) $(REDIS_OBJS) $(UTILITY_OBJS)
	$(CC) $(CCFLAGS) -o $@ $^ $(STD_LIBS)

$(BIN_DIR)/log_monitor: $(MONITOR_OBJS)
	$(CC) $(CCFLAGS) -o $@ $^ $(STD_LIBS)

$(BIN_DIR)/tester: $(TESTER_OBJS) bin/src/utils/log.o
	$(CC) $(CCFLAGS) -o $@ $^ $(STD_LIBS)

clean:
	@echo "Cleaning Objects and Build"
	rm -rf bin





