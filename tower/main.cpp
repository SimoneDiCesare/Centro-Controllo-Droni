#include <iostream>
#include <string>
#include "tower.hpp"
#include "log.hpp"
#include "channel.hpp"
#include <chrono>

// TODO: - Argument parsing
int main(int argc, char* argv[]) {
    /*std::string s = "$4\r\ncome\r\n$5\r\nstai?\r\n";
    std::size_t delimiter = s.find("\r\n");
    while (true) {
        if (delimiter >= s.length()) {
            break;
        }
        int size = std::stoi(s.substr(1, delimiter));
        std::string t = s.substr(delimiter + 2, size);
        std::cout << size << " -> " << t.length() << "[" << t << "]\n";
        s = s.substr(delimiter + 4 + size, s.length());
    }
    return 0;*/
    logVerbose(true);
    logOpen("tower.log");
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
    connected = tower.connectChannel("127.0.0.1", 6379);
    if (!connected) {
        logError("Init", "Can't start process without redis connection!");
        return 1;
    }
    tower.start();
    return 0;
}