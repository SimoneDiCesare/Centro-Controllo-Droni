#include <iostream>
#include <string>
// #include "postgresql.hpp"
#include "log.hpp"

int main() {
    logOpen("test.log");
    logInfo("TOWER", "Hello World");
    verbose = true;
    logInfo("DRONE 1", "Hello World");
    return 0;
}