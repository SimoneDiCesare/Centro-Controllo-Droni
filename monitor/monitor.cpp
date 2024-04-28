#include <iostream>
#include <fstream>
#include <string>
#include <vector>

int main () {
    std::string fileName;
    // Utilizza getline per ottenere una riga di input
    std::cout << "Inserire file .log: ";
    std::getline(std::cin, fileName); 

    std::ifstream fileLog(fileName); // Apri direttamente il file specificato

    if (!fileLog.is_open()) {
        std::cerr << "Impossibile aprire il file " << fileName << std::endl;
        return 1; 
    }

    std::string line;
    std::vector<std::string> tokens;
    while (std::getline(fileLog, line)) {
        
        
        tokens.clear(); 
        size_t pos1 = line.find("[");
        size_t pos2 = line.find("]");
        while (pos1 != std::string::npos && pos2 != std::string::npos) {
            // Estrai la sottostringa tra i delimitatori
            std::string token = line.substr(pos1 + 1, pos2 - pos1 - 1); 
            tokens.push_back(token);
            // Trova la prossima occorrenza dei delimitatori nella riga
            pos1 = line.find("[", pos2 + 1);
            pos2 = line.find("]", pos1 + 1);
        }
        
        for (const auto& t : tokens) {
            if (t == "ERROR"){
                std::cout << line << std::endl;
            }
        }
    }

    fileLog.close();
    
    return 0;
}
