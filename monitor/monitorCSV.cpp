#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <iomanip>
#include <algorithm>
#include <climits>

std::vector<std::string> split(const std::string &str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

int main(int argc, char* argv[]) {

    std::string filename = "../area.csv";
    
    std::ifstream file(filename);
    
    if (!file.is_open()) {
        std::cerr << "Errore nell'apertura del file " << filename << std::endl;
        return 1;
    }

    std::string line;
    unsigned long long media = 0;
    long long max = LLONG_MIN;
    long long min = LLONG_MAX;
    int tot = 0;
    int visited = 0;

    while (std::getline(file, line)) {
        std::vector<std::string> fields = split(line, ',');
        for (const auto& field : fields) {
            //std::cerr << "'" << field.find('\n') << "'"<< "\n";
            //std::cerr << "'" << std::string::npos << "'"<< "\n";
            if (!field.empty() && field.find('\n') == std::string::npos && field.find('\r') == std::string::npos ) {
                long long value = std::stoll(field); 
                //std::cerr << "'" << value << "'"<< "\n";
                media += value;
                if (value > max) {
                    max = value;
                }
                if (value < min) {
                    min = value;
                }
                if (value != 0){
                    visited += 1;
                }
                
                tot += 1;
            }
        }
    }
    
    file.close(); 
    float percentuale =  (static_cast<float>(visited) / tot) * 100;
    std::cout << "il massimo è: " << max << "\n";
    std::cout << "il minimo è : " << min << "\n";
    std::cout << "la media è: " << media / tot << "\n";
    std::cout << "la percentuale è: " << percentuale << "%" << "\n";
    return 0;
}
