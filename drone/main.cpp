#include <iostream>
#include <string>
#include "drone.hpp"
#include "channel.hpp"
#include "log.hpp"
#include "globals.hpp"
#include "time.hpp"

int main(int argc, char* argv[]){
    logVerbose(false);
    // logOpen("drone - " + std::to_string(Time::nanos()) + ".log");
    logOpen("drone.log");
    float executionSpeed = 1;
    if (argc < 2) {
        logInfo("Init", "Using Default Execution Speed: 1");
        executionSpeed = 1;   
    } else {
        executionSpeed = std::stof(argv[1]);
        logInfo("Init", "Uscing Execution Speed: " + std::to_string(executionSpeed));
    }

    Drone drone;
    bool connected = drone.connectChannel("127.0.0.1", 6379);
    if (!connected) {
        logError("Init", "Can't start process without redis connection!");
        return 1;
    } else {
        logInfo("Init", "Drone connected on redis with temp id: " + std::to_string(drone.getId()));
    }
    drone.start(executionSpeed);
    return 0;
}