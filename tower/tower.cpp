#include "tower.hpp"
#include "channel.hpp"
#include <iostream>

Tower::Tower() {
    this->channel = new Channel(0);
    bool connected = this->channel->connect("127.0.0.1", 6379);
    if (!connected) {
        std::cout << "Can't create channel for tower\n";
    }
}

Tower::~Tower() {
    delete this->channel;
}

void Tower::start() {
    if (!this->channel->isConnected()) {
        std::cout << "Can't start tower without a connected channel!\n";
        return;
    }
    while (true) {
        Message *message = this->channel->awaitMessage();
        // TODO: - Implement Multithreading for requests
        //       - Implement Signals for Terminating Process
        //       - IMplement Response Logic
        if (message == NULL) {
            std::cout << "No Message Recived\n";
        } else {
            std::cout << "Received Message: m:" << message->getChannelId() << ":" << message->getMessageId() << "\n";
        }
        break;
    }
}