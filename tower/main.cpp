#include <iostream>
#include <string>
#include "tower.hpp"
#include "log.hpp"
#include "channel.hpp"
#include "area.hpp"
#include "time.hpp"
#include "drawer.hpp"
#include <cmath>
#include <chrono>

// TODO: - Argument parsing
// ./bin/tower_exe DRONE_NUMBER
int main(int argc, char* argv[]) {
    std::string logFile = "tower - " + std::to_string(Time::nanos()) + ".log";
    if (argc >= 5) {
        logFile = argv[4];
    }
    logVerbose(true);
    logOpen(logFile);
    int droneCount = (argc >= 2)? std::stoi(argv[1]) : 100;
    // 6km -> 6000m -> 6000/GRID_FACTOR = Cell_Count
    int areaWidth = 6000 / GRID_FACTOR;
    int areaHeight = 6000 / GRID_FACTOR;
    logInfo("Init", "Running tower with: " + std::to_string(droneCount) + " " + std::to_string(areaWidth) + " " + std::to_string(areaHeight));
    PostgreArgs args;
    args.dbname = "towerdb";
    args.user = "tower";
    Tower tower(droneCount, areaWidth, areaHeight);
    bool connected = tower.connectDb(args);
    if (!connected) {
        logError("Init", "Can't start process without postgre connection!");
        return 1;
    } else {
        logInfo("Init", "Postgre Connection Enstablished");
    }
    connected = tower.connectChannel("127.0.0.1", 6379);
    if (!connected) {
        logError("Init", "Can't start process without redis connection!");
        return 1;
    } else {
        logInfo("Init", "Redis Connection Enstablished");
    }
    tower.start();
    return 0;
}