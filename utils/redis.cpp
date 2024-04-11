#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <string>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>
#include <vector>
#include "redis.hpp"

/** Redis Response Class
 * It defines a class that wraps a redis response.
 * There are 3 different response that we accept:
 * STRING -> a string response, that can be a status like "OK" or a value retrieved
 * INTEGER -> an base-10 integer.
 * ERROR -> an error string. It also comes with the variable error set to true.
*/

RedisResponse::RedisResponse() {
    this->type = NONE;
    this->error = false;
}

RedisResponse::~RedisResponse() {

}

std::string RedisResponse::getContent() {
    return this->content;
}

std::string RedisResponse::getError() {
    return this->errorString;
}

RedisResponseType RedisResponse::getType() {
    return this->type;
}

std::vector<std::string> RedisResponse::getVectorContent() {
    return this->vectorContent;
}

bool RedisResponse::hasError() {
    return this->error;
}

void RedisResponse::setError(std::string errorString) {
    std::size_t delimiter = errorString.find("\r\n");
    if (delimiter == std::string::npos) {
        this->errorString = errorString;
    } else {
        std::string line = errorString.substr(0, delimiter);
        this->errorString = line;
    }
    this->error = true;
}

void RedisResponse::setContent(std::string content) {
    switch(this->type) {
        case NONE: {
            break;
        }
        case INTEGER: {
            std::size_t delimiter = content.find("\r\n");
            if (delimiter == std::string::npos) {
                this->content = content;
            } else {
                this->content = content.substr(0, delimiter);
            }
            break;
        }
        case STRING:
        case ERROR: {
            // Check empty string -> null result from redis
            if (content.at(0) == '-' && content.at(1) == '1') {
                this->content = "null";
                break;
            }
            std::size_t delimiter = content.find("\r\n");
            if (delimiter == std::string::npos) {
                this->content = content;
            } else {
                std::string line = content.substr(0, delimiter);
                if (content.length() > delimiter + 2) {
                    int contentSize = std::stoi(line);
                    this->content = content.substr(delimiter + 2, delimiter + contentSize - 1);
                } else {
                    this->content = line;
                }
            }
            break;
        }
        case VECTOR: {
            std::size_t delimiter = content.find("\r\n");
            std::string line = content.substr(0, delimiter);
            int start = delimiter + 2;
            int size = std::stoi(line);
            line = content.substr(start, content.length());
            bool skipNext = false;
            for (int i = 0; i < size; i++) {
                delimiter = line.find("\r\n");
                if (skipNext) {
                    line = line.substr(delimiter + 2, content.length());
                    skipNext = false;
                    i -= 1;
                    continue;
                }
                switch(line.at(0)) {
                    case '$': {
                        int lineSize = std::stoi(line.substr(1, delimiter));
                        std::string s = line.substr(delimiter + 2, delimiter + lineSize - 2);
                        // line = line.substr(delimiter + lineSize + 3);
                        this->vectorContent.push_back(s);
                        skipNext = true;
                        break;
                    }
                    default:
                        std::cout << "Need to parse type: " << line.at(0) << "\n";
                        line = line.substr(delimiter + 2, content.length());
                        break;
                }
                line = line.substr(delimiter + 2, content.length());
            }
            break;
        }
        default: {
            std::cout << "INVALID MESSAGE TYPE\n";
            break;
        }
    }
}

void RedisResponse::setType(RedisResponseType type) {
    this->type = type;
}

/** Redis Client Class
 * It represent a redis client connection. Via this we can send command to redis and retrieve
    RedisResponse.
*/

Redis::Redis() {
    this->connected = false;
    this->sockFd = 0;
}

bool Redis::setTimeout(long sec) {
    struct timeval timeout;
    timeout.tv_sec = sec;
    timeout.tv_usec = 0;
    if (setsockopt(this->sockFd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
        // close(this->sockFd);
        // this->connected = false;
        return false;
    }
    return true;
}

bool Redis::connect(std::string host, int port) {
    this->sockFd = socket(AF_INET, SOCK_STREAM, 0);
    if (this->sockFd < 0) {
        this->connected = false;
        return false;
    }
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    if (inet_pton(AF_INET, host.c_str(), &serverAddr.sin_addr) <= 0) {
        close(this->sockFd);
        this->connected = false;
        return false;
    }
    if (::connect(this->sockFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        close(this->sockFd);
        this->connected = false;
        return false;
    }
    this->connected = true;
    return true;
}

RedisResponse* Redis::sendCommand(std::string command) {
    if (command.at(command.length() - 1) != '\n') {
        command += "\n";
    }
    RedisResponse *response = new RedisResponse();
    if (send(this->sockFd, command.c_str(), command.length(), 0) == -1) {
        response->setType(NONE);
        response->setError("Send Error");
        return response;
    }
    char controlByte[1];
    if (recv(this->sockFd, controlByte, 1, 0) == -1) {
        response->setType(NONE);
        response->setError("Connection Error");
        return response;
    }
    char dataBuffer[512];
    if (recv(this->sockFd, dataBuffer, 512, 0) == -1) {
        response->setType(NONE);
        response->setError("Data retrieving Error");
        return response;
    }
    // std::cout << "Control Byte: " << controlByte << "\n";
    switch(controlByte[0]) {
        case '+':
            response->setType(STRING);
            response->setContent(dataBuffer);
            break;
        case '-':
            response->setType(ERROR);
            response->setError(dataBuffer);
            break;
        case ':':
            response->setType(INTEGER);
            response->setContent(dataBuffer);
            break;
        case '$':
            response->setType(STRING);
            response->setContent(dataBuffer);
            break;
        case '*':
            response->setType(VECTOR);
            response->setContent(dataBuffer);
            break;
        default:
            response->setType(NONE);
            response->setContent(dataBuffer);
            response->setError("Invalid Parsing Type");
            break;
    }
    return response;
}

bool Redis::isConnected() {
    return this->connected;
}

Redis::~Redis() {
    if (this->connected > 0) {
        close(sockFd);
    }
}

