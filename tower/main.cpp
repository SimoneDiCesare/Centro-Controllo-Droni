#include <iostream>
#include <string>
#include "tower.hpp"
#include "log.hpp"
#include "channel.hpp"
#include "area.hpp"
#include "time.hpp"
#include <cmath>
#include <chrono>

// TODO: - Argument parsing
// ./bin/tower_exe DRONE_NUMBER AREA_WIDTH AREA_HEIGHT log_file
int main(int argc, char* argv[]) {
    std::string logFile = "tower - " + std::to_string(Time::nanos()) + ".log";
    if (argc >= 5) {
        logFile = argv[4];
    }
    logVerbose(true);
    logOpen(logFile);
    for (int i = 0; i < argc; i++) {
        logInfo("Init", std::to_string(i) + ") " + std::string(argv[i]));
    }
    int droneCount;
    int areaWidth;
    int areaHeight;
    if (argc < 4) {
        logInfo("Init", "Using Default Parameters 10 10 10");
        droneCount = 10;
        areaWidth = 10;
        areaHeight = 10;
    } else {
        droneCount = std::stoi(argv[1]);
        areaWidth = std::stoi(argv[2]);
        areaHeight = std::stoi(argv[3]);
    }
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