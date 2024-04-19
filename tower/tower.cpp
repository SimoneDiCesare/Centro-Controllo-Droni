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

void Tower::handleMessage(Message* message) {
    if (message == NULL) {
        return;
    }
    int type = message->getType();
    switch (type) {
        case 0:
            std::cout << "Ping Message\n";
            break;
        case 1:
            std::cout << "Associate Message\n";
            break;
        case 2:
            std::cout << "Drone Info Message\n";
            break;
        case 3:
            std::cout << "Location Message\n";
            break;
        default:
            std::cout << "Type not handled: " << type << "\n";
            break;
    }
    if (AssociateMessage *m = dynamic_cast<AssociateMessage*>(message)) {
        std::cout << m->getDroneId() << "\n";
    } else if (DroneInfoMessage *m = dynamic_cast<DroneInfoMessage*>(message)) {
        std::cout << m->getX() << "\n";
    }
    // TODO: - Process message information
    delete message;
}

void Tower::handleSignal(int signal) {
    std::cout << "Received Signal: "  << signal << "\n";
    Tower::running = false;
}