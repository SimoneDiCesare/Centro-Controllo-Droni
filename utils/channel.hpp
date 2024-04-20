#ifndef CHANNEL_HPP
#define CHANNEL_HPP
#include "redis.hpp"
#include <string>

class Message {
    public:
        Message(std::string id);
        Message(int messageId);
        ~Message();
        virtual void parseResponse(RedisResponse*) = 0;
        virtual std::string parseMessage() = 0;
        int getMessageId();
        int getChannelId();
        int getType();
    protected:
        int type;
    private:
        int messageId;
        int channelId;
};

class PingMessage : public Message {
    public:
        PingMessage(std::string id);
        PingMessage(int messageId);
        void parseResponse(RedisResponse*);
        std::string parseMessage();
};

class AssociateMessage : public Message {
    public:
        AssociateMessage(std::string id, long long droneId);
        AssociateMessage(int messageId, long long droneId);
        void parseResponse(RedisResponse*);
        std::string parseMessage();
        long long getDroneId();
    private:
        long long droneId;
};

// TODO: - Define in channel.cpp

class DroneInfoMessage : public Message {
    public:
        DroneInfoMessage(std::string id, long long droneId);
        DroneInfoMessage(int messageId, long long droneId);
        void parseResponse(RedisResponse*);
        std::string parseMessage();
        long long getDroneId();
        int getX() {return 0;};
    private:
        // TODO: - Fill Fields
        long long droneId;
};

class LocationMessage : public Message {
    public:
        LocationMessage(std::string id);
        LocationMessage(int messageId);
        void parseResponse(RedisResponse*);
        std::string parseMessage();
        int getX();
        int getY();
    private:
        int x;
        int y;
};

class RetireMessage : public Message {
    public:
        RetireMessage(std::string id, long long droneId);
        RetireMessage(int messageId, long long droneId);
        void parseResponse(RedisResponse*);
        std::string parseMessage();
};

class DisconnectMessage: public Message {
    public:
        DisconnectMessage(std::string id, long long droneId);
        DisconnectMessage(int messageId, long long droneId);
        void parseResponse(RedisResponse*);
        std::string parseMessage();
        bool getStatus();
    private:
        bool status;
};

class Channel {
    public:
        Channel(int id);
        ~Channel();
        bool connect(std::string ip = "127.0.0.1", int port = 6379);
        bool sendMessageTo(int channelId, Message& message);
        bool isConnected();
        Message* awaitMessage();
        void setTimeout(long timeout = -1);
        bool flush();
    private:
        Redis* redis;
        int id;
};

#endif