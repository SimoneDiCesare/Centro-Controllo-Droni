#include <iostream>
#include <string>
// #include "postgresql.hpp"
#include "log.hpp"

int main() {
    logOpen("test.log");
    logInfo("TOWER", "Hello World");
    verbose = true;
    logInfo("DRONE 1", "Hello World");
    // struct PostgreArgs args;
    // Postgre p(args);
    // auto [success,  result] = p.execute("TRUNCATE TABLE drone");
    // std::cout << success << " - " << result.empty() << "\n";
    // std::cout << p.truncateTable("drone") << "\n";
    // return 0;
}