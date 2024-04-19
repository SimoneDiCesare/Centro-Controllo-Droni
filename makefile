# TODO: Check drone files and target
# Compiler and Flags
CC = gcc
CCFLAGS = -O2 -Iutils
LIBS = -lstdc++
POSTGRESQL_LIBS = -lpqxx -lpq
TOWER_FILES = tower/main.cpp tower/tower.cpp
DRONE_FILES = Drone/main.cpp Drone/Droni.cpp
REDIS_FILES = utils/redis.cpp utils/channel.cpp utils/log.cpp
POSTGRESQL_FILES = utils/postgresql.cpp

# Targets
TARGETS = tower drone

# Default Target
all: $(TARGETS)

$(shell mkdir -p bin)

# Tower Executable
tower: $(TOWER_FILES) $(REDIS_FILES)
	$(CC) $(CCFLAGS) $(TOWER_FILES) $(REDIS_FILES) -o bin/tower $(LIBS) $(POSTGRESQL_LIBS)

# Drone Executable
drone: $(DRONE_FILES) $(REDIS_FILES)
	$(CC) $(CCFLAGS) $(DRONE_FILES) $(REDIS_FILES) -o bin/drone $(LIBS)

clean:
	@echo "Cleaning Executables"
	rm -f bin/tower bin/drone

.PHONY: all clean tower drone