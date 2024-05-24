#include "drone.hpp"
#include <iostream>
#include "channel.hpp"
#include <csignal>
#include <cstdlib>
#include <thread>
#include <vector>
#include <sys/time.h>
#include "time.hpp"
#include "log.hpp"
#include "globals.hpp"
#include <cmath>

// Drone class

// Utility functions

void Drone::logi(std::string message) {
    logInfo("D" + std::to_string(this->id), message);
}

void Drone::loge(std::string message) {
    logError("D" + std::to_string(this->id), message);
}

void Drone::logd(std::string message) {
    logDebug("D" + std::to_string(this->id), message);
}

// Can be simplified in just Time::nanos()?
long long Drone::createId() {
    return Time::nanos();
}

// Block and update id message 
long long Drone::generateMessageId() {
    this->messageCounterLock.lock();
    long long id = this->messageCounter;
    this->messageCounter++;
    this->messageCounterLock.unlock();
    return id;
}

// Constructors and Deconstructor

Drone::Drone() : Drone(Drone::createId()){

}

Drone::Drone(long long id) : messageCounterLock(), destinationLock(), positionLock() {
    this->state = WAITING;
    this->id = id;
    this->posX = 0;
    this->posY = 0;
    this->batteryAutonomy = randomBattery();
    this->batteryLife = this->batteryAutonomy;
    this->state = DroneState::WAITING;
    this->channel = nullptr;
    this->running = false;
    this->messageCounter = 0;
    this->destX = 0;
    this->destY = 0;
    this->velocity = 30; // km/h
}

Drone::~Drone() {
    if (this->channel != nullptr) {
        delete this->channel;
    }
}

// Drone Functions

bool Drone::connectChannel(std::string ip, int port) {
    this->channel = new Channel(this->id);
    bool connected = this->channel->connect(ip, port);
    if (connected) {
        logi("Drone connected on redis channel " + std::to_string(this->id));
    } else {
        loge("Can't create channel for drone");
    }
    return connected;
}

bool Drone::connectToTower() {
    if (!this->channel->isUp()) {
        loge("Can't start drone without a connection!");
        return false;
    }
    // Setting timeout for redis responses
    AssociateMessage message(this->generateMessageId(), this->id);
    this->channel->sendMessageTo(0, &message);
    Message *response = this->channel->awaitMessage(10);
    if (response == nullptr) {
        loge("Can't enstablish connection to tower!");
        delete response;
        return false;
    }
    if (AssociateMessage *association = dynamic_cast<AssociateMessage*>(response)) {
        long long associateId = association->getDroneId();
        if (this->id != associateId) {
            logi("New Id: " + std::to_string(associateId));
            this->id = associateId;
            this->channel->setId(this->id);
        }
        this->towerX = association->getTowerX() * 20;
        this->towerY = association->getTowerY() * 20;
        this->positionLock.lock();
        this->posX = this->towerX;
        this->posY = this->towerY;
        this->positionLock.unlock();
        this->destinationLock.lock();
        this->destX = this->posX;
        this->destY = this->posY;
        this->destinationLock.unlock();
        // Consume message
        bool deleted = this->channel->removeMessage(association);
        if (deleted) {
            logi("Consumed message m:" + association->getFormattedId());
        }  else {
            logi("Can't delete message m:" + association->getFormattedId());
        }
    }
    delete response;
    // TODO: Send infos to tower
    
    return true;
}

void Drone::start() {
    // Enstablish connection to tower
    bool connectedToTower = this->connectToTower();
    if (!connectedToTower) {
        loge("Can't connect to tower!");
        return;
    }
    logi("Connected to tower");
    
    std::thread moveThread(&Drone::movement, this);

    // Start loop
    std::vector<std::thread> threads;
    this->running = true;
    while (this->running) {
        Message *message = this->channel->awaitMessage();
        if (message == nullptr) {
            // No message received. Maybe ping tower?
            continue;
        }
        logi("Received message m:" + message->getFormattedId());
        threads.emplace_back(&Drone::handleMessage, this, message);
        // Free up completed threads
        for (auto it = threads.begin(); it != threads.end();) {
            if (it->joinable()) {
                it ++;
            } else {
                it = threads.erase(it);
            }
        }
    }
    // Disconnect drone
    // Wait threads to finish
    for (auto &thread : threads) {
        thread.join();
    }
    // TODO: Implement disconnect logic
}

void Drone::movement(){
    long long lastTime = Time::nanos();
    long long nowTime = Time::nanos();
    bool sendMessage = true;
    int posX, posY, destX, destY;
    while(this->running) {
        // Copy values for thread safety and consistency
        this->positionLock.lock();
        posX = this->posX;
        posY = this->posY;
        this->positionLock.unlock();
        this->destinationLock.lock();
        destX = this->destX;
        destY = this->destY;
        this->destinationLock.unlock();
        // Check Destination
        /**
         * Si muove a prescindere dallo stato. Dovrebbe muoversi unicamente se è in
         * stato di monitoring. Fai un check, ed in caso deve muoversi (monitoring o
         * di rientro) allora prosegui con il check sulla destinazione
         */
        if (destX != posX || destY != posY && (getState() != CHARGING || getState() != WAITING)) {
            logi("Moving to (" + std::to_string(destX) + "," + std::to_string(destY) + ")");
            
            setState(MONITORING);
            sendMessage = true;
            nowTime = Time::nanos();
            
            // Check on battery level
            float dist = std::sqrt((posX - this->towerX) * (posX - this->towerX) + (posY - this->towerY) * (posY - this->towerY));
            float margin = 1 * 20; // Blocks of Margin
            float mPerSec = this->velocity / 3.6;
            float availableMeters = this->batteryAutonomy * mPerSec;

            if (dist + margin <= availableMeters && getState() != RETURNING) {
                long long delta = (nowTime - lastTime) * 1000;
                this->batteryAutonomy -= delta;

                if (sendMessage = false){
                    sendMessage = true;
                }
                /**
                 * Perché controlli se posX = destX, e poi controlli se posX < | > destX?
                 * lo fai anche per le Y. Ricontrolla bene la logica dietro i movimenti.
                 * Non so se è giusta o sbagliata eh, ma non mi convincono sti controlli.
                 * Vedi poi se puoi accorpare le diverse cose in funzioni, perché più parti
                 * di questa funzione sono molto simili tra loro. Magari lavora con delle refence.
                 * n più, i controlli con i dx e i dy non mi sembrano correti, perché se vai
                 * da sinistra a destra allora posX+dx <= destX, ma se vai da destra a sinistra
                 * hai che posX+dx >= destX, perché ti muovi da un punto più lontano ad un punto più
                 * vicino. La stessa cosa vale con le Y, se ti muovi dal basso all'alto
                 * posY+dy >= destY sempre.
                 */
                if (posX == destX) { // X Movement
                    int upDown;
                    if (destX > posX) {
                        upDown = 1;
                    } else {
                        upDown = -1;
                    }
                    long long dx = delta * mPerSec * upDown;
                    if (destX < posX + dx) {
                        posX = destX;
                    } else {
                        posX = posX + dx;
                    }
                } else if (posY == destY) { // Y Movement
                    long long delta = (nowTime - lastTime) * 1000;
                    int upDown;
                    if (destY > posY) {
                        upDown = 1;
                    } else {
                        upDown = -1;
                    }
                    if (posY == destY) {
                        long long dy = delta * mPerSec * upDown;
                        if (destY < posY + dy) {
                            posY = destY;
                        } else {
                            posY = posY + dy;
                        }
                    }
                } else { // Diagonal Movements
                    long long delta = (nowTime - lastTime) * 1000;
                    long long arcTang = std::atan((posY - destY) / (posX - destX));
                    long long speedX = std::cos(arcTang);
                    long long speedY = std::sin(arcTang);
                    long long dx = speedX * delta;
                    long long dy = speedY * delta;
                    if (destX < posX + dx) {
                        posX = destX;
                    } else {
                        posX = posX + dx;
                    }
                    if (destY < posY + dy) {
                        posY = destY;
                    } else {
                        posY = posY + dy;
                    }
                }
                this->positionLock.lock();
                this->posX = posX;
                this->posY = posY;
                this->positionLock.unlock();

            }else if (getState() == RETURNING) // esegue gli stessi calcoli del viaggio in diagonale
            {
                long long delta = (nowTime - lastTime) * 1000;
                    long long arcTang = std::atan((posY - destY) / (posX - destX));
                    long long speedX = std::cos(arcTang);
                    long long speedY = std::sin(arcTang);
                    long long dx = speedX * delta;
                    long long dy = speedY * delta;
                    if (destX < posX + dx) {
                        posX = destX;
                    } else {
                        posX = posX + dx;
                    }
                    if (destY < posY + dy) {
                        posY = destY;
                    } else {
                        posY = posY + dy;
                    }
            }
             else {
                /**
                 * Qui la funzione arriva ogni qualvolta che c'è bisogno di rientrare,
                 * facendo rimanere il drone immobile se ha bisogno di rientrare ed
                 * intasando il canale di redis. Fai il check per controllare se sta
                 * rientrando, e muoviti a prescindere dai metri che puoi percorrere
                 * se devi rientrare.
                 */
                setState(RETURNING);
                logi("Need to Retire");
                RetireMessage *retire = new RetireMessage(this->generateMessageId());
                this->channel->sendMessageTo(0, retire);
                this->destinationLock.lock();
                this->destX = this->towerX;
                this->destY = this->towerY;
                this->destinationLock.unlock();
            }
        } else if (sendMessage) { // manda il mesaggio quando arriva alla posizione obbiettivo
            sendMessage = false;
            LocationMessage *location = new LocationMessage(this->generateMessageId());
            location->setLocation(posX, posY);
            this->channel->sendMessageTo(0, location);
            delete location;
        } else if (destX == 150 == posX  && destY == 150 == posY && (getState() != CHARGING || getState() != WAITING)) {
            setState(CHARGING);
        }
         else if (getState() == CHARGING) // se lo stato è in carica allora ricarica la batteria 
        {
            long long delta = (nowTime - lastTime) * 1000;
            
            if (getBatteryAutonomy() + delta > getBatteryLife())
            {
                setBatteryLife(this->batteryLife);
                setState(WAITING);
            }else{
                setBatteryLife(this->batteryAutonomy += delta);
            }
            
        }
        lastTime = nowTime;
    }
}

long long Drone::randomBattery(){
    srand(time(NULL));
    int minuti = rand() % 31;
    long long _ = minuti * 60;
    return _ + 9000;
}

void Drone::moveTo(int x, int y) {
    this->destinationLock.lock();
    this->destX = x * 20;
    this->destY = y * 20;
    this->destinationLock.unlock();
    this->state = MONITORING;
    logi("Setting destination to (" + std::to_string(this->destX) + "," + std::to_string(this->destY) + ")");
}

void Drone::handleMessage(Message *message) {
    if (message == nullptr) {
        return;
    }
    logi("Handling message m:" + message->getFormattedId());
    // Message handling logic
    int type = message->getType();
    switch (type) {
        case 0: {
            logi("Reassociating Drone");
            break;
        }
        case 1:{
            logi("Ping!");
            PingMessage *ping = new PingMessage(generateMessageId());
            this->channel->sendMessageTo(0, ping);
            break;
        }
        case 2: {
            logi("Drone Info Message");
            DroneInfoMessage *inf = new DroneInfoMessage(generateMessageId());
            inf->setDroneId(id);
            inf->setBatteryAutonomy(batteryAutonomy);
            inf->setBatteryLife(batteryLife);
            inf->setDroneId(id);
            inf->setPosX(posX);
            inf->setPosY(posY);
            inf->setState(state);
            this->channel->sendMessageTo(0, inf);
            break;
        }
        case 3: {
            // chiedere 
            logi("Location Message");
            LocationMessage *locMes = dynamic_cast<LocationMessage*>(message);
            moveTo(locMes->getX(), locMes->getY());

            //LocationMessage *loc = new LocationMessage(generateMessageId());
            //this->channel->sendMessageTo(id, loc);
            break;
        }
        // case 4: La torre non deve mandare un RetireMessage
        case 5: {
            logi("Disconnect Message");
            break;
        }
        default: {
            loge("Unhandled message type: " + std::to_string(type));
            break;
        }
    }
    // Consume message
    bool deleted = this->channel->removeMessage(message);
    if (deleted) {
        logi("Consumed message m:" + message->getFormattedId());
    }  else {
        logi("Can't delete message m:" + message->getFormattedId());
    }
    delete message;
}

// Setter

void Drone::setPosX(int posX) {
    this->positionLock.lock();
    this->posX = posX;
    this->positionLock.unlock();
}

void Drone::setPosY(int posY) {
    this->positionLock.lock();
    this->posY = posY;
    this->positionLock.unlock();
}

void Drone::setBatteryAutonomy(long long batteryAutonomy) {
    this->batteryAutonomy = batteryAutonomy;
}

void Drone::setBatteryLife(long long batteryLife) {
    this->batteryLife = batteryLife;
}

void Drone::setState(DroneState state) {
    this->state = state;
}

// Getter

long long Drone::getId() {
    return this->id;
}

int Drone::getPosX() {
    this->positionLock.lock();
    int posX = this->posX;
    this->positionLock.unlock();
    return posX;
}

int Drone::getPosY() {
    this->positionLock.lock();
    int posY = this->posY;
    this->positionLock.unlock();
    return posY;
}

long long Drone::getBatteryAutonomy() {
    return this->batteryAutonomy;
}

long long Drone::getBatteryLife() {
    return this->batteryLife;
}

DroneState Drone::getState() {
    return this->state;
}

int Drone::getRangeOfAction() {
    return this->rangeOfAction;
}

int Drone::getVelocity() {
    return this->velocity;
}




/*


int Droni::timeID(){
    struct timeval currentTime;
    gettimeofday(&currentTime, NULL);

    long milliseconds = currentTime.tv_sec * 1000 + currentTime.tv_usec / 1000;
    return milliseconds;
}

Droni::Droni()
{   
    ID = timeID();
    MaxPwr = 100;
    Vel = 0;  
    RoA = 6;
    Bat = 100;
    PosX = 0;
    PosY = 0;
    Stt = WAITING;
}

Droni::~Droni(){
    running = false;
    delete this->channel;
}

void Droni::Start(){

    this->channel->connect();
    AssociateMessage *MessageNewID = new AssociateMessage (0 , ID);
    this->channel->sendMessageTo(0 , *MessageNewID);

    

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
*/