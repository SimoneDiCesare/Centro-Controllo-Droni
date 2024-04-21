#include <iostream>
#include <string>
#include "tower.hpp"
#include "log.hpp"

// TODO: - Argument parsing
int main(int argc, char* argv[]) {
    logVerbose(true);
    logOpen("test.log");
    for (int i = 0; i < argc; i++) {
        logInfo("Init", std::to_string(i) + ") " + std::string(argv[i]));
    }
    PostgreArgs args;
    args.dbname = "towerdb";
    args.user = "tower";
    Tower tower;
    bool connected = tower.connectDb(args);
    if (!connected) {
        logError("Init", "Can't start process without postgre connection!");
        return 1;
    }
    connected = tower.connect("127.0.0.1", 6379);
    if (!connected) {
        logError("Init", "Can't start process without redis connection!");
        return 1;
    }
    tower.start();
    return 0;
}