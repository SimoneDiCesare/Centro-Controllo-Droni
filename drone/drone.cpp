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

Drone::Drone(long long id) : messageCounterLock() {
    this->id = id;
    this->posX = 0;
    this->posY = 0;
    this->batteryAutonomy = 1800;
    this->batteryLife = 1800;
    this->state = DroneState::WAITING;
    this->channel = nullptr;
    this->running = false;
    this->messageCounter = 0;
    this->destX = 0;
    this->destY = 0;
    this->velocity = 0;
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
    bool sendMessage = false;

    while(true){
        if (this->destX != this->posX || this->destY != this->posY){
            sendMessage = false;
            nowTime = Time::nanos();
            

            //controlla se riesci a tornare alla torre
            float dist = std::sqrt((this->posX - 150)*(this->posX - 150) + (this->posY - 150)*(this->posY - 150)) * 20;
            float metriSec = this->velocity / 3.6;
            float percorsoDisp = this->batteryAutonomy * metriSec;
            
            if (dist + 1 <= percorsoDisp){
                long long delta = (nowTime - lastTime) * 1000;
                this->batteryAutonomy -= delta;

                if(this->posX == this->destX){
                    int upDown;
                    if(destX > posX){
                        upDown = 1;
                    }else{
                        upDown = -1;
                    }

                    long long dX = delta * metriSec * upDown;
                    if (this->destX < this->posX + dX){
                        this->posX = this->destX;
                    } else {
                        this->posX = this->posX + dX;
                    }

                }else if (this->posY == this->destY){
                    long long delta = (nowTime - lastTime) * 1000;
                    
                    int upDown;
                    if(destY > posY){
                        upDown = 1;
                    }else{
                        upDown = -1;
                    }

                    if(this->posY == this->destY){
                        long long dY = delta * metriSec * upDown;
                        if (this->destY < this->posY + dY){
                            this->posY = this->destY;
                        }else{
                            this->posY = this->posY + dY;
                        }
                    }

                }else{
                    long long delta = (nowTime - lastTime) * 1000;
                    long long arcTang = std::atan((this->posY - this->destY) / (this->posX - this->destX));
                    long long speedX = std::cos(arcTang);
                    long long speedY = std::sin(arcTang);

                    long long dX = speedX * delta;
                    long long dY = speedY * delta;

                    if(this->destX < this->posX + dX || this->destY < this->posY + dY ){
                        this->posX = this->destX;
                        this->posY = this->destY;
                    }else{
                        this->posX = this->posX + dX;
                        this->posY = this->posY + dY;
                    }
                }

            }else{
                RetireMessage  *message = new RetireMessage(this->generateMessageId());
                this->channel->sendMessageTo(0, message);
                this->destX = 150;
                this->destY = 150;
            }
        }else{
            sendMessage = true;
            LocationMessage *locMessage = new LocationMessage(this->generateMessageId());
            locMessage->setLocation(this->posX , this->posY);
            this->channel->sendMessageTo(0, locMessage);
        }
        lastTime = nowTime;
    }
}

void Drone::moveTo(int x, int y) {
    this->accelerate(30);
    this->destX = x * 20;
    this->destY = y * 20;
}

// Logically valid? Consider a step approach
void Drone::accelerate(int amount) {
    this->velocity = amount;
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
            this->channel->sendMessageTo(id, ping);
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
            this->channel->sendMessageTo(id, inf);
            break;
        }
        case 3: {
            // chiedere 
            logi("Location Message");
            LocationMessage *locMes = dynamic_cast<LocationMessage*>(message);
            moveTo(locMes->getX() , locMes->getY());

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
    this->posX = posX;
}

void Drone::setPosY(int posY) {
    this->posY = posY;
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
    return this->posX;
}

int Drone::getPosY() {
    return this->posY;
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