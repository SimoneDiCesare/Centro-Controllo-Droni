#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <filesystem>
namespace fs = std::filesystem;


// Numero di thread da creare per ogni processo
const int NUM_THREADS = 5;

// Funzione eseguita da ciascun thread
void* threadFunction(void* arg) {
    long threadId = (long) arg;
    std::cout << "Thread " << threadId << " è in esecuzione\n";
    std::string command = "./bin/drone_exe";
    int exit_code = std::system(command.c_str()); // Esegui il comando system per avviare il processo ./bin/drone_exe
    return nullptr; // Termina il thread
}

// Funzione per creare e gestire i thread nel processo figlio
void createThreadsInProcess() {
    pthread_t threads[NUM_THREADS]; // Array di thread

    // Ciclo per creare NUM_THREADS thread
    for (long i = 0; i < NUM_THREADS; ++i) {
        // Creazione del thread
        if (pthread_create(&threads[i], nullptr, threadFunction, (void*)i) != 0) {
            std::cerr << "Errore nella creazione del thread " << i << "\n";
            exit(1); // Esce dal programma in caso di errore nella creazione del thread
        }
    }

    // Ciclo per attendere la terminazione di tutti i thread
    for (int i = 0; i < NUM_THREADS; ++i) {
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
                    std::cout << "Trovato file .log: " << entry.path() << std::endl;
                    std::cout << "Trovato file .log: " << entry.path().filename().string() << std::endl;
                    // Costruisci il comando completo includendo il parametro
                    std::string executable = "./bin/monitor_exe";
                    std::string parameter = entry.path().filename().string(); // Passa il percorso completo del file

                    std::string command = executable + " " + parameter;

                    // Esegui il comando
                    int exit_code = std::system(command.c_str());

                    // Verifica il codice di uscita del comando
                    if (exit_code == 0) {
                        std::cout << "Comando eseguito con successo." << std::endl;
                    } else {
                        std::cerr << "Errore nell'esecuzione del comando." << std::endl;
                    }
                }
            }
        }
    } catch (const fs::filesystem_error& ex) {
        std::cerr << "Errore durante la lettura della directory: " << ex.what() << std::endl;
    }
}

int main(int argc, char* argv[]) {
    pid_t tpid = fork();
    if (tpid < 0) {
        std::cout << "Errore nella creazione del processo TORRE\n";
        return -1;
    } else if (tpid == 0) {
        // Child -> run tower
        // TODO: Generate tower params
        const char *argv[] = {"./bin/tower_gui", NULL};

        // Sostituisce il processo figlio con tower_exe
        if (execvp(argv[0], (char *const *)argv) == -1) {
            std::cerr << "Errore nell'esecuzione di execvp\n";
            return -1;
        }
    } else {
        // Father -> spawn drones
        std::cout << "TORRE on " << tpid << "\n";
        pid_t dpid = fork();
        if (dpid < 0) {
            // Can't spawn drones -> interrupt tower
            std::cout << "Errore nella creazione del processo DRONI\n";
            kill(tpid, SIGINT);
            return -1;
        } else if (dpid == 0) {
            // Child -> spawn drones
            createThreadsInProcess();
        } else { // Father
            std::cout << "DRONI on " << dpid << "\n";
            // Wait 30'' of simulation, then interrupt 
            sleep(3 * 60);
            kill(tpid, SIGINT);
            // Wait all process to finish
            while (true) {
                int status;
                pid_t done = wait(&status);
                if (done == -1) {
                    if (errno == ECHILD) break; // no more child processes
                    else {
                        std::cout << done << " exited with status " << status << "\n";
                    }
                }
            }
            checkLogs();
        }
    }
    return 0; 
}
