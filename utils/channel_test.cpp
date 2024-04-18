#include <iostream>
#include <string>
#include "postgresql.hpp"
int main() {
    struct PostgreArgs args;
    Postgre p(args);
    auto [success,  result] = p.execute("TRUNCATE TABLE drone");
    std::cout << success << " - " << result.empty() << "\n";
    std::cout << p.truncateTable("drone") << "\n";
    return 0;
}