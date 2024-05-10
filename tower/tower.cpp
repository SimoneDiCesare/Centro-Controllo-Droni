#include "tower.hpp"
#include "channel.hpp"
#include "postgresql.hpp"
#include "log.hpp"
#include "time.hpp"
#include <iostream>
#include <csignal>
#include <cstdlib>
#include <thread>
#include <vector>
#include <mutex>
#include <tuple>

long long Tower::generateMessageId() {
    this->messageCounterLock.lock();
    long long id = this->messageCounter;
    this->messageCounter++;
    this->messageCounterLock.unlock();
    return id;
}

// Utility for faster logs

void loge(std::string message) {
    logError("Tower", message);
}

void logi(std::string message) {
    logInfo("Tower", message);
}

// Tower class definitions

bool Tower::running = false;

Tower::Tower() : messageCounterLock() {
    logi("Initializing Tower");
    this->channel = nullptr;
    this->db = nullptr;
    this->messageCounter = 0;
    // Initialize Area
    this->areaWidth = 100;
    this->areaHeight = 100;
    logi("Initializing Area");
    this->area = std::vector<std::vector<int>>();
    this->area.resize(this->areaWidth, std::vector<int>(this->areaHeight, 0));
    logi("Tower Initialized");
}

Tower::~Tower() {
    running = false;
    if (this->channel != nullptr) {
        delete this->channel;
    }
    // TODO: Dealloc Area
}

bool Tower::connectChannel(std::string ip, int port) {
    this->channel = new Channel(0);
    bool connected = this->channel->connect(ip, port);
    if (connected) {
        logi("Tower connected on redis channel 0");
    } else {
        loge("Can't create channel for tower");
    }
    return connected;
}

bool Tower::connectDb(const PostgreArgs args) {
    this->db = new Postgre(args);
    if (this->db->isConnected()) {
        logi("DB Connection Enstablished");
        int x = 10;
        PostgreResult result = this->db->execute(R"(CREATE TABLE IF NOT EXISTS drone (
        id BIGINT PRIMARY KEY NOT NULL,
        x INTEGER NOT NULL,
        y INTEGER NOT NULL,
        battery_autonomy BIGINT,
        battery_life BIGINT,
        dstate DSTATE,
        last_update BIGINT
        ))");
        // CHECK(id > 0)
        if (result.error) {
            logError("DB", result.errorMessage);
            return false;
        }
        logi("Table drone Created");
        result = this->db->execute("TRUNCATE TABLE drone");
        if (result.error) {
            logError("DB", result.errorMessage);
            return false;
        }
        logi("Table drone Cleared");
        return true;
    } else {
        loge("Can't connected to db!");
        return false;
    }
}

std::vector<Drone> Tower::getDrones() {
    std::vector<Drone> drones;
    if (!this->db->isConnected()) {
        loge("Can't retrive connected drones!");
        return drones;
    }
    PostgreResult result = this->db->execute("SELECT id, x, y, battery_autonomy, battery_life, dstate, last_update FROM drone");
    for (const auto& row : result.result) {
        Drone d;
        d.id = row[0].as<long long>();
        d.posX = row[1].as<int>();
        d.posY = row[2].as<int>();
        d.batteryAutonomy = std::chrono::seconds(row[3].as<long long>());
        d.batteryLife = std::chrono::seconds(row[4].as<long long>());
        d.droneState = row[5].as<std::string>();
        d.lastUpdate = std::chrono::seconds(row[6].as<long long>());
        drones.push_back(d);
    }
    logi("Found: " + std::to_string(drones.size()) + " drones");
    return drones;
}

void Tower::calcolateDronePath(Drone drone) {
    // Add algorithm
    // For now pick a random x, y movements
    std::vector<std::tuple<char, int>> locations;
    int locationCount = rand() % 5; // Max 5 locations
    // (0, 0) is the upper-left corner.
    // Faulty algorithm > doesn't check bounds
    int xPos = drone.posX;
    int yPos = drone.posY;
    char c = rand() % 2 == 0? 'x' : 'y';
    int amount;
    if (c == 'x') {
        amount = rand() % this->areaWidth;
    } else {
        amount = rand() % this->areaHeight;
    }
    locations.push_back(std::tuple<char, int>(c, amount));
    // Send Location Message
    PathMessage *message = new PathMessage(this->generateMessageId());
    message->setLocations(locations);
    this->channel->sendMessageTo(drone.id, message);
    delete message;
}

Drone Tower::getDrone(long long id) {
    Drone drone;
    PostgreResult result = this->db->execute("SELECT id, x, y, battery_autonomy, battery_life, dstate, last_update FROM drone WHERE id = " + std::to_string(id));
    for (const auto& row : result.result) {
        if (row[0].as<long long>() == id) {
            drone.id = row[0].as<long long>();
            drone.posX = row[1].as<int>();
            drone.posY = row[2].as<int>();
            drone.batteryAutonomy = std::chrono::seconds(row[3].as<long long>());
            drone.batteryLife = std::chrono::seconds(row[4].as<long long>());
            drone.droneState = row[5].as<std::string>();
            drone.lastUpdate = std::chrono::seconds(row[6].as<long long>());
        }
    }
    return drone;
}

void Tower::checkDrones() {
    std::vector<Drone> drones = this->getDrones();
    auto currentTime = std::chrono::system_clock::now();
    auto timePassed = std::chrono::seconds(0);
    for (const Drone& drone : drones) {
        timePassed = std::chrono::duration_cast<std::chrono::seconds>(currentTime.time_since_epoch()) - drone.lastUpdate;
        logi("Time Passed: " + std::to_string(timePassed.count()));
        if (timePassed > std::chrono::seconds(120)) {
            logi("Pinging drone");
            PingMessage *ping = new PingMessage(generateMessageId());
            this->channel->sendMessageTo(drone.id, ping);
            delete ping;
        } else {
            if (drone.droneState.compare("waiting")) {
                logi("Sending drone to monitoring");
                this->calcolateDronePath(drone);
            }
        }
    }
}

void Tower::droneCheckLoop() {
    while (this->running) {
        std::this_thread::sleep_for(std::chrono::seconds(10));
        logi("Checking Drones");
        this->checkDrones();
    }
}

void Tower::start() {
    if (!this->channel->isUp()) {
        loge("Can't start tower without a connected channel!");
        return;
    }
    // Register signals
    signal(SIGINT, Tower::handleSignal);
    signal(SIGTERM, Tower::handleSignal);
    this->running = true;
    std::vector<std::thread> threads;
    threads.emplace_back(&Tower::droneCheckLoop, this);
    logi("Tower online");
    while (this->running) {
        // 1' of waiting before restarting the cycle
        Message *message = this->channel->awaitMessage(60);
        if (message == nullptr) {
            // If we have no message to handle, we check last updates from drones
            // If a last update is > x second (to decide, maybe 1-5' => 60-300'') -> ping and wait a response
            // If the drone is doing nothing, we commit it to monitor a zone

        } else {
            // Handle message received on another thread, and return to listen
            logi("Received message from Drone " + std::to_string(message->getChannelId()) + ". Type: " + std::to_string(message->getType()));
            threads.emplace_back(&Tower::handleMessage, this, message);
        }
        // Free finished threads
        for (auto it = threads.begin(); it != threads.end(); ) {
            if (it->joinable()) {
                it++;
            } else {
                it = threads.erase(it);
            }
        }
    }
    logi("Powering Off");
    // Await Spawned Threads
    for (auto& thread : threads) {
        thread.join();
    }
    // TODO: - Disconnect and release drones
    //       - Exit with received signal 
}

long long checkDroneId(Postgre* db, long long id) {
    if (db == nullptr || !db->isConnected()) {
        logError("DB", "Can't check drone id validity!");
        return id;
    }
    if (id <= 0) {
        loge("Can't obtain a valid id!");
        return id;
    }
    PostgreResult result = db->execute("SELECT id FROM drone WHERE id = " + std::to_string(id));
    if (result.error) {
        logError("DB", result.errorMessage);
        return id;
    }
    return result.result.empty()? id : checkDroneId(db, id + 1);
}

void Tower::handlePing(PingMessage *message) {
    long long id = message->getChannelId();
    // TODO: Update lastUpdate on DB;
    PingMessage *ping = new PingMessage(generateMessageId());
    this->channel->sendMessageTo(id, ping);
    delete ping;
}

void Tower::handleAssociation(AssociateMessage *message) {
    long long id = message->getDroneId();
    logi("Getting Valid ID");
    long long validId = checkDroneId(db, id);
    logi("Valid Drone Id found");
    // Get drones info
    PostgreResult result = this->db->execute("INSERT INTO drone (id, x, y, battery_autonomy, battery_life, dstate, last_update) VALUES (" + std::to_string(validId) + ", 0, 0, 0, 0, 'waiting', " + CURRENT_TIMESTAMP + ")");
    if (result.error) {
        logError("DB", result.errorMessage);
    }
    long long messageId = generateMessageId();
    AssociateMessage *m = new AssociateMessage(messageId, validId);
    this->channel->sendMessageTo(id, m);
    delete m;
}

void Tower::handleInfoMessage(DroneInfoMessage *message) {
    logi(message->parseMessage());
}

void Tower::handleLocationMessage(LocationMessage *message) {
    long long droneId = message->getChannelId();
    Drone drone = this->getDrone(droneId);
    this->area[drone.posX][drone.posY] = 0;
    drone.posX = message->getX();
    drone.posY = message->getY();
    logi("Drone " + std::to_string(droneId) + " arrived at: " + std::to_string(drone.posX) + "," + std::to_string(drone.posY));
    PostgreResult result = this->db->execute("UPDATE drone SET x = " + std::to_string(drone.posX) + ", y = " + std::to_string(drone.posY) + ", last_update = " + CURRENT_TIMESTAMP + " WHERE id = " + std::to_string(droneId));
    if (result.error) {
        logError("DB", result.errorMessage);
    }
    // Next Step
    calcolateDronePath(drone);
}

void Tower::handleMessage(Message* message) {
    if (message == nullptr) {
        return;
    }
    int type = message->getType();
    switch (type) {
        case 0: {
            this->handlePing(dynamic_cast<PingMessage*>(message));
            break;
        }
        case 1: {
            this->handleAssociation(dynamic_cast<AssociateMessage*>(message));
            break;
        }
        case 2: {
            this->handleInfoMessage(dynamic_cast<DroneInfoMessage*>(message));
            break;
        }
        case 4: {
            this->handleLocationMessage(dynamic_cast<LocationMessage*>(message));
            break;
        }
        case 3: // PathMessagge
        default: {
            loge("Invalid message type received: " + std::to_string(type));
            break;
        }
    }
    // Clear message on channel
    bool deleted = this->channel->removeMessage(message);
    if (deleted) {
        logi("Consumed message m:" + message->getFormattedId());
    } else {
        loge("Can't delete message m:" + message->getFormattedId() + " from redis!");
    }
    delete message;
}

void Tower::handleSignal(int signal) {
    switch (signal) {
        case SIGINT:
            logi("Received Interrupt Signal");
            Tower::running = false;
            break;
        case SIGTERM:
            logi("Received Termination Signal");
            Tower::running = false;
            break;
        default:
            logDebug("Tower", "Signal not handled: " + std::to_string(signal));
            break;
    }
}

bool Tower::isRunning() {
    return this->running;
}