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
    for (int i = 0; i < argc; i++) {
        logInfo("Init", std::to_string(i) + ") " + std::string(argv[i]));
    }
    Drone drone;
    bool connected = drone.connectChannel("127.0.0.1", 6379);
    if (!connected) {
        logError("Init", "Can't start process without redis connection!");
        return 1;
    } else {
        logInfo("Init", "Drone connected on redis with temp id: " + std::to_string(drone.getId()));
    }
    drone.start();
    return 0;
}