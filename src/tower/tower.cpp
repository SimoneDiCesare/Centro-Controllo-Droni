#include "tower.hpp"
#include "channel.hpp"
#include "postgresql.hpp"
#include "log.hpp"
#include "time.hpp"
#include "globals.hpp"
#include "drawer.hpp"
#include <iostream>
#include <csignal>
#include <cstdlib>
#include <thread>
#include <vector>
#include <mutex>
#include <tuple>
#include <climits>

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

void logw(std::string message) {
    logWarning("Tower", message);
}

// Tower class definitions

bool Tower::running = false;

Tower::Tower(int droneCount, int areaWidth, int areaHeight, int cellTollerance) : messageCounterLock() {
    logi("Initializing Tower");
    this->channel = nullptr;
    this->db = nullptr;
    this->messageCounter = 0;
    // Initialize Area
    logi("Initializing Area");
    this->areaWidth = areaWidth;
    this->areaHeight = areaHeight;
    this->area = new Area(this->areaWidth, this->areaHeight);
    this->x = this->areaWidth / 2;
    this->y = this->areaHeight / 2;
    this->area->initArea(droneCount, this->x, this->y);
    logi("Tower Initialized");
}

Tower::~Tower() {
    running = false;
    if (this->channel != nullptr) {
        delete this->channel;
    }
    delete this->area;
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

bool Tower::createTrigger() {
    std::string dropTriggers = R"(
            DROP TRIGGER IF EXISTS drone_location_check ON drone;
            DROP TRIGGER IF EXISTS drone_battery_check ON drone;
        )";
    PostgreResult result = this->db->execute(dropTriggers);
    if (result.error) {
        logError("DB", "Dropping Triggers: " + result.errorMessage);
    } else {
        logInfo("DB", "Triggers cleared");
    }
    std::string locationTrigger = R"(
        CREATE OR REPLACE FUNCTION check_drone_area()
        RETURNS TRIGGER AS $$
        DECLARE
            min_x INT := 0;
            min_y INT := 0;
            max_x INT := )" + std::to_string(this->areaWidth) + R"(;
            max_y INT := )" + std::to_string(this->areaHeight) + R"(;
        BEGIN
            IF NEW.x < min_x OR NEW.x > max_x OR NEW.y < min_y OR NEW.y > max_y THEN
                RAISE EXCEPTION 'Drone % is out of surveillance area', NEW.id;
            END IF;
            RETURN NEW;
        END;
        $$ LANGUAGE plpgsql;
        CREATE TRIGGER drone_location_check
        BEFORE INSERT OR UPDATE ON drone
        FOR EACH ROW
        EXECUTE FUNCTION check_drone_area();
        
    )";
    std::string batteryTrigger = R"(
        CREATE OR REPLACE FUNCTION check_drone_battery()
        RETURNS TRIGGER AS $$
        BEGIN
            IF NEW.dstate != )" + std::to_string(CHARGING) + R"( AND NEW.dstate != )" + std::to_string(DEAD) + R"( AND NEW.battery_autonomy <= 0 THEN
                RAISE EXCEPTION 'Drone % has no battery left but is still flying', NEW.id;
            END IF;
            RETURN NEW;
        END;
        $$ LANGUAGE plpgsql;
        CREATE TRIGGER drone_battery_check
        AFTER UPDATE ON drone
        FOR EACH ROW
        EXECUTE FUNCTION check_drone_battery();
    )";
    result = this->db->execute(locationTrigger);
    bool ret = true;
    if (result.error) {
        logError("DB", "Location Trigger: " + result.errorMessage);
        ret = false;
    }
    result = this->db->execute(batteryTrigger);
    if (result.error) {
        logError("DB", "Battery Trigger: " + result.errorMessage);
        ret = false;
    }
    return ret;
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
        charge_time BIGINT,
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
        if (this->createTrigger()) {
            logi("Trigger Created");
        } else {
            loge("Error Creating Trigger");
        }
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
    PostgreResult result = this->db->execute("SELECT id, x, y, battery_autonomy, charge_time, dstate, last_update FROM drone");
    for (const auto& row : result.result) {
        Drone d;
        d.id = row[0].as<long long>();
        d.posX = row[1].as<int>();
        d.posY = row[2].as<int>();
        d.batteryAutonomy = row[3].as<long long>();
        d.chargeTime = row[4].as<long long>();
        d.droneState = static_cast<DroneState>(row[5].as<int>());
        d.lastUpdate = row[6].as<long long>();
        drones.push_back(d);
    }
    logi("Found: " + std::to_string(drones.size()) + " drones");
    return drones;
}

Drone Tower::getDrone(long long id) {
    Drone drone;
    PostgreResult result = this->db->execute("SELECT id, x, y, battery_autonomy, charge_time, dstate, last_update FROM drone WHERE id = " + std::to_string(id));
    for (const auto& row : result.result) {
        if (row[0].as<long long>() == id) {
            drone.id = row[0].as<long long>();
            drone.posX = row[1].as<int>();
            drone.posY = row[2].as<int>();
            drone.batteryAutonomy = row[3].as<long long>();
            drone.chargeTime = row[4].as<long long>();
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
        if (drone.droneState == DEAD) {
            continue; // Skip dead drones
        }
        timePassed = Time::seconds() - drone.lastUpdate;
        logi("Last Update for D" + std::to_string(drone.id) + ": " + std::to_string(timePassed));
        if (timePassed > 120) {
            if (timePassed > 300) { // Over 5' -> consider dead
                logw("Drone " + std::to_string(drone.id) + " not responding");
                PostgreResult result = this->db->execute("UPDATE drone SET dstate =" + std::to_string(DEAD) + " WHERE id = " + std::to_string(drone.id));
            } else {
                logi("Checking on Drone " + std::to_string(drone.id));
                PingMessage *ping = new PingMessage(generateMessageId());
                this->channel->sendMessageTo(drone.id, ping);
                delete ping;
            }
        } else if (drone.droneState == READY) {
            this->associateBlock(drone);
        }
    }
}

void Tower::droneCheckLoop() {
    while (this->running) {
        std::this_thread::sleep_for(std::chrono::seconds(60));
        if (!this->running) {
            break;
        }
        logi("Checking Drones");
        this->checkDrones();
    }
}

void Tower::drawGrid() {
    Drawer::init();
    while (this->running && !Drawer::shouldClose()) {
        // Update GUI
        Drawer::drawGrid(this->area->getMatrix(), this->areaWidth, this->areaHeight);
    }
    Drawer::close();
}

void Tower::areaUpdateLoop() {
    // Not Thread Safe -> does not create problems
    while (this->running) {
        // Arbitrary 60 seconds for showing stats
        std::this_thread::sleep_for(std::chrono::seconds(60));
        if (!this->running) {
            break;
        }
        this->calculateStatistics();
    }
    this->calculateStatistics();
    // TODO: Calculate True average.
}

void Tower::calculateStatistics() {
    unsigned long long media = 0;
    long long max = LLONG_MIN;
    long long min = LLONG_MAX;
    int tot = 0;
    int visited = 0;
    for(int i = 0; i < this->areaWidth; i++) {
        for(int j = 0; j < this->areaHeight; j++) {
            long long value = this->area->operator[](i)[j];
            if (value != 0){
                visited += 1;
            } else {
                float elapsed = (Time::nanos() - this->startTime) / 1e9;
                if (elapsed > this->cellTollerance) {
                    loge("Cell (" + std::to_string(i) + "," + std::to_string(i) + ") is not being visited!");
                }
                value = this->startTime;
            }  
            media += value;
            if (value > max) {
                max = value;
            }
            if (value < min) {
                min = value;
            } 
        }
    }
    float percentuale =  (static_cast<float>(visited) / tot) * 100;
    media = media / visited;
    this->avgs.emplace_back(media);
    logi("Stats: {visited:" + std::to_string(percentuale) + "%, max:" + std::to_string(max) + ", min:" + std::to_string(min) + ", avg:" + std::to_string(media) + "}");
}

void Tower::start() {
    long long start = Time::nanos();
    if (!this->channel->isUp()) {
        loge("Can't start tower without a connected channel!");
        return;
    }
    // Clean channel before running -> remove garbage from previous simulations
    this->channel->flush();
    // Register signals
    signal(SIGINT, Tower::handleSignal);
    signal(SIGTERM, Tower::handleSignal);
    this->running = true;
    std::vector<std::thread> threads;
    threads.emplace_back(&Tower::droneCheckLoop, this);
    threads.emplace_back(&Tower::areaUpdateLoop, this);
    threads.emplace_back(&Tower::drawGrid, this);
    logi("Tower online");
    this -> startTime = Time::nanos();
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
    // Disconnect Drones
    std::vector<Drone> drones = this->getDrones();
    for (Drone& drone : drones) {
        if (drone.droneState == DEAD) {
            continue;
        }
        DisconnectMessage *message = new DisconnectMessage(this->generateMessageId());
        this->channel->sendMessageTo(drone.id, message);
        delete message;
    }
    logi("Waiting threads to finish");
    // Await Spawned Threads
    for (auto& thread : threads) {
        thread.join();
    }
    // Flush Channel
    logi("Flushing Channel");
    bool channelFlushed = this->channel->flush();
    if (channelFlushed) {
        logi("Channel Flushed!");
    } else {
        logw("Can't flush redis channel");
    }
    long long end = Time::nanos();
    float sec = (end - start) / 1e9;
    logi("Duration of the Simulation: " + std::to_string(sec));
    unsigned long long total = 0;
    for (unsigned long long avg : this->avgs) {
        total += avg;
    }
    total = total / this->avgs.size();
    logi("System Total Average: " + std::to_string(total));
    /*logi("Printing Area");
    std::ofstream areaFile;
    areaFile.open("area.csv", std::ios_base::app);
    for(int i = 0; i < this->areaWidth; i++) {
        for(int j = 0; j < this->areaHeight; j++) {
            areaFile << this->area->operator[](i)[j] << ",";
        }
        areaFile << "\n";
    }
    areaFile.close();
    logi("Area Saved on area.scv");*/
}

long long checkDroneId(Postgre* db, long long id) {
    if (db == nullptr || !db->isConnected()) {
        logError("DB", "Can't check drone id validity!");
        return id;
    }
    if (id <= 0) {
        loge("Can't obtain a valid id!");
        return 1;
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
    PostgreResult result = this->db->execute("INSERT INTO drone (id, x, y, battery_autonomy, charge_time, dstate, last_update) VALUES (" + std::to_string(validId) + ", 0, 0, 0, 0, " + std::to_string(WAITING) + ", " + CURRENT_TIMESTAMP + ")");
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
    long long minValue = LLONG_MAX;
    int value = -1;
    Block *blockChosen = nullptr;
    for (Block& block : *blocks) {
        if (block.isAssigned()) {
            continue;
        }
        value = this->area->getMinIn(block);
        if (minValue > value) {
            minValue = value;
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
    drone.chargeTime = message->getChargeTime();
    drone.droneState = message->getState();
    std::string query = "UPDATE drone SET x=" + std::to_string(drone.posX)
        + ",y=" + std::to_string(drone.posY) + ",battery_autonomy=" + std::to_string(drone.batteryAutonomy)
        + ",charge_time=" + std::to_string(drone.chargeTime) + ",dstate=" + std::to_string(drone.droneState)
        + ",last_update=" + CURRENT_TIMESTAMP + " WHERE id=" + std::to_string(drone.id);
    PostgreResult result = this->db->execute(query);
    if (result.error) {
        logError("DB", result.errorMessage);
    }
    switch (drone.droneState) {
        case CHARGING:
            // Skip
            logi("Drone Charging");
            break;
        case READY: // Associate Block
            logi("Associate Block for " + std::to_string(drone.id));
            this->associateBlock(drone);
            break;
        case WAITING: // Next Move
            logi("Next step for " + std::to_string(drone.id));
            this->calcolateDronePath(drone);
            break;
        case MONITORING:
            // Skip
            logi("Drone Monitoring");
            break;
        case RETURNING:
            // Skip
            logi("Drone Returning");
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
            int x = block.getLastX() + block.getDirX();
            int y = block.getLastY();
            if (x == this->x && y == this->y) {
                logi("Skipping Tower Cell");
                // Safe because the tower is in the middle
                x += block.getDirX();
            }
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
    drone.posX = message->getX();
    drone.posY = message->getY();
    if (drone.posX < 0 || drone.posX > this->areaWidth || drone.posY < 0 || drone.posY > this->areaHeight) {
        loge("Drone " + std::to_string(droneId) + " out of bounds! (" + std::to_string(drone.posX) + "," + std::to_string(drone.posY) + ")");
    } else {
        this->area->operator[](drone.posX)[drone.posY] = Time::nanos();
    }
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
    this->area->operator[](drone.posX)[drone.posY] = Time::nanos();
    logi("Drone " + std::to_string(droneId) + " retiring");

    PostgreResult result = this->db->execute("UPDATE drone SET last_update = " + CURRENT_TIMESTAMP + " WHERE id = " + std::to_string(droneId));
    if (result.error) {
        logError("DB", result.errorMessage);
    }
    LocationMessage *loc = new LocationMessage(generateMessageId());
    loc->setLocation(this->x, this->y);
    loc->setMovementType(0);
    this->channel->sendMessageTo(drone.id, loc);
    delete loc;
    // Deassociate Block
    for (Block& block : *this->area->getBlocks()) {
        if (block.getAssignment() == droneId) {
            block.assignTo(-1);
            break;
        }
    }
}

void Tower::handleDisconnection(DisconnectMessage* message) {
    long long droneId = message->getChannelId();
    std::string droneIdText = std::to_string(droneId);
    logi("Drone " + droneIdText + " disconnected");
    PostgreResult result = this->db->execute("DELETE FROM drone WHERE id = " + droneIdText);
    if (result.error) {
        logError("DB", result.errorMessage);
        loge("Can't clear db entry for Drone " + droneIdText);
    } else {
        logi("Drone " + droneIdText + " entry cleared");
    }
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