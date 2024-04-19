#include "tower.hpp"
#include "channel.hpp"
#include <iostream>
#include <csignal>
#include <cstdlib>
#include <thread>
#include <vector>

bool Tower::running = false;

Tower::Tower() {
    this->channel = nullptr;
}

Tower::~Tower() {
    running = false;
    if (this->channel != nullptr) {
        delete this->channel;
    }
}

void Tower::connect(std::string ip, int port) {
    this->channel = new Channel(0);
    bool connected = this->channel->connect(ip, port);
    if (!connected) {
        std::cout << "Can't create channel for tower\n";
    } else {
        std::cout << "Tower connected to redis on channel: 0\n";
    }
}

void Tower::start() {
    if (!this->channel->isConnected()) {
        std::cout << "Can't start tower without a connected channel!\n";
        return;
    }
    // Register signals
    signal(SIGINT, Tower::handleSignal);
    signal(SIGTERM, Tower::handleSignal);
    this->running = true;
    std::vector<std::thread> threads;
    std::cout << "Starting\n";
    while (this->running) {
        Message *message = this->channel->awaitMessage();
        // TODO: - Implement Multithreading for requests
        //       - Implement Signals for Terminating Process
        //       - IMplement Response Logic
        if (message == NULL) {
            // std::cout << "No Message Recived\n";
        } else {
            std::cout << "Received Message: m:" << message->getChannelId() << ":" << message->getMessageId() << "\n";
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
    // Await Spawned Threads
    for (auto& thread : threads) {
        thread.join();
    }
    // TODO: - Disconnect and release drones
    //       - Exit with received signal 
}

void Tower::handleAssociation(AssociateMessage *message) {
    int id = message->getDroneId();
    // TODO: - Check validity inside db
    int newId = id;
    AssociateMessage *m = new AssociateMessage(this->messageCount, newId);
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
    std::cout << "Received Signal: "  << signal << "\n";
    Tower::running = false;
}