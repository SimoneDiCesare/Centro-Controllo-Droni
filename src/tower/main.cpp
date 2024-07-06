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


// ./bin/tower_exe DRONE_NUMBER WIDTH HEIGHT TOLLERANCE
int main(int argc, char* argv[]) {
    // std::string logFile = "tower - " + std::to_string(Time::nanos()) + ".log";
    std::string logFile = "tower.log";
    logVerbose(true);
    logOpen(logFile);
    // We divide by 7 for efficency reasons
    int droneCount = (argc >= 2)? std::stoi(argv[1]) / 7 : 100 / 7;
    // Sizes in meters -> 6000m default
    int width = (argc >= 3)? std::stoi(argv[2]) : 6000;
    int height = (argc >= 4)? std::stoi(argv[3]) : 6000;
    // Tollerance of elapsed time for a visited cell in seconds -> default is 5 minutes
    int cellTollerance = (argc >= 4)? std::stoi(argv[4]) : 60 * 5;
    int areaWidth = width / GRID_FACTOR;
    int areaHeight = height / GRID_FACTOR;
    logInfo("Init", "Running tower with: " + std::to_string(droneCount) + " " + std::to_string(areaWidth) + " " + std::to_string(areaHeight));
    PostgreArgs args;
    args.dbname = "towerdb";
    args.user = "tower";
    Tower tower(droneCount, areaWidth, areaHeight, cellTollerance);
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