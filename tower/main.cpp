#include <iostream>
#include <string>
#include "tower.hpp"
#include "log.hpp"

// TODO: - Argument parsing
int main(int argc, char* argv[]) {
    logVerbose(true);
    logOpen("test.log");
    for (int i = 0; i < argc; i++) {
        logInfo("ARGS", std::to_string(i) + ") " + std::string(argv[i]));
    }
    Tower tower;
    tower.connect("127.0.0.1", 6379);
    tower.start();
    return 0;
}