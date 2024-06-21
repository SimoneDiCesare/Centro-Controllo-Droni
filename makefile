# Compiler
CC = gcc
CCFLAGS_D = -g -O0 -Iutils -Ipostgresql -Iredis
CCFLAGS = -O2 -Iutils -Ipostgresql -Iredis -Idrawer
# Libs
LIBS = -lstdc++ -lm
POSTGRESQL_LIBS = -lpqxx -lpq
RAY_LIBS = -lraylib -lGL -lpthread -ldl -lrt -lX11
# Directories
BIN_DIR := bin
TOWER_DIR := tower
DRONE_DIR := drone
REDIS_DIR := redis
UTILITY_DIR := utils
POSTGRESQL_DIR := postgresql
MONITOR_DIR := monitor
TESTER_DIR := tester
DRAWER_DIR := drawer
# Code Files
TOWER_FILES = $(wildcard $(TOWER_DIR)/*.cpp)
DRONE_FILES = $(wildcard $(DRONE_DIR)/*.cpp)
REDIS_FILES = $(wildcard $(REDIS_DIR)/*.cpp)
UTILITY_FILES = $(wildcard $(UTILITY_DIR)/*.cpp)
POSTGRESQL_FILES = $(wildcard $(POSTGRESQL_DIR)/*.cpp)
MONITOR_FILES = $(wildcard $(MONITOR_DIR)/*.cpp)
TESTER_FILES = $(wildcard $(TESTER_DIR)/*.cpp)
DRAWER_FILES = $(wildcard $(DRAWER_DIR)/*.cpp)
# Objects Files 
TOWER_OBJS := $(addprefix $(BIN_DIR)/, $(TOWER_FILES:.cpp=.o))
DRONE_OBJS := $(addprefix $(BIN_DIR)/, $(DRONE_FILES:.cpp=.o))
REDIS_OBJS := $(addprefix $(BIN_DIR)/, $(REDIS_FILES:.cpp=.o))
UTILITY_OBJS := $(addprefix $(BIN_DIR)/, $(UTILITY_FILES:.cpp=.o))
POSTGRESQL_OBJS := $(addprefix $(BIN_DIR)/, $(POSTGRESQL_FILES:.cpp=.o))
MONITOR_OBJS := $(addprefix $(BIN_DIR)/, $(MONITOR_FILES:.cpp=.o))
TESTER_OBJS := $(addprefix $(BIN_DIR)/, $(TESTER_FILES:.cpp=.o))
DRAWER_OBJS := $(addprefix $(BIN_DIR)/, $(DRAWER_FILES:.cpp=.o))

# Creating directories
$(shell mkdir -p $(BIN_DIR)/$(TOWER_DIR) $(BIN_DIR)/$(DRONE_DIR) $(BIN_DIR)/$(REDIS_DIR) $(BIN_DIR)/$(UTILITY_DIR) $(BIN_DIR)/$(POSTGRESQL_DIR) $(BIN_DIR)/$(MONITOR_DIR) $(BIN_DIR)/$(TESTER_DIR) $(BIN_DIR)/$(DRAWER_DIR))

PROGS := $(BIN_DIR)/tower_exe $(BIN_DIR)/drone_exe $(BIN_DIR)/monitor_exe $(BIN_DIR)/tester_exe $(BIN_DIR)/tower_gui

all: $(PROGS)

# Compiling Targets
$(BIN_DIR)/$(TOWER_DIR)/%.o: $(TOWER_DIR)/%.cpp
	$(CC) $(CCFLAGS) -c $< -o $@

$(BIN_DIR)/$(DRONE_DIR)/%.o: $(DRONE_DIR)/%.cpp
	$(CC) $(CCFLAGS) -c $< -o $@

$(BIN_DIR)/$(REDIS_DIR)/%.o: $(REDIS_DIR)/%.cpp
	$(CC) $(CCFLAGS) -c $< -o $@

$(BIN_DIR)/$(UTILITY_DIR)/%.o: $(UTILITY_DIR)/%.cpp
	$(CC) $(CCFLAGS) -c $< -o $@

$(BIN_DIR)/$(POSTGRESQL_DIR)/%.o: $(POSTGRESQL_DIR)/%.cpp
	$(CC) $(CCFLAGS) -c $< -o $@

$(BIN_DIR)/$(MONITOR_DIR)/%.o: $(MONITOR_DIR)/%.cpp
	$(CC) $(CCFLAGS) -c $< -o $@

$(BIN_DIR)/$(TESTER_DIR)/%.o: $(TESTER_DIR)/%.cpp
	$(CC) $(CCFLAGS) -c $< -o $@

$(BIN_DIR)/$(DRAWER_DIR)/%.o: $(DRAWER_DIR)/%.cpp
	$(CC) $(CCFLAGS) -c $< -o $@

$(BIN_DIR)/tower_exe: $(TOWER_OBJS) $(REDIS_OBJS) $(UTILITY_OBJS) $(POSTGRESQL_OBJS) $(DRAWER_OBJS)
	$(CC) $(CCFLAGS) -o $@ $^ $(LIBS) $(POSTGRESQL_LIBS)

$(BIN_DIR)/tower_gui: $(TOWER_OBJS) $(REDIS_OBJS) $(UTILITY_OBJS) $(POSTGRESQL_OBJS) drawer/drawer.cpp
	$(CC) $(CCFLAGS) -o $@ $^ $(LIBS) $(POSTGRESQL_LIBS) $(RAY_LIBS) -DGUI

$(BIN_DIR)/drone_exe: $(DRONE_OBJS) $(REDIS_OBJS) $(UTILITY_OBJS)
	$(CC) $(CCFLAGS) -o $@ $^ $(LIBS)

$(BIN_DIR)/monitor_exe: $(MONITOR_OBJS)
	$(CC) $(CCFLAGS) -o $@ $^ $(LIBS)

$(BIN_DIR)/tester_exe: $(TESTER_OBJS)
	$(CC) $(CCFLAGS) -o $@ $^ $(LIBS)

.PHONY: all clean

clean:
	@echo "Cleaning binaries"
	rm -rf bin