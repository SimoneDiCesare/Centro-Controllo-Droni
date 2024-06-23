#include <iostream>
#include <string>
#include "drone.hpp"
#include "channel.hpp"
#include "log.hpp"
#include "globals.hpp"
#include "time.hpp"

int main(int argc, char* argv[]){
    logVerbose(true);
    logOpen("drone - " + std::to_string(Time::nanos()) + ".log");

    float executionSpeed = 1;
    for (int i = 0; i < argc; i++) {
        logInfo("Init", std::to_string(i) + ") " + std::string(argv[i]));
    }

    if (argc < 2) {
        logInfo("Init", "Using Default Parameters 1 ");
        executionSpeed = 1;   
    } else {
        executionSpeed = std::stoi(argv[1]);
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