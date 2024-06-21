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

int main(int argc, char* argv[]) {

    std::string command = "./bin/tower_exe"; 
    // Esegui il comando utilizzando std::system
    int exit_code = std::system(command.c_str());

    // Verifica il codice di uscita del comando
    if (exit_code == 0) {
        std::cout << "Comando eseguito con successo." << std::endl;
    } else {
        std::cerr << "Errore nell'esecuzione del comando." << std::endl;
    }

    pid_t pid = fork(); // Creazione di un processo figlio usando fork()

    if (pid < 0) {
        std::cerr << "Errore nella creazione del processo figlio.\n";
        return 1; // Esce dal programma in caso di errore nella creazione del processo
    }

    if (pid == 0) {
        // Questo è il processo figlio
        std::cout << "Sono il processo figlio, PID: " << getpid() << "\n";
        createThreadsInProcess(); // Chiama la funzione per creare e gestire i thread nel processo figlio
    } else {
        // Questo è il processo padre
        std::cout << "Sono il processo padre, PID: " << getpid() << "\n";
        wait(nullptr); // Processo padre aspetta la terminazione del processo figlio
    }
    
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

    return 0; 
}
