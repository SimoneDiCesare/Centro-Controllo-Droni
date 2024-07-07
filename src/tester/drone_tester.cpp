#include <random>
#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <filesystem>
#include <cstring>
#include "log.hpp"
namespace fs = std::filesystem;

#define MAX_DRONES 100

int NUMS_DRONES = MAX_DRONES;
float eS = 1;

void handleSignal(int signal) {
    switch (signal) {
        case SIGINT:
            logInfo("Tester", "Waiting Simulation to finish");
            break;
        case SIGTERM:
            logInfo("Tester", "Waiting Simulation to finish");
            break;
        default:
            logDebug("Tower", "Signal not handled: " + std::to_string(signal));
            break;
    }
}

// Funzione eseguita da ciascun thread
void* threadFunction(void* arg) {
    long threadId = (long) arg;
    logInfo("Tester", "Thread " + std::to_string(threadId) + " in esecuzione");
    std::string executable = "./bin/drone";
    std::string parameter = std::to_string(eS);
    std::string command = executable + " " + parameter;
    int exit_code = std::system(command.c_str());
    return nullptr; // Termina il thread
}

// Funzione per creare e gestire i thread nel processo figlio
void createThreadsInProcess() {
    pthread_t threads[NUMS_DRONES]; // Array di thread

    // Ciclo per creare NUM_THREADS thread
    for (long i = 0; i < NUMS_DRONES; ++i) {
        // Creazione del thread
        int code = pthread_create(&threads[i], nullptr, threadFunction, (void*)i);
        if (code != 0) {
            logError("Tester", "Errore nella creazione del thread " + std::to_string(i) + ": " + std::to_string(code));
            // exit(1); // Esce dal programma in caso di errore nella creazione del thread
        }
    }
    // Ciclo per attendere la terminazione di tutti i thread
    for (int i = 0; i < NUMS_DRONES; ++i) {
        pthread_join(threads[i], nullptr); // Attende la terminazione del thread i
    }
}

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

// ./bin/tester_exe EXECUTION_SPEED EXECUTION_TIME DRONE_COUNT
int main(int argc, char* argv[]) {
    logVerbose(true);
    logOpen("tester.log");
    // Register for optimal handling
    signal(SIGINT, handleSignal);
    signal(SIGTERM, handleSignal);
    if (argc >= 2) {
        eS = std::stof(argv[1]);
    }
    int executionTime = 5 * 60;
    if (argc >= 3) {
        executionTime = std::stoi(argv[2]);
    }
    if (argc >= 4) {
        NUMS_DRONES = std::stoi(argv[3]);
    }
    pid_t dpid = fork();
    if (dpid < 0) {
        logError("Tester", "Errore nella creazione del processo DRONE");
        exit(-1);
    } else if (dpid == 0) {
        // Child -> spawn drones
        createThreadsInProcess();
    } else {
        // Father
        logInfo("Tester", "DRONI on " + std::to_string(dpid));
        sleep(executionTime);
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
        logInfo("Tester", "Fine Esecuzione");
        checkLogs();
    }
    return 0; 
}

