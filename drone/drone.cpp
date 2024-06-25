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
    info->setPosX(drone->getPosX() / GRID_FACTOR);
    info->setPosY(drone->getPosY() / GRID_FACTOR);
    info->setBatteryAutonomy(drone->getBatteryAutonomy());
    info->setChargeTime(drone->getRechargeTime());
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

void Drone::logw(std::string message) {
    logWarning("D" + std::to_string(this->id), message);
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
    this->executionSpeed = 0;
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
        this->towerX = association->getTowerX() * GRID_FACTOR;
        this->towerY = association->getTowerY() * GRID_FACTOR;
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

void Drone::start(float executionSpeed) {

    setExecutionSpeed(executionSpeed);

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
        location->setLocation(posX / GRID_FACTOR, posY / GRID_FACTOR);
        this->channel->sendMessageTo(0, location);
        delete location;
        if (posX == this->towerX && posY == this->towerY && this->getState() == RETURNING) {
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
    posX += dx * this -> executionSpeed;
    posY += dy * this -> executionSpeed;
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
    this->batteryAutonomy -= delta * this -> executionSpeed;
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
                this->charge += delta * this -> executionSpeed;
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
                this->batteryAutonomy -= delta * this -> executionSpeed;
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
        this->destX = x * GRID_FACTOR;
        this->destY = y * GRID_FACTOR;
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
            DisconnectMessage *disconnect = new DisconnectMessage(this->generateMessageId());
            this->channel->sendMessageTo(0, disconnect);
            delete disconnect;
            bool channelFlushed = this->channel->flush();
            if (channelFlushed) {
                logi("Channel Flushed!");
            } else {
                logw("Can't flush redis channel");
            }
            std::exit(0);
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

void Drone::setExecutionSpeed(float es){
    this -> executionSpeed = es;
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

long long Drone::getRechargeTime() {
    return this->rechargeTime;
}

int Drone::getVelocity() {
    return this->velocity;
}
