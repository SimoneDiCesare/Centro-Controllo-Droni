#include <iostream>
#include <fstream>
#include <string>
#include <vector>

int main(int argc, char* argv[]) {

    std::string fileName = std::string(argv[1]) + " " + std::string(argv[2]) + " " + std::string(argv[3]);

    //std::cout << "Inserire file .log: ";
    //std::getline(std::cin, fileName); 
    //std::cout << "\n";
    std::ifstream fileLog(fileName);

    if (!fileLog.is_open()) {
        std::cerr << "Impossibile aprire il file " << fileName << std::endl;
        return 1; 
    }

    bool error = false;
    
    std::string line;
    std::vector<std::string> tokens;
    while (std::getline(fileLog, line)) {
        
        
        tokens.clear(); 
        size_t pos1 = line.find("[");
        size_t pos2 = line.find("]");
        while (pos1 != std::string::npos && pos2 != std::string::npos) {
            std::string token = line.substr(pos1 + 1, pos2 - pos1 - 1); 
            tokens.push_back(token);

            pos1 = line.find("[", pos2 + 1);
            pos2 = line.find("]", pos1 + 1);
        }

        for (const auto& t : tokens) {
            if (t == "ERROR"){
                error = true;
                std::cout << line << std::endl;
            }
        }
    }

    if (error == false){
        std::cerr << "Nel file "<< fileName << " non ci sono messaggi di errore." << std::endl;
    }
    
    fileLog.close();
    
    return 0;
}
