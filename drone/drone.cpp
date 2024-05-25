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
#include <random>
#include <chrono>

// Drone class

// Utility functions

DroneInfoMessage* generateDroneInfoMessage(Drone *drone, long long id) {
    DroneInfoMessage *info = new DroneInfoMessage(id);
    info->setDroneId(drone->getId());
    info->setPosX(drone->getPosX() / 20);
    info->setPosY(drone->getPosY() / 20);
    info->setBatteryAutonomy(drone->getBatteryAutonomy());
    info->setBatteryLife(drone->getBatteryLife());
    std::cout << drone->getState() << "\n";
    info->setState(drone->getState());
    return info;
}

long long generateRandomRechargeTime() {
    long long minSeconds = 2 * 60 * 60;
    long long maxSeconds = 3 * 60 * 60;
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 generator(seed);
    std::uniform_int_distribution<long long> distribution(minSeconds, maxSeconds);
    return distribution(generator);
}

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

Drone::Drone(long long id) : messageCounterLock(), destinationLock(),
        positionLock(), stateMutex() {
    this->setState(READY);
    this->id = id;
    this->posX = 0;
    this->posY = 0;
    this->rechargeTime = generateRandomRechargeTime();
    this->charge = 0;
    this->batteryAutonomy = 30 * 60;
    this->batteryLife = this->batteryAutonomy;
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
    
    std::thread moveThread(&Drone::behaviourLoop, this);

    // Start loop
    std::vector<std::thread> threads;
    this->running = true;
    this->setState(READY);
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

void Drone::checkBattery() {
    DroneState state = this->getState();
    if (this->batteryAutonomy <= 0) {
        switch (state) {
            case WAITING:
            case MONITORING:
            case RETURNING:
                this->batteryAutonomy = 0;
                loge("Dead for 0 Battery");
                this->setState(DEAD);
                std::exit(-1); // Simulate a completely shutdown
            default:
                break;
        }
        
    }
    if (state == WAITING || state == MONITORING) {
        double posX, posY;
        this->positionLock.lock();
        posX = this->posX;
        posY = this->posY;
        this->positionLock.unlock();
        double dist = (towerX - posX) * (towerX - posX);
        dist += (towerY - posY) * (towerY - posY);
        dist = std::sqrt(dist);
        double margin = .5 * 20; // Blocks Margin
        double meterPerSeconds = this->velocity / 3.6;
        if ((dist + margin) / meterPerSeconds >= this->batteryAutonomy) {
            // Low Batter, returning
            logi("Need to return -> " + std::to_string((dist + margin) / meterPerSeconds) + " >= " + std::to_string(this->batteryAutonomy));
            this->setState(RETURNING);
            RetireMessage *message = new RetireMessage(this->generateMessageId());
            this->channel->sendMessageTo(0, message);
            DroneInfoMessage *info = generateDroneInfoMessage(this, this->generateMessageId());
            this->channel->sendMessageTo(0, info);
            delete info;
            delete message;
        }
    }   
}

void Drone::move(double delta) {
    if (delta == 0) {
        // Skip nullable movements
        return;
    }
    double posX, posY, destX, destY;
    double velocity = this->velocity / 3.6;
    // Copy values for thread safety and consistency
    this->positionLock.lock();
    posX = this->posX;
    posY = this->posY;
    this->positionLock.unlock();
    this->destinationLock.lock();
    destX = this->destX;
    destY = this->destY;
    this->destinationLock.unlock();
    if (posX == destX && posY == destY) {
        // logi("Waiting New Location");
        // Arrived at destination.
        LocationMessage *location = new LocationMessage(this->generateMessageId());
        // Normalize Positon
        location->setLocation(posX / 20, posY / 20);
        this->channel->sendMessageTo(0, location);
        delete location;
        if (posX == this->towerX && posY == this->towerY) {
            logi("Charging Battery");
            this->setState(CHARGING);
            DroneInfoMessage *info = generateDroneInfoMessage(this, this->generateMessageId());
            this->channel->sendMessageTo(0, info);
            delete info;
        } else {
            logi("Waiting new Location");
            this->setState(WAITING);
        }
        return;
    }
    // Calculate Speed components via Normalized Vector
    double deltaX = destX - posX;
    double deltaY = destY - posY;
    double dist = std::sqrt(deltaX * deltaX + deltaY * deltaY);
    double speedX = deltaX / dist;
    double speedY = deltaY / dist;
    double dx = speedX * delta * velocity;
    double dy = speedY * delta * velocity;
    posX += dx;
    posY += dy;
    // Check Bounderies
    if (dx > 0 && destX < posX) { // Going right -> check right limit
        posX = destX;
    } else if (dx < 0 && destX > posX) { // Going left -> check left limit
        posX = destX;
    }
    if (dy > 0 && destY < posY) { // Going down -> check down limit
        posY = destY;
    } else if (dy < 0 && destY > posY) { // Going up -> check up limit
        posY = destY;
    }
    this->positionLock.lock();
    this->posX = posX;
    this->posY = posY;
    this->positionLock.unlock();
    this->batteryAutonomy -= delta;
}

void Drone::behaviourLoop() {
    this->running = true;
    long long lastTime = Time::nanos();
    long long nowTime;
    double delta;
    while (this->running) {
        nowTime = Time::nanos();
        delta = (nowTime - lastTime) / 1e9;
        this->checkBattery();
        switch(this->getState()) {
            case CHARGING:
                this->charge += delta;
                if (this->charge >= this->rechargeTime) {
                    logi("Charge Ended");
                    this->batteryAutonomy = this->batteryLife;
                    this->charge = 0;
                    this->setState(READY);
                    DroneInfoMessage *info = generateDroneInfoMessage(this, this->generateMessageId());
                    this->channel->sendMessageTo(0, info);
                    delete info;
                }
                break;
            case READY:
                // Waiting to be assigned.
                break;
            case WAITING:
                // Waiting a new location.
                // Consume Battery
                this->batteryAutonomy -= delta;
                break;
            case MONITORING:
                // Move to location and monitor.
                move(delta);
                break;
            case RETURNING:
                // Return to tower.
                move(delta);
                break;
            default:
                break;
            
        }
        lastTime = nowTime;
    }
}

void Drone::moveTo(int x, int y) {
    DroneState state = this->getState();
    if (state == READY || state == MONITORING || state == WAITING) {
        this->destinationLock.lock();
        this->destX = x * 20;
        this->destY = y * 20;
        this->destinationLock.unlock();
        this->setState(MONITORING);
        logi("Setting destination to (" + std::to_string(this->destX) + "," + std::to_string(this->destY) + ")");
    }
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
            DroneInfoMessage *info = generateDroneInfoMessage(this, this->generateMessageId());
            this->channel->sendMessageTo(0, info);
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

void Drone::setPosX(double posX) {
    this->positionLock.lock();
    this->posX = posX;
    this->positionLock.unlock();
}

void Drone::setPosY(double posY) {
    this->positionLock.lock();
    this->posY = posY;
    this->positionLock.unlock();
}

void Drone::setBatteryAutonomy(double batteryAutonomy) {
    this->batteryAutonomy = batteryAutonomy;
}

void Drone::setBatteryLife(double batteryLife) {
    this->batteryLife = batteryLife;
}

void Drone::setState(DroneState state) {
    this->stateMutex.lock();
    this->state = state;
    this->stateMutex.unlock();
}

// Getter

long long Drone::getId() {
    return this->id;
}

double Drone::getPosX() {
    this->positionLock.lock();
    int posX = this->posX;
    this->positionLock.unlock();
    return posX;
}

double Drone::getPosY() {
    this->positionLock.lock();
    int posY = this->posY;
    this->positionLock.unlock();
    return posY;
}

double Drone::getBatteryAutonomy() {
    return this->batteryAutonomy;
}

double Drone::getBatteryLife() {
    return this->batteryLife;
}

DroneState Drone::getState() {
    this->stateMutex.lock();
    DroneState state = this->state;
    this->stateMutex.unlock();
    return state;
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