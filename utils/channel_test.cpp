#include <iostream>
#include <string>
#include "redis.hpp"
#include "tower.hpp"
#include "channel.hpp"

int main() {
    Channel *channel = new Channel(0);
    channel->connect();
    Message *message = new PingMessage(0);
    bool sended = channel->sendMessageTo(0, *message);
    std::cout << "Sended: " << sended << "\n";
    delete message;
    message = channel->awaitMessage(5);
    delete message;
    delete channel;
    // Tower tower;
    // tower.start();
    return 0;
}