#include "channel.hpp"
#include "redis.hpp"
#include <iostream>
#include <string>
#include <vector>

// Abstract Class Message 

Message::Message(std::string id) {
    std::size_t delimiter = id.find(":");
    if (delimiter == std::string::npos) {
        std::cout << "Invalid ID: " << id << "\n";
        return;
    } else {
        this->channelId = std::stoi(id.substr(0, delimiter));
        this->messageId = std::stoi(id.substr(delimiter + 1, id.length()));
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

int Message::getChannelId() {
    return this->channelId;
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
        for (int i = 0; i < data.size(); i++) {
            std::string value = data[i];
            if (value.compare("droneId") == 0) {
                this->droneId = std::stoll(data[i + 1]);
            }
        }
    }
}

std::string DroneInfoMessage::parseMessage() {
    return "type " + std::to_string(this->getType()) + " droneId " + std::to_string(this->droneId);
}

long long DroneInfoMessage::getDroneId() {
    return this->droneId;
}

// Location Message Class

LocationMessage::LocationMessage(std::string id) : Message(id) {
    
}

LocationMessage::LocationMessage(long long messageId) : Message(messageId) {
    
}

void LocationMessage::parseResponse(RedisResponse* response) {
    if (response->getType() == VECTOR) {
        std::vector<std::string> data = response->getVectorContent();
        for (int i = 0; i < data.size(); i++) {
            std::string value = data[i];
            if (value.compare("x") == 0) {
                this->x = std::stoi(data[i + 1]);
            }
            if (value.compare("y") == 0) {
                this->y = std::stoi(data[i + 1]);
            }
        }
    }
}

std::string LocationMessage::parseMessage() {
    return "type " + std::to_string(this->getType()) + " x " + std::to_string(this->x) + " y " + std::to_string(this->y);
}

int LocationMessage::getX() {
    return this->x;
}

int LocationMessage::getY() {
    return this->y;
}

// Channel Class

Channel::Channel(int id) {
    this->id = id;
    this->redis = nullptr;
}

Channel::~Channel() {
    delete this->redis;
}

bool Channel::connect(std::string ip /*= "127.0.0.1"*/, int port /*= 6379*/) {
    if (this->redis == nullptr) {
        this->redis = new Redis();
    }
    if (this->redis->isConnected()) {
        std::cout << "Channel Already Connected\n";
        return true;
    }
    return this->redis->connect(ip, port);
}

bool Channel::isConnected() {
    if (this->redis == nullptr) {
        return false;
    }
    return this->redis->isConnected();
}

bool Channel::hasMessage(long long messageId) {
    if (!this->redis->isConnected()) {
        std::cout << "Channel not connected!\n";
        return false;
    }
    RedisResponse *response = this->redis->sendCommand("KEYS m:" + std::to_string(this->id) + ":" + std::to_string(messageId));
    if (response->hasError()) {
        if (response->getType() == NONE) {
            std::cout << "Timeout\n";
            return false;
        } else {
            std::cout << "Error checking message:\n\t" << response->getError() << "\n";
            return false;
        }
    }
    if (response->getType() == VECTOR) {
        return response->getVectorContent().size() > 0;
    }
    return false;
}

bool Channel::removeMessage(Message *message) {
    if (!this->redis->isConnected()) {
        std::cout << "Channel not connected!\n";
        return false;
    }
    std::string key = "m:" + std::to_string(message->getChannelId()) + ":" + std::to_string(message->getMessageId());
    RedisResponse *response = this->redis->sendCommand("DEL " + key);
    if (response->hasError()) {
        if (response->getType() == NONE) {
            std::cout << "Timeout\n";
            return false;
        } else {
            std::cout << "Error checking message:\n\t" << response->getError() << "\n";
            return false;
        }
    }
    return true;
}

bool Channel::sendMessageTo(int channelId, Message& message) {
    if (!this->redis->isConnected()) {
        std::cout << "Channel not connected!\n";
        return false;
    }
    std::string messageId = "m:" + std::to_string(this->id) + ":" + std::to_string(message.getMessageId()); 
    std::string command = "hset " + messageId + " " + message.parseMessage();
    std::cout << "Sending: " << command << "\n";
    RedisResponse *response = this->redis->sendCommand(command);
    if (response->hasError()) {
        if (response->getType() == NONE) {
            std::cout << "Timeout\n";
            return false;
        } else {
            std::cout << "Error creating message:\n\t" << response->getError() << "\n";
            return false;
        }
    }
    delete response;
    command = "lpush c:" + std::to_string(channelId) + " " + messageId;
    std::cout << "Sending: " << command << "\n";
    response = this->redis->sendCommand(command);
    if (response->hasError()) {
        if (response->getType() == NONE) {
            std::cout << "Timeout\n";
            return false;
        } else {
            std::cout << "Error sending message:\n\t" << response->getError() << "\n";
            return false;
        }
    }
    delete response;
    return true;
}

void Channel::setTimeout(long timeout) {
    if (timeout != -1) {
        bool succes = this->redis->setTimeout(timeout);
        if (!succes) {
            std::cout << "Can't set timeout\nRunning until a message is received\n";
        }
    }
}

Message* Channel::awaitMessage() {
    std::string channelId = "c:" + std::to_string(this->id);
    RedisResponse *response = this->redis->sendCommand("RPOP " + channelId);
    if (response->hasError()) {
        if (response->getType() == NONE) {
            std::cout << "Timeout on RPOP\n";
        } else {
            std::cout << "Error reading channel queue:\n\t" << response->getError() << "\n";    
        }
        delete response;
        return nullptr;
    }
    // Check if queue is empty
    if (response->getType() == NLL) {
        delete response;
        return nullptr;
    }
    std::string messageId = response->getContent(); 
    delete response;
    response = this->redis->sendCommand("HGETALL " + messageId);
    if (response->hasError()) {
        if (response->getType() == NONE) {
            std::cout << "Timeout on HGETALL\n";
        } else {
            std::cout << "Error reading message:\n\t" << response->getError() << "\n";
        }
        delete response;
        return nullptr;
    }
    // Convert in the format channelId:messageId
    messageId = messageId.substr(2, messageId.length());
    int messageType = -1;
    Message *m = nullptr;
    std::vector<std::string> params = response->getVectorContent();
    for (int i = 0; i < params.size(); i++) {
        if (params[i].compare("type") == 0 && i < params.size() - 1) {
            messageType = std::stoi(params[i + 1]);
            break;
        }
    }
    if (messageType == -1) {
        // std::cout << "Can't analyze message without a type!\n";
        delete response;
        return nullptr;
    }
    switch(messageType) {
        case 0:
            m = new PingMessage(messageId);
            break;
        case 1:
            m = new AssociateMessage(messageId, -1);
            break;
        default:
            std::cout << "Unhandled type: " << messageType << "\n";
    }
    if (m != nullptr) {
        m->parseResponse(response);
    }
    delete response;
    return m;
}

bool Channel::flush() {
    RedisResponse *response = this->redis->sendCommand("del c:" + this->id);
    if (response->hasError()) {
        if (response->getType() == NONE) {
            std::cout << "Timeout";
        } else {
            std::cout << "Error while flushing:\n\t" << response->getError() << "\n";
        }
        delete response;
        return false;
    }
    delete response;
    return true;
}