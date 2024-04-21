#include "tower.hpp"
#include "channel.hpp"
#include "postgresql.hpp"
#include <iostream>
#include <csignal>
#include <cstdlib>
#include <thread>
#include <vector>
#include "log.hpp"

long long nanos() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    // Convert the time point to nanoseconds since epoch
    auto nanoseconds_since_epoch = std::chrono::time_point_cast<std::chrono::nanoseconds>(currentTime).time_since_epoch().count();
    // Store the nanoseconds in a long long
    long long currentNanoseconds = nanoseconds_since_epoch;
    return currentNanoseconds;
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

Tower::Tower() {
    this->channel = nullptr;
    this->db = nullptr;
}

Tower::~Tower() {
    running = false;
    if (this->channel != nullptr) {
        delete this->channel;
    }
}

bool Tower::connect(std::string ip, int port) {
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
        battery_autonomy INTERVAL,
        battery_life INTERVAL,
        dstate DSTATE,
        last_update TIMESTAMP,
        CHECK(id > 0)
        ))");
        if (result.error) {
            loge(result.errorMessage);
            return false;
        }
        logi("Table drone Created");
        result = this->db->execute("TRUNCATE TABLE drone");
        if (result.error) {
            loge(result.errorMessage);
            return false;
        }
        logi("Table drone Cleared");
        return true;
    } else {
        loge("Can't connected to db!");
        return false;
    }
}

void Tower::start() {
    if (!this->channel->isConnected()) {
        loge("Can't start tower without a connected channel!");
        return;
    }
    // Register signals
    signal(SIGINT, Tower::handleSignal);
    signal(SIGTERM, Tower::handleSignal);
    this->running = true;
    std::vector<std::thread> threads;
    logi("Tower online");
    // Timeout on awaitMessage = 5sec
    this->channel->setTimeout(5);
    while (this->running) {
        Message *message = this->channel->awaitMessage();
        if (message == nullptr) {
            // If we have no message to handle, we check last updates from drones
            // If a last update is > x second (to decide, maybe 1-5' => 60-300'')
        } else {
            // Handle message received on another thread, and return to listen
            logi("Received message from Drone:" + std::to_string(message->getChannelId()));
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
        loge("Can't check drone id validity!");
        return id;
    }
    PostgreResult result = db->execute("SELECT id FROM drone WHERE id = " + std::to_string(id));
    if (result.error) {
        loge(result.errorMessage);
        return id;
    }
    return result.result.empty()? id : checkDroneId(db, id + 1);
}

void Tower::handleAssociation(AssociateMessage *message) {
    long long id = message->getDroneId();
    long long validId = checkDroneId(db, id);
    // Get drones info
    PostgreResult result = this->db->execute("INSERT INTO drone (id, x, y, battery_autonomy, battery_life, dstate, last_update) VALUES (" + std::to_string(validId) + ", 0, 0, '00:00:00', '00:00:00', 'waiting', CURRENT_TIMESTAMP)");
    if (result.error) {
        loge(result.errorMessage);
    }
    long long messageId = nanos();
    while (true) {
        if (this->channel->hasMessage(messageId)) {
            messageId += 1;
        } else {
            break;
        }
    }
    AssociateMessage *m = new AssociateMessage(messageId, validId);
    this->channel->sendMessageTo(id, *m);
}

void Tower::handleMessage(Message* message) {
    if (message == nullptr) {
        return;
    }
    int type = message->getType();
    switch (type) {
        case 0: {
            std::cout << "Ping Message\n";
            break;
        }
        case 1: {
            this->handleAssociation(dynamic_cast<AssociateMessage*>(message));
            break;
        }
        case 2: {
            std::cout << "Drone Info Message\n";
            break;
        }
        case 3: {
            std::cout << "Location Message\n";
            break;
        }
        default: {
            std::cout << "FUCK: " << type << "\n";
            break;
        }
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