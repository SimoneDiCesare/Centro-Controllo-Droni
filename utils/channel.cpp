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

Message::Message(int messageId) {
    this->messageId = messageId;
    this->channelId = -1;
}

Message::~Message() {

}

int Message::getMessageId() {
    return this->messageId;
}

int Message::getChannelId() {
    return this->channelId;
}

int Message::getType() {
    return this->type;
}

// Ping Message

PingMessage::PingMessage(std::string id) : Message(id) {
    this->type = 0;
}

PingMessage::PingMessage(int messageId) : Message(messageId) {
    this->type = 0;
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
    return "type " + std::to_string(this->type);
}

// AssociateMessage

AssociateMessage::AssociateMessage(std::string id, int droneId) : Message(id) {
    this->droneId = droneId;
    this->type = 1;
}

AssociateMessage::AssociateMessage(int messageId, int droneId) : Message(messageId) {
    this->droneId = droneId;
    this->type = 1;
} 

void AssociateMessage::parseResponse(RedisResponse* response) {
    if (response->getType() == VECTOR) {
        std::vector<std::string> data = response->getVectorContent();
        for (int i = 0; i < data.size(); i++) {
            std::string value = data[i];
            if (value.compare("droneId") == 0) {
                this->droneId = std::stoi(data[i + 1]);
            }
        }
    }
}

std::string AssociateMessage::parseMessage() {
    return "type " + std::to_string(this->type) + " droneId " + std::to_string(this->droneId);
}

int AssociateMessage::getDroneId() {
    return this->droneId;
}

// Drone Info Message Class
// TODO: Add all drone infos

DroneInfoMessage::DroneInfoMessage(std::string id, int droneId) : Message(id) {
    this->droneId = droneId;
    this->type = 2;
}

DroneInfoMessage::DroneInfoMessage(int messageId, int droneId) : Message(messageId) {
    this->droneId = droneId;
    this->type = 2;
} 

void DroneInfoMessage::parseResponse(RedisResponse* response) {
    if (response->getType() == VECTOR) {
        std::vector<std::string> data = response->getVectorContent();
        for (int i = 0; i < data.size(); i++) {
            std::string value = data[i];
            if (value.compare("droneId") == 0) {
                this->droneId = std::stoi(data[i + 1]);
            }
        }
    }
}

std::string DroneInfoMessage::parseMessage() {
    return "type " + std::to_string(this->type) + " droneId " + std::to_string(this->droneId);
}

int DroneInfoMessage::getDroneId() {
    return this->droneId;
}

// Location Message Class

LocationMessage::LocationMessage(std::string id) : Message(id) {
    this->type = 3;
}

LocationMessage::LocationMessage(int messageId) : Message(messageId) {
    this->type = 3;
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
    return "type " + std::to_string(this->type) + " x " + std::to_string(this->x) + " y " + std::to_string(this->y);
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

Message* Channel::awaitMessage(long timeout) {
    if (timeout != -1) {
        bool succes = this->redis->setTimeout(timeout);
        if (!succes) {
            std::cout << "Can't set timeout\nRunning until a message is received\n";
        }
    }
    std::string channelId = "c:" + std::to_string(this->id);
    RedisResponse *response = this->redis->sendCommand("LLEN " + channelId);
    if (response->hasError()) {
        if (response->getType() == NONE) {
            std::cout << "Timeout\n";
        } else {
            std::cout << "Error reading channel queue:\n\t" << response->getError() << "\n";
        }
        delete response;
        return NULL;
    } else {
        int queueLength = std::stoi(response->getContent());
        // std::cout << "Messages in queue: " << queueLength << "\n";
        if (queueLength > 0) {
            delete response;
            response = this->redis->sendCommand("RPOP " + channelId);
            if (response->hasError()) {
                if (response->getType() == NONE) {
                    std::cout << "Timeout\n";
                } else {
                    std::cout << "Error reading channel queue:\n\t" << response->getError() << "\n";    
                }
                delete response;
                return NULL;
            }
            std::string messageId = response->getContent();
            delete response;
            response = this->redis->sendCommand("HGET " + messageId + " type");
            if (response->hasError()) {
                if (response->getType() == NONE) {
                    std::cout << "Timeout\n";
                } else {
                    std::cout << "Error reading message:\n\t" << response->getError() << "\n";    
                }
                delete response;
                return NULL;
            }
            if (response->getContent().compare("null") == 0) {
                std::cout << "Malformatted Message " << messageId << ": null type";
                delete response;
                return NULL;
            }
            int messageType = std::stoi(response->getContent());
            delete response;
            response = this->redis->sendCommand("HGETALL " + messageId);
            if (response->hasError()) {
                if (response->getType() == NONE) {
                    std::cout << "Timeout\n";
                } else {
                    std::cout << "Error reading message:\n\t" << response->getError() << "\n";    
                }
                delete response;
                return NULL;
            }
            // Convert in the format channelId:messageId
            messageId = messageId.substr(2, messageId.length());
            Message *m = NULL;
            // TODO: - add message types here
            //       - delete message from redis
            switch(messageType) {
                case 0:
                    m = new PingMessage(messageId);
                    break;
                case 1:
                    m = new AssociateMessage(messageId, -1);
                    break;
                default:
                    std::cout << "Type not Handled: " << messageType << "\n";
                    break;
            }
            if (m != NULL) {
                m->parseResponse(response);
            }
            delete response;
            return m;
        }
    }
    delete response;
    return NULL;
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