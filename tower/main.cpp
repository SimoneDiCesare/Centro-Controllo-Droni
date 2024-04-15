#include <iostream>
#include <string>
#include "tower.hpp"

// TODO: - Argument parsing
int main(int argc, char* argv[]) {
    std::cout << "Running with arguments: \n";
    for (int i = 0; i < argc; i++) {
        std::cout << "" << i << ") " << argv[i] << "\n";
    }
    std::cout << "========================\n";
    Tower tower;
    tower.connect("127.0.0.1", 6379);
    tower.start();
    return 0;
}