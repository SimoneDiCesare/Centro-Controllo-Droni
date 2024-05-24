#include "tower.hpp"
#include "channel.hpp"
#include "postgresql.hpp"
#include "log.hpp"
#include "time.hpp"
#include "globals.hpp"
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
    logi("Initializing Area");
    this->areaWidth = 10;
    this->areaHeight = 10;
    this->area = new Area(this->areaWidth, this->areaHeight);
    this->x = this->areaWidth / 2;
    this->y = this->areaHeight / 2;
    this->area->initArea(128, this->x, this->y);
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
        dstate INT,
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
        d.batteryAutonomy = row[3].as<long long>();
        d.batteryLife = row[4].as<long long>();
        d.droneState = static_cast<DroneState>(row[5].as<int>());
        d.lastUpdate = row[6].as<long long>();
        drones.push_back(d);
    }
    logi("Found: " + std::to_string(drones.size()) + " drones");
    return drones;
}

Drone Tower::getDrone(long long id) {
    Drone drone;
    PostgreResult result = this->db->execute("SELECT id, x, y, battery_autonomy, battery_life, dstate, last_update FROM drone WHERE id = " + std::to_string(id));
    for (const auto& row : result.result) {
        if (row[0].as<long long>() == id) {
            drone.id = row[0].as<long long>();
            drone.posX = row[1].as<int>();
            drone.posY = row[2].as<int>();
            drone.batteryAutonomy = row[3].as<long long>();
            drone.batteryLife = row[4].as<long long>();
            drone.droneState = static_cast<DroneState>(row[5].as<int>());
            drone.lastUpdate = row[6].as<long long>();
        }
    }
    return drone;
}

void Tower::checkDrones() {
    std::vector<Drone> drones = this->getDrones();
    long long currentTime = Time::seconds();
    long long timePassed = 0;
    for (const Drone& drone : drones) {
        timePassed = Time::seconds() - drone.lastUpdate;
        logi("Last Update for D" + std::to_string(drone.id) + ": " + std::to_string(timePassed));
        /*if (drone.droneState.compare("waiting") == 0) {
            logi("Sending drone to monitoring");
            this->calcolateDronePath(drone);
        } else if (drone.droneState.compare("monitoring") == 0) {
            if (timePassed > std::chrono::seconds(120)) {
                logi("Pinging drone");
                PingMessage *ping = new PingMessage(generateMessageId());
                this->channel->sendMessageTo(drone.id, ping);
                delete ping;
            }
        } else { // Charging Drone -> request for battery update
            logi("Checking drone with state: " + drone.droneState);
        }*/
    }
}

void Tower::droneCheckLoop() {
    /*while (this->running) {
        std::this_thread::sleep_for(std::chrono::seconds(60));
        logi("Checking Drones");
        this->checkDrones();
    }*/
}

void Tower::areaUpdateLoop() {
    long long start = Time::nanos();
    // Not Thread Safe -> does not create problems
    while (this->running) {
        // Arbitrary 10 seconds for growing up areas
        std::this_thread::sleep_for(std::chrono::seconds(10));
        for (int i = 0; i < this->areaWidth; i++) {
            for (int j = 0; j < this->areaHeight; j++) {
                this->area->operator[](i)[j] = this->area->operator[](i)[j] + 1;
            }
        }
    }
    long long end = Time::nanos();
    // Calculate Area Media Value
    float values = 0;
    float min = 500;
    float max = 0;
    for (int i = 0; i < this->areaWidth; i++) {
        for (int j = 0; j < this->areaHeight; j++) {
            int v = this->area->operator[](i)[j];
            values += v;
            if (v < min) {
                min = v;
            }
            if (v > max) {
                max = v;
            }
        }
    }
    float avg = values / (float)(this->areaWidth * this->areaHeight);
    float sec = (end - start) / 1e9;
    logi("Average values: " + std::to_string(avg) + " in " + std::to_string(sec) + "s");
    logi("Max: " + std::to_string(max) + ", Min" + std::to_string(min));
}

void Tower::start() {
    if (!this->channel->isUp()) {
        loge("Can't start tower without a connected channel!");
        return;
    }
    // TODO: Create Thread for updating are values
    // Register signals
    signal(SIGINT, Tower::handleSignal);
    signal(SIGTERM, Tower::handleSignal);
    this->running = true;
    std::vector<std::thread> threads;
    threads.emplace_back(&Tower::droneCheckLoop, this);
    threads.emplace_back(&Tower::areaUpdateLoop, this);
    logi("Tower online");
    while (this->running) {
        // 1' of waiting before restarting the cycle
        Message *message = this->channel->awaitMessage(10);
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
    PostgreResult result = this->db->execute("UPDATE drone SET last_update = " + CURRENT_TIMESTAMP + " WHERE id = " + std::to_string(id));
    if (result.error) {
        logError("DB", result.errorMessage);
    }
}

void Tower::handleAssociation(AssociateMessage *message) {
    long long id = message->getDroneId();
    logi("Getting Valid ID");
    long long validId = checkDroneId(db, id);
    logi("Valid Drone Id found");
    // Get drones info
    PostgreResult result = this->db->execute("INSERT INTO drone (id, x, y, battery_autonomy, battery_life, dstate, last_update) VALUES (" + std::to_string(validId) + ", 0, 0, 0, 0, " + std::to_string(WAITING) + ", " + CURRENT_TIMESTAMP + ")");
    if (result.error) {
        logError("DB", result.errorMessage);
    }
    // Send back Association
    AssociateMessage *associateMessage = new AssociateMessage(generateMessageId(), validId);
    associateMessage->setTowerX(this->x);
    associateMessage->setTowerY(this->y);
    this->channel->sendMessageTo(id, associateMessage);
    // Require Infos
    DroneInfoMessage *infoMessage = new DroneInfoMessage(generateMessageId());
    this->channel->sendMessageTo(validId, infoMessage);
    delete associateMessage;
    delete infoMessage;
}

void Tower::associateBlock(Drone drone) {
    std::vector<Block> *blocks = this->area->getBlocks();
    int maxValue = -1;
    int value = -1;
    Block *blockChosen = nullptr;
    for (Block& block : *blocks) {
        if (block.isAssigned()) {
            continue;
        }
        value = this->area->getMaxIn(block);
        if (maxValue < value) {
            maxValue = value;
            blockChosen = &block;
        }
    }
    if (blockChosen != nullptr) {
        logi("Associating Block " + blockChosen->toString());
        blockChosen->assignTo(drone.id);
        LocationMessage *location = new LocationMessage(generateMessageId());
        location->setLocation(blockChosen->getStartX(), blockChosen->getStartY());
        this->channel->sendMessageTo(drone.id, location);
        delete location;
    } else {
        logi("No block found for D" + std::to_string(drone.id));
        if (drone.posX != this->x || drone.posY != this->y) {
            // Return to Tower
            logi("Retiring D" + std::to_string(drone.id));
            LocationMessage *location = new LocationMessage(generateMessageId());
            location->setLocation(this->x, this->y);
            this->channel->sendMessageTo(drone.id, location);
            delete location;
        }
    }
}

void Tower::handleInfoMessage(DroneInfoMessage *message) {
    logi(message->parseMessage());
    Drone drone;
    drone.id = message->getDroneId();
    drone.posX = message->getPosX();
    drone.posY = message->getPosY();
    drone.batteryAutonomy = message->getBatteryAutonomy();
    drone.batteryLife = message->getBatteryLife();
    drone.droneState = static_cast<DroneState>(message->getState());
    std::string query = "UPDATE drone SET x=" + std::to_string(drone.posX)
        + ",y=" + std::to_string(drone.posY) + ",battery_autonomy=" + std::to_string(drone.batteryAutonomy)
        + ",battery_life=" + std::to_string(drone.batteryLife) + ",dstate=" + std::to_string(drone.droneState)
        + ",last_update=" + CURRENT_TIMESTAMP + " WHERE id=" + std::to_string(drone.id);
    PostgreResult result = this->db->execute(query);
    if (result.error) {
        logError("DB", result.errorMessage);
    }
    switch (drone.droneState) {
        case WAITING: // Associate Block
            logi("Associate Block for " + std::to_string(drone.id));
            this->associateBlock(drone);
            break;
        case CHARGING:
            logi(std::to_string(drone.id) + " Charging");
            break;
        case MONITORING:
            logi(std::to_string(drone.id) + " Monitoring");
            break;
        default:
            loge("Invalid state " + std::to_string(drone.droneState) + " for drone " + std::to_string(drone.id));
            break;
    }
}

void Tower::calcolateDronePath(Drone drone) {
    // Check Active Block
    std::vector<Block> *blocks = this->area->getBlocks();
    for (Block& block : *blocks) {
        if (block.getAssignment() == drone.id) {
            // Check block cells
            if (block.getLastX() != drone.posX || block.getLastY() != drone.posY) {
                block.setLastX(drone.posX);
                block.setLastY(drone.posY);
            }
            x = block.getLastX() + block.getDirX();
            y = block.getLastY();
            if (x >= block.getX() + block.getWidth() || x < block.getX()) {
                y += block.getDirY();
                x -= block.getDirX();
                if (y > block.getY() + block.getHeight() || y < block.getY()) {
                    // Reset Block and Associate a new One
                    this->associateBlock(drone);
                    block.reset(this->x, this->y);
                    return;
                }
                block.setDirX(-block.getDirX());
            }
            LocationMessage *location = new LocationMessage(generateMessageId());
            location->setLocation(x, y);
            this->channel->sendMessageTo(drone.id, location);
            delete location; 
            return;
        }
    }
    // Not Returned before -> no Block associated
    this->associateBlock(drone);
}

void Tower::handleLocationMessage(LocationMessage *message) {
    long long droneId = message->getChannelId();
    Drone drone = this->getDrone(droneId);
    this->area->operator[](drone.posX)[drone.posY] = 0;
    drone.posX = message->getX();
    drone.posY = message->getY();
    logi("Drone " + std::to_string(droneId) + " arrived at: " + std::to_string(drone.posX) + "," + std::to_string(drone.posY));
    PostgreResult result = this->db->execute("UPDATE drone SET x = " + std::to_string(drone.posX) + ", y = " + std::to_string(drone.posY) + ", last_update = " + CURRENT_TIMESTAMP + " WHERE id = " + std::to_string(droneId));
    if (result.error) {
        logError("DB", result.errorMessage);
    }
    // Pick next step
    this->calcolateDronePath(drone);
}

void Tower::handleRetireMessage(RetireMessage* message) {
    long long droneId = message->getChannelId();
    Drone drone = this->getDrone(droneId);
    this->area[drone.posX];
    this->area->operator[](drone.posX)[drone.posY] = 0;
    logi("Drone " + std::to_string(droneId) + " retiring");
    // TODO: Update to real drone position
    PostgreResult result = this->db->execute("UPDATE drone SET x = 0 ,y = 0, last_update = " + CURRENT_TIMESTAMP + " WHERE id = " + std::to_string(droneId));
    if (result.error) {
        logError("DB", result.errorMessage);
    }
    LocationMessage *loc = new LocationMessage(generateMessageId());
    loc->setLocation(this->x, this->y);
    loc->setMovementType(0);
    this->channel->sendMessageTo(drone.id, loc);
    delete loc;
}

void Tower::handleDisconnection(DisconnectMessage* message) {
    logi("Drone " + std::to_string(message->getChannelId()) + " disconnecting");
}

void Tower::handleMessage(Message* message) {
    if (message == nullptr) {
        return;
    }
    int type = message->getType();
    switch (type) {
        case 0: {
            this->handleAssociation(dynamic_cast<AssociateMessage*>(message));
            break;
        }
        case 1: {
            this->handlePing(dynamic_cast<PingMessage*>(message));
            break;
        }
        case 2: {
            this->handleInfoMessage(dynamic_cast<DroneInfoMessage*>(message));
            break;
        }
        case 3: {
            this->handleLocationMessage(dynamic_cast<LocationMessage*>(message));
            break;
        }
        case 4: {
            this->handleRetireMessage(dynamic_cast<RetireMessage*>(message));
            break;
        }
        case 5: {
            this->handleDisconnection(dynamic_cast<DisconnectMessage*>(message));
            break;
        }
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