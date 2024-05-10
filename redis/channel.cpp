#include "channel.hpp"
#include "redis.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include <chrono>

// utility local functions

bool connectClient(Redis *redis, std::string ip, int port) {
    if (redis == nullptr) {
        return false;
    }
    if (redis->isConnected()) {
        return true;
    }
    return redis->connect(ip, port);
}

bool isValidNumber(const std::string& str) {
    // Skip leading whitespace
    const char* ptr = str.c_str();
    while (std::isspace(*ptr)) {
        ptr++;
    }
    // Attempt to convert the remaining string to a long integer
    char* endptr;
    std::strtoll(ptr, &endptr, 10);
    // If endptr is equal to ptr, no valid digits were found
    // If endptr is not equal to '\0', not all characters were consumed
    return *ptr != '\0' && *endptr == '\0';
}

bool isValidMessageId(std::string id) {
    if (id.at(0) != 'm' || id.at(1) != ':') {
        return false;
    }
    std::size_t delimiter = id.find_last_of(":");
    bool f1 = isValidNumber(id.substr(1, delimiter));
    bool f2 = isValidNumber(id.substr(delimiter + 1, id.length()));
    return f1 & f2;
}


// Abstract Class Message 

Message::Message(std::string id) {
    std::size_t delimiter = id.find(":");
    if (delimiter == std::string::npos) {
        std::cout << "Invalid ID: " << id << "\n";
        return;
    } else {
        this->channelId = std::stoll(id.substr(0, delimiter));
        this->messageId = std::stoll(id.substr(delimiter + 1, id.length()));
    }
}

Message::Message(long long messageId) {
    this->messageId = messageId;
    this->channelId = -1;
}

Message::~Message() {

}

long long Message::getMessageId() {
    return this->messageId;
}

long long Message::getChannelId() {
    return this->channelId;
}

std::string Message::getFormattedId() {
    return std::to_string(this->channelId) + ":" + std::to_string(this->messageId);
}

// Ping Message

PingMessage::PingMessage(std::string id) : Message(id) {
    
}

PingMessage::PingMessage(long long messageId) : Message(messageId) {
    
}

void PingMessage::parseResponse(RedisResponse *response) {
    if (response->hasError()) {
        std::cout << "Can't PING:\n" << response->getError() << "\n";
        delete response;
        return;
    }
    if (response->getType() == VECTOR) {
        std::vector<std::string> vector = response->getVectorContent();
        std::cout << vector.size() << "\n";
        for (int i = 0; i < vector.size(); i++) {
            std::cout << vector[i] << "\n";
        }
    } else {
        std::cout << response->getContent() << "\n";
    }
}

std::string PingMessage::parseMessage() {
    return "type " + std::to_string(this->getType());
}

// AssociateMessage

AssociateMessage::AssociateMessage(std::string id, long long droneId) : Message(id) {
    this->droneId = droneId;
}

AssociateMessage::AssociateMessage(long long messageId, long long droneId) : Message(messageId) {
    this->droneId = droneId;
} 

void AssociateMessage::parseResponse(RedisResponse* response) {
    if (response->getType() == VECTOR) {
        std::vector<std::string> data = response->getVectorContent();
        for (int i = 0; i < data.size(); i++) {
            std::string value = data[i];
            if (value.compare("droneId") == 0) {
                this->droneId = std::stoll(data[i + 1]);
            }
        }
    }
}

std::string AssociateMessage::parseMessage() {
    return "type " + std::to_string(this->getType()) + " droneId " + std::to_string(this->droneId);
}

long long AssociateMessage::getDroneId() {
    return this->droneId;
}

// Drone Info Message Class
// TODO: Add all drone infos

DroneInfoMessage::DroneInfoMessage(std::string id, long long droneId) : Message(id) {
    this->droneId = droneId;
}

DroneInfoMessage::DroneInfoMessage(long long messageId, long long droneId) : Message(messageId) {
    this->droneId = droneId;
} 

void DroneInfoMessage::parseResponse(RedisResponse* response) {
    if (response->getType() == VECTOR) {
        std::vector<std::string> data = response->getVectorContent();
        for (int i = 0; i < data.size(); i+=2) {
            std::string key = data[i];
            if (key.compare("type") == 0) {
                continue;
            }
            std::string value = data[i + 1];
            int intValue = std::stoi(value);
            long long longValue = std::stoll(value);
            if (key.compare("droneId") == 0) {
                this->droneId = longValue;
            } else if (key.compare("x") == 0) {
                this->posX = intValue;
            } else if (key.compare("y") == 0) {
                this->posY = intValue;
            } else if (key.compare("batteryAutonomy") == 0) {
                this->batteryAutonomy = longValue;
            } else if (key.compare("batteryLife") == 0) {
                this->batteryLife = longValue;
            } else if (key.compare("state") == 0) {
                this->state = intValue;
            }
        }
    }
}

std::string DroneInfoMessage::parseMessage() {
    std::string message = "type " + std::to_string(this->getType());
    message += " droneId " + std::to_string(this->droneId);
    message += " x " + std::to_string(this->posX);
    message += " y " + std::to_string(this->posY);
    message += " battery_autonomy " + std::to_string(this->batteryAutonomy);
    message += " battery_life " + std::to_string(this->batteryLife);
    message += " state " + std::to_string(this->state);
    return message;
}

// Getter

long long DroneInfoMessage::getDroneId() {
    return this->droneId;
}

int DroneInfoMessage::getPosX() {
    return this->posX;
}

int DroneInfoMessage::getPosY() {
    return this->posY;
}

long long DroneInfoMessage::getBatteryAutonomy() {
    return this->batteryAutonomy;
}

long long DroneInfoMessage::getBatteryLife() {
    return this->batteryLife;
}

int DroneInfoMessage::getState() {
    return this->state;
} 

// Setter

void DroneInfoMessage::setDroneId(long long id) {
    this->droneId = id;
}

void DroneInfoMessage::setPosX(int x) {
    this->posX = x;
}

void DroneInfoMessage::setPosY(int y) {
    this->posY = y;
}

void DroneInfoMessage::setBatteryAutonomy(long long batteryAutonomy) {
    this->batteryAutonomy = batteryAutonomy;
}

void DroneInfoMessage::setBatteryLife(long long batteryLife) {
    this->batteryLife = batteryLife;
}

void DroneInfoMessage::setState(int state) {
    this->state = state;
}

// Location Message Class

PathMessage::PathMessage(std::string id) : Message(id) {
    
}

PathMessage::PathMessage(long long messageId) : Message(messageId) {
    
}

void PathMessage::parseResponse(RedisResponse* response) {
    if (response->getType() == VECTOR) {
        std::vector<std::string> data = response->getVectorContent();
        for (int i = 0; i < data.size(); i+=2) {
            std::string key = data[i];
            if (key.compare("type") == 0) {
                continue;
            }
            std::string value = data[i + 1];
            this->locations.push_back(std::tuple<char, int>(key.at(0), std::stoi(value)));
        }
    }
}

std::string PathMessage::parseMessage() {
    std::string data = "type " + std::to_string(this->getType());
    int xCount = 0;
    int yCount = 0;
    for (int i = 0; i < this->locations.size(); i++) {
        char axis = std::get<0>(this->locations[i]);
        std::string axisVar(1, axis);
        if (axis == 'x') {
            axisVar += std::to_string(xCount);
            xCount++;
        } else if (axis == 'y') {
            yCount++;
            axisVar += std::to_string(yCount);
        }
        std::string value = std::to_string(std::get<1>(this->locations[i]));
        data = data + " " + axisVar + " " + value;
    }
    return data;
}

std::tuple<char, int> PathMessage::getLocation(int i) {
    return this->locations[i];
}

int PathMessage::getStepCount() {
    return this->locations.size();
}

void PathMessage::setLocations(std::vector<std::tuple<char, int>> locations) {
    this->locations = locations;
}

// Location Message

// Location Message Class

LocationMessage::LocationMessage(std::string id) : Message(id) {
    
}

LocationMessage::LocationMessage(long long messageId) : Message(messageId) {
    
}

void LocationMessage::parseResponse(RedisResponse* response) {
    if (response->getType() == VECTOR) {
        std::vector<std::string> data = response->getVectorContent();
        for (int i = 0; i < data.size(); i+=2) {
            std::string key = data[i];
            std::string value = data[i + 1];
            if (key.compare("type") == 0) {
                continue;
            }
            if (key.compare("x") == 0) {
                this->x = std::stoi(value);
            } else if (key.compare("y") == 0) {
                this->y = std::stoi(value);
            }
        }
    }
}

std::string LocationMessage::parseMessage() {
    std::string data = "type " + std::to_string(this->getType());
    data = data + " x " + std::to_string(this->x);
    data = data + " y " + std::to_string(this->y);
    return data;
}

void LocationMessage::setLocation(int x, int y) {
    this->x = x;
    this->y = y;
}

int LocationMessage::getX() {
    return this->x;
}

int LocationMessage::getY() {
    return this->y;
}

// Channel Class

Channel::Channel(long long id) : readingLock(), writingLock() {
    this->id = id;
    this->readingClient = nullptr;
    this->writingClient = nullptr;
}

Channel::~Channel() {
    delete this->readingClient;
    delete this->writingClient;
}

// Utility Functions
RedisResponse* Channel::sendWriteCommand(std::string command) {
    this->writingLock.lock();
    RedisResponse *response = this->writingClient->sendCommand(command);
    this->writingLock.unlock();
    return response;
}

RedisResponse* Channel::sendReadCommand(std::string command) {
    this->readingLock.lock();
    RedisResponse *response = this->readingClient->sendCommand(command);
    this->readingLock.unlock();
    return response;
}

// Connection Functions

bool Channel::connect(std::string ip /*= "127.0.0.1"*/, int port /*= 6379*/) {
    this->readingLock.lock();
    if (this->readingClient == nullptr) {
        this->readingClient = new Redis();
    }
    if (!connectClient(this->readingClient, ip, port)) {
        std::cout << "Can't connect Reading Client!";
    }
    this->readingLock.unlock();
    this->writingLock.lock();
    if (this->writingClient == nullptr) {
        this->writingClient = new Redis();
    }
    if (!connectClient(this->writingClient, ip, port)) {
        std::cout << "Can't connect Reading Client!";
    }
    this->writingLock.unlock();
    return this->isUp();
}

bool Channel::disconnect() {
    // TODO: Implement safe disconnection
    return true;
}

// Writing Operations

bool Channel::sendMessageTo(long long channelId, Message *message) {
    if (message == nullptr || !this->canWrite()) {
        return false;
    }
    std::string messageId = "m:" + std::to_string(this->id) + ":" + std::to_string(message->getMessageId());
    // Create message on Redis
    std::string command = "hset " + messageId + " " + message->parseMessage();
    std::cout << "Sending command: [" << command << "]\n";
    // Only one thread can write at a time
    RedisResponse *response = this->sendWriteCommand(command);
    if (response->hasError()) {
        if (response->getType() == NONE) {
            std::cout << "Timeout writing message!\n";
        } else {
            std::cout << "Error: " << response->getError() << "\n";
        }
        delete response;
        return false;
    }
    delete response;
    // Insert message on receiver channel 
    command = "lpush c:" + std::to_string(channelId) + " " + messageId;
    std::cout << "Sending command: [" << command << "]\n";
    response = this->sendWriteCommand(command);
    if (response->hasError()) {
        if (response->getType() == NONE) {
            std::cout << "Timeout sending message!\n";
        } else {
            std::cout << "Error: " << response->getError() << "\n";
        }
        delete response;
        return false;
    }
    delete response;
    return true;
}

bool Channel::removeMessage(Message *message) {
    if (message == nullptr || !this->canWrite()) {
        return false;
    }
    // Only one thread can write at a time
    std::string key = "m:" + message->getFormattedId();
    RedisResponse *response = this->sendWriteCommand("DEL " + key);
    if (response->hasError()) {
        if (response->getType() == NONE) {
            std::cout << "Timeout deleting message!";
        } else {
            std::cout << "Error: " << response->getError() << "\n";
        }
        delete response;
        return false;
    }
    delete response;
    return true;
}

bool Channel::flush() {
    if (!this->canWrite()) {
        return false;
    }
    RedisResponse *response = this->sendWriteCommand("DEL c:" + std::to_string(this->id));
    if (response->hasError()) {
        if (response->getType() == NONE) {
            std::cout << "Timeout flushin channel!";
        } else {
            std::cout << "Error: " << response->getError() << "\n";
        }
        delete response;
        return false;
    }
    delete response;
    return true;
}

// Reading Operations

Message* Channel::readMessageWithId(std::string messageId) {
    RedisResponse *response = this->sendReadCommand("HGETALL " + messageId);
    if (response->hasError()) {
        if (response->getType() == NONE) {
            std::cout << "Timeout on HGETALL\n";
        } else {
            std::cout << "Error: " << response->getError() << "\n";
        }
        delete response;
        return nullptr;
    }
    // Convert messageId as cId:mId
    std::vector<std::string> params = response->getVectorContent();
    messageId = messageId.substr(2, messageId.length());
    int messageType = -1;
    for (int i = 0; i < params.size(); i++) {
        if (params[i].compare("type") == 0 && i < params.size() - 1) {
            messageType = std::stoi(params[i + 1]);
            break;
        }
    }
    Message *m = nullptr;
    switch(messageType) {
        case 0:
            m = new PingMessage(messageId);
            break;
        case 1:
            m = new AssociateMessage(messageId, -1);
            break;
        case 2:
            m = new DroneInfoMessage(messageId, -1);
            break;
        case 3:
            m = new PathMessage(messageId);
            break;
        case 4:
            m = new LocationMessage(messageId);
            // m = new RetireMessage(messageId, -1);
            break;
        case 5:
            // m = new DisconnectMessage(messageId, -1);
            break;
        default:
            std::cout << "Unhandled Type: " << messageType << "\n";
    }
    if (m != nullptr) {
        m->parseResponse(response);
    }
    delete response;
    return m;
}

Message* Channel::readMessage() {
    if (!this->canRead()) {
        return nullptr;
    }
    std::string channelId = "c:" + std::to_string(this->id);
    RedisResponse *response = this->sendReadCommand("RPOP " + channelId);
    if (response->hasError()) {
        if (response->getType() == NONE) {
            std::cout << "Timeout on RPOP\n";
        } else {
            std::cout << "Error: " << response->getError() << "\n"; 
        }
        delete response;
        return nullptr;
    }
    // Empty queue?
    if (response->getType() == NLL) {
        delete response;
        return nullptr;
    }
    std::string messageId = response->getContent();
    if (!isValidMessageId(messageId)) {
        std::cout << "Invalid message id in queue: " << messageId << "\n";
        delete response;
        return nullptr;        
    }
    delete response;
    return this->readMessageWithId(messageId);
}

Message* Channel::awaitMessage(long timeout /* = 0*/) {
    if (!this->canRead()) {
        return nullptr;
    }
    std::string channelId = "c:" + std::to_string(this->id);
    RedisResponse *response = this->sendReadCommand("BRPOP " + channelId + " " + std::to_string(timeout));
    if (response->hasError()) {
        if (response->getType() == NONE) {
            std::cout << "Timeout on BRPOP\n";
        } else {
            std::cout << "Error: " << response->getError() << "\n";
        }
        delete response;
        return nullptr;
    }
    if (response->getType() != VECTOR) {
        std::cout << "Unexpected behaviour from redis! Await Type: " << response->getType() << "\n";
        delete response;
        return nullptr;
    }
    // Check if response is empty
    std::vector<std::string> params = response->getVectorContent();
    delete response;
    if (params.size() == 0) {
        return nullptr;
    }
    std::string messageId = params[1];
    return this->readMessageWithId(messageId);
}

// Getter

long long Channel::getId() {
    return this->id;
}

bool Channel::canRead() {
    return this->readingClient->isConnected();
}

bool Channel::canWrite() {
    return this->writingClient->isConnected();
}

bool Channel::isUp() {
    return this->canRead() && this->canWrite();
}

// Setter

void Channel::setId(long long id) {
    this->id = id;
}