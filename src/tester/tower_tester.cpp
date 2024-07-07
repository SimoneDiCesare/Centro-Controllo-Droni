#include <random>
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <filesystem>
#include <cstring>
#include "log.hpp"
namespace fs = std::filesystem;

void checkLogs() {
    // Directory di cui vogliamo elencare i file
    std::string directory_path = "./";
    try {
        // Iteriamo attraverso tutti i file nella directory specificata
        for (const auto& entry : fs::directory_iterator(directory_path)) {
            // Verifichiamo se l'elemento corrente è un file
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();
                size_t dotIndex = filename.find_last_of('.');
                if (dotIndex == std::string::npos) {
                    // Se non è presente un punto, non c'è estensione
                    continue; // Passa al prossimo file
                }
                // Estrae l'estensione dal nome del file
                std::string fileExtension = filename.substr(dotIndex + 1);
                // Controlla se l'estensione è "log"
                if (fileExtension == "log") {
                    logInfo("Tester", "Monitoring file " + entry.path().filename().string());
                    // Costruisci il comando completo includendo il parametro
                    std::string executable = "./bin/log_monitor";
                    std::string parameter = entry.path().filename().string(); // Passa il percorso completo del file
                    std::string command = executable + " \"" + parameter + "\"";
                    // Esegui il comando
                    int exit_code = std::system(command.c_str());
                    // Verifica il codice di uscita del comando
                    if (exit_code != 0) {
                        logError("Tester", "Errore durante l'esecuzione del monitor: " + std::to_string(exit_code));
                    }
                }
            }
        }
    } catch (const fs::filesystem_error& ex) {
        logError("Tester", ex.what());
    }
}

// ./bin/tower_tester EXECUTION_TIME DRONE_COUNT WIDTH HEIGHT
int main(int argc, char* argv[]) {
    logVerbose(true);
    logOpen("tower_tester.log");
    int executionTime = (argc >= 2)? std::stoi(argv[1]) : 5 * 60;
    int droneCount = (argc >= 3)? std::stoi(argv[2]) : 100;
    int width = (argc >= 4)? std::stoi(argv[3]) : 6000;
    int height = (argc >= 5)? std::stoi(argv[4]) : 6000;
    pid_t tpid = fork();
    if (tpid < 0) {
        logError("Tester", "Can't create tower process");
        exit(-1);
    } else if (tpid == 0) {
        // Child -> run tower
        const char *argv[] = {"./bin/tower", std::to_string(droneCount).c_str(), std::to_string(width).c_str(), std::to_string(height).c_str(), NULL};
        // Sostituisce il processo figlio con tower_exe
        int code = execvp(argv[0], (char *const *)argv);
        if (code == -1) {
            logError("Tester", "Errore nell'esecuzione di execvp: " + std::string(strerror(errno)));
            exit(-1);
        }
    } else {
        // Father process -> wait executionTime
        logInfo("Tester", "TOWER on " + std::to_string(tpid));
        logInfo("Tester", "Waiting end of simulation");
        sleep(executionTime);
        kill(tpid, SIGINT);
        // Wait all process to finish
        while (true) {
            int status;
            pid_t done = wait(&status);
            if (done == -1) {
                if (errno == ECHILD) break; // no more child processes
                else {
                    logInfo("Tester", std::to_string(done) + " exited with status " + std::to_string(status));
                }
            }
        }
        checkLogs();
    }
    return 0;
}