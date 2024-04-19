#include "ClassDroni.hpp"
#include <iostream>
#include "channel.hpp"
#include <csignal>
#include <cstdlib>
#include <thread>
#include <vector>


Droni::Droni(int id, int X, int Y, droniState stato, int batteria)
{   
    ID = id;
    MaxPwr = 100;
    Vel = 0; // i droni partono tutti dalla base ?! ma di sicuro partano da fermi 
    RoA = 6;
    Bat = batteria; //come simulare la batteria che si esauruisce la batteria ? ldopo ogni movimento
    PosX = X;
    PosY = Y;
    Stt = stato;
}

Droni::~Droni(){
    running = false;
    delete this->channel;
}

void Droni::Start(){
    // controlla se il canale si è conesso bene 
    if (!this->channel->isConnected()) {
        std::cout << "Can't start tower without a connected channel!\n";
        return;
    }
    
    std::vector<std::thread> threads;

    this->running = true;
    while (this->running) {
        // Attende un messaggio dal canale della torre
        Message *message = this->channel->awaitMessage();
        
        // TODO: - Implement Multithreading for requests
        //       - Implement Signals for Terminating Process
        //       - Implement Response Logic
        
        // Se non viene ricevuto alcun messaggio
        if (message == NULL) {
            // Possibile stampare un messaggio di debug (commentato qui)
        } else {
            // Se viene ricevuto un messaggio, stampa le informazioni relative ad esso
            std::cout << "Received Message: m:" << message->getChannelId() << ":" << message->getMessageId() << "\n";
            
            // Crea un nuovo thread per gestire il messaggio
            threads.emplace_back(&Droni::handleMessage, this, message);
        }
        
        // Rimuove i thread terminati dal vettore di thread
        for (auto it = threads.begin(); it != threads.end(); ) {
            if (it->joinable()) {
                it++;
            } else {
                it = threads.erase(it);
            }
        }
    }
    // Attende che tutti i thread creati abbiano terminato l'esecuzione prima di terminare
    for (auto& thread : threads) {
        thread.join();
    }
    // TODO: - Disconnect and release drones
    //       - Exit with received signal 
}

void Droni::connect(std::string ip, int port) {
    // Crea un nuovo oggetto Channel e lo assegna al membro "channel" della classe Tower
    this->channel = new Channel(ID);
    
    // Tenta di connettere il canale all'indirizzo IP e alla porta specificati
    bool connected = this->channel->connect(ip, port);
    
    // Verifica se la connessione è riuscita
    if (!connected) {
        // Se la connessione non è riuscita, stampa un messaggio di avviso
        std::cout << "Can't create channel for Droni\n";
    }
}


int Droni::SetID(int id){
    if (id == 0) {
        // Se l'ID è 0, restituisci un codice di errore
        return -1;
    } 
    // Imposta l'ID con il valore fornito
    ID = id;
    // Restituisci 0 per indicare che l'operazione è avvenuta con successo
    return 0;
}

void Droni::SetStt(droniState Stato){
    Stt = Stato;
}

void Droni::Movimento(int X, int Y){
    // aggiungere controllo per non fare uscire i droni dai confini della griglia 
    PosX = X;
    PosY = Y;
    // quando si chiama il moviemnto di sicuro accelera 
    Accelerazione(30);
}

int Droni::GetBatteria(){
    return Bat;
}

droniState Droni::GetStato(){
    return Stt;
}

int Droni::GetVelocita(){
    return Vel;
}

int Droni::GetRaggio(){
    return RoA;
}

//chiedere se vale la pena fare una struttura per rappresentare la posizione
int Droni::GetPosX(){
    return PosX;
}

int Droni::GetPosY(){
    return PosY;
}

void Droni::Accelerazione(int vel){
    Vel = vel;
}

void Droni::handleSignal(int signal) {
    std::cout << "Received Signal: "  << signal << "\n";
    Droni::running = false;
}

void Droni::handleMessage(Message* message) {
    // Verifica se il puntatore al messaggio è nullo
    if (message == NULL) {
        // Se il messaggio è nullo, non c'è nulla da gestire, quindi la funzione esce immediatamente
        return;
    }
    
    // TODO: - Process message information
    // Stampa l'ID del messaggio
    std::cout << message->getMessageId() << "\n";
    
    // Elimina il messaggio per evitare memory leak
    delete message;
}
