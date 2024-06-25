#include "channel.hpp"
#include "redis.hpp"
#include "log.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include <chrono>

// utility local functions

void logChannelError(std::string message) {
    logError("Channel", message);
}

void logChannelDebug(std::string message) {
    logDebug("Channel", message);
}

void logChannelWarning(std::string message) {
    logWarning("Channel", message);
}

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
        logChannelError("Invalid Message ID: " + id);
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
        logChannelError("Can't ping: " + response->getError());
        delete response;
        return;
    }
}

std::string PingMessage::parseMessage() {
    return "type " + std::to_string(this->getType());
}

// AssociateMessage

AssociateMessage::AssociateMessage(std::string id, long long droneId) : Message(id) {
    this->droneId = droneId;
    this->towerX = 0;
    this->towerY = 0;
}

AssociateMessage::AssociateMessage(long long messageId, long long droneId) : Message(messageId) {
    this->droneId = droneId;
    this->towerX = 0;
    this->towerY = 0;
} 

void AssociateMessage::parseResponse(RedisResponse* response) {
    if (response->getType() == VECTOR) {
        std::vector<std::string> data = response->getVectorContent();
        for (int i = 0; i < data.size(); i++) {
            std::string value = data[i];
            if (value.compare("droneId") == 0) {
                this->droneId = std::stoll(data[i + 1]);
            } else if (value.compare("x") == 0) {
                this->towerX = std::stoi(data[i + 1]);
            } else if (value.compare("y") == 0) {
                this->towerY = std::stoi(data[i + 1]);
            }
        }
    }
}

std::string AssociateMessage::parseMessage() {
    return "type " + std::to_string(this->getType()) + " droneId " + std::to_string(this->droneId) + " x " + std::to_string(this->towerX) + " y " + std::to_string(this->towerY);
}

long long AssociateMessage::getDroneId() {
    return this->droneId;
}

int AssociateMessage::getTowerX() {
    return this->towerX;
}

int AssociateMessage::getTowerY() {
    return this->towerY;
}

void AssociateMessage::setTowerX(int towerX) {
    this->towerX = towerX;
}

void AssociateMessage::setTowerY(int towerY) {
    this->towerY = towerY;
}
// Drone Info Message Class


DroneInfoMessage::DroneInfoMessage(std::string id) : Message(id) {
    this->droneId = -1;
    this->posX = 0;
    this->posY = 0;
    this->batteryAutonomy = 0;
    this->chargeTime = 0;
    this->state = READY;
}

DroneInfoMessage::DroneInfoMessage(long long messageId) : Message(messageId) {
    this->droneId = -1;
    this->posX = 0;
    this->posY = 0;
    this->batteryAutonomy = 0;
    this->chargeTime = 0;
    this->state = READY;
} 

void DroneInfoMessage::parseResponse(RedisResponse* response) {
    if (response->hasError()) {
        logChannelError("Can't parse Drone Info: " + response->getError());
        return;
    }
    if (response->getType() == VECTOR) {
        std::vector<std::string> data = response->getVectorContent();
        for (int i = 0; i < data.size(); i+=2) {
            std::string key = data[i];
            if (key.compare("type") == 0) {
                continue;
            }
            std::string value = data[i + 1];
            if (key.compare("droneId") == 0) {
                this->droneId = std::stoll(value);
            } else if (key.compare("x") == 0) {
                this->posX = std::stoi(value);
            } else if (key.compare("y") == 0) {
                this->posY = std::stoi(value);
            } else if (key.compare("battery_autonomy") == 0) {
                this->batteryAutonomy = std::stoll(value);
            } else if (key.compare("charge_time") == 0) {
                this->chargeTime = std::stoll(value);
            } else if (key.compare("state") == 0) {
                this->state = static_cast<DroneState>(std::stoi(value));
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
    message += " charge_time " + std::to_string(this->chargeTime);
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

long long DroneInfoMessage::getChargeTime() {
    return this->chargeTime;
}

DroneState DroneInfoMessage::getState() {
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

void DroneInfoMessage::setChargeTime(long long chargeTime) {
    this->chargeTime = chargeTime;
}

void DroneInfoMessage::setState(DroneState state) {
    this->state = state;
}

// Location Message Class

LocationMessage::LocationMessage(std::string id) : Message(id) {
    
}

LocationMessage::LocationMessage(long long messageId) : Message(messageId) {
    
}

void LocationMessage::parseResponse(RedisResponse* response) {
    if (response->hasError()) {
        logChannelError("Can't parse Location: " + response->getError());
        return;
    }
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
            } else if (key.compare("movement_type") == 0) {
                this->movementType = std::stoi(value);
            }
        }
    }
}

std::string LocationMessage::parseMessage() {
    std::string data = "type " + std::to_string(this->getType());
    data = data + " x " + std::to_string(this->x);
    data = data + " y " + std::to_string(this->y);
    data = data + " movement_type " + std::to_string(this->movementType);
    return data;
}

void LocationMessage::setLocation(int x, int y) {
    this->x = x;
    this->y = y;
}

void LocationMessage::setMovementType(int type) {
    this->movementType = type;
}

int LocationMessage::getX() {
    return this->x;
}

int LocationMessage::getY() {
    return this->y;
}

int LocationMessage::getMovementType() {
    return this->movementType;
}

// Retire Message
RetireMessage::RetireMessage(std::string id) : Message(id) {
    
}

RetireMessage::RetireMessage(long long messageId) : Message(messageId) {
    
}

void RetireMessage::parseResponse(RedisResponse* response) {
    if (response->hasError()) {
        logChannelError("Can't parse Retire: " + response->getError());
        return;
    }
}

std::string RetireMessage::parseMessage() {
    std::string data = "type " + std::to_string(this->getType());
    return data;
}

// Disconnect Message
DisconnectMessage::DisconnectMessage(std::string id) : Message(id) {
    
}

DisconnectMessage::DisconnectMessage(long long messageId) : Message(messageId) {
    
}

void DisconnectMessage::parseResponse(RedisResponse* response) {
    if (response->hasError()) {
        logChannelError("Can't parse Disconnection: " + response->getError());
        return;
    }
}

std::string DisconnectMessage::parseMessage() {
    std::string data = "type " + std::to_string(this->getType());
    return data;
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
        logChannelError("Can't connect Reading Client!");
    }
    this->readingLock.unlock();
    this->writingLock.lock();
    if (this->writingClient == nullptr) {
        this->writingClient = new Redis();
    }
    if (!connectClient(this->writingClient, ip, port)) {
        logChannelError("Can't connect Reading Client!");
    }
    this->writingLock.unlock();
    return this->isUp();
}

bool Channel::disconnect(){
    this->readingLock.lock();
    this->readingClient->disconnect();
    this->readingLock.unlock();
    this->writingLock.lock();
    this->writingClient->disconnect();
    this->writingLock.unlock();
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
    logChannelDebug("Sending Command: [" + command + "]");
    // Only one thread can write at a time
    RedisResponse *response = this->sendWriteCommand(command);
    if (response->hasError()) {
        if (response->getType() == NONE) {
            logChannelWarning("Timeout writing message!");
        } else {
            logChannelError(response->getError());
        }
        delete response;
        return false;
    }
    delete response;
    // Insert message on receiver channel 
    command = "lpush c:" + std::to_string(channelId) + " " + messageId;
    logChannelDebug("Sending Command: [" + command + "]");
    response = this->sendWriteCommand(command);
    if (response->hasError()) {
        if (response->getType() == NONE) {
            logChannelWarning("Timeout writing message");
        } else {
            logChannelError(response->getError());
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
            logChannelWarning("Timeout deleting message");
        } else {
            logChannelError(response->getError());
        }
        delete response;
        return false;
    }
    delete response;
    return true;
}

bool Channel::flush() {
    if (!this->isUp()) {
        return false;
    }
    RedisResponse *response = this->sendReadCommand("FLUSH ALL");
    if (response->hasError()) {
        if (response->getType() == NONE) {
            logChannelWarning("Timeout on flushing");
        } else {
            logChannelError(response->getError());
        }
        delete response;
        return false;
    }
    return true;
}

// Reading Operations

Message* Channel::readMessageWithId(std::string messageId) {
    RedisResponse *response = this->sendReadCommand("HGETALL " + messageId);
    if (response->hasError()) {
        if (response->getType() == NONE) {
            logChannelWarning("Timeout on getting message data");
        } else {
            logChannelError(response->getError());
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
            m = new AssociateMessage(messageId, -1);
            break;
        case 1:
            m = new PingMessage(messageId);
            break;
        case 2:
            m = new DroneInfoMessage(messageId);
            break;
        case 3:
            m = new LocationMessage(messageId);
            break;
        case 4:
            m = new RetireMessage(messageId);
            break;
        case 5:
            m = new DisconnectMessage(messageId);
            break;
        default:
            logChannelWarning("Unhandled Type: " + std::to_string(messageType));
            break;
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
            logChannelWarning("Timeout on RPOP");
        } else {
            logChannelError(response->getError()); 
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
        logChannelError(response->getError());
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
            logChannelWarning("Timeout on BRPOP");
        } else {
            logChannelError(response->getError());
        }
        delete response;
        return nullptr;
    }
    if (response->getType() != VECTOR) {
        logChannelError("Unexpected behaviour from redis. Await Type:" + std::to_string(response->getType()));
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