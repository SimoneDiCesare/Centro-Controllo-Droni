#include "tower.hpp"
#include "channel.hpp"
#include "postgresql.hpp"
#include <iostream>
#include <csignal>
#include <cstdlib>
#include <thread>
#include <vector>
#include "log.hpp"

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
    this->messageCount = 0;
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
        auto result = this->db->execute(R"(CREATE TABLE IF NOT EXISTS drone ("
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
    this->channel->setTimeout(5);
    while (this->running) {
        Message *message = this->channel->awaitMessage();
        // TODO: - Implement Multithreading for requests
        //       - Implement Signals for Terminating Process
        //       - IMplement Response Logic
        if (message == nullptr) {
            // std::cout << "No Message Recived\n";
        } else {
            logi("Received message from Drone:" + std::to_string(message->getChannelId()));
            threads.emplace_back(&Tower::handleMessage, this, message);
        }
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

long long generateUniqueId(PostgreResult result, long long id) {
    if (id == -1) {
        loge("Too many id where assigned!");
        return 1;
    }
    for (auto const &row: result.result) {
        for (auto const &field: row) {
            // Safe type check
            if (field.as<long long>() == id) {
                return generateUniqueId(result, id + 1);
            }
        }
    }
    return id;
}

long long checkDroneId(Postgre* db, long long id) {
    if (db == nullptr || !db->isConnected()) {
        loge("Can't check drone id validity!");
        return id;
    }
    PostgreResult result = db->execute("SELECT id FROM drone");
    if (result.error) {
        loge(result.errorMessage);
        return id;
    }
    return generateUniqueId(result, id);
}

void Tower::handleAssociation(AssociateMessage *message) {
    long long id = message->getDroneId();
    long long validId = checkDroneId(db, id);
    AssociateMessage *m = new AssociateMessage(this->messageCount, validId);
    this->messageCount++;
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