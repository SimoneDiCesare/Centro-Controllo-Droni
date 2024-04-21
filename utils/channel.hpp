#ifndef CHANNEL_HPP
#define CHANNEL_HPP
#include "redis.hpp"
#include <string>

class Message {
    public:
        Message(std::string id);
        Message(long long messageId);
        ~Message();
        virtual void parseResponse(RedisResponse*) = 0;
        virtual std::string parseMessage() = 0;
        long long getMessageId();
        int getChannelId();
        int getType();
    protected:
        int type;
    private:
        long long messageId;
        int channelId;
};

class PingMessage : public Message {
    public:
        PingMessage(std::string id);
        PingMessage(long long messageId);
        void parseResponse(RedisResponse*);
        std::string parseMessage();
};

class AssociateMessage : public Message {
    public:
        AssociateMessage(std::string id, long long droneId);
        AssociateMessage(long long messageId, long long droneId);
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
        DroneInfoMessage(long long messageId, long long droneId);
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
        LocationMessage(long long messageId);
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
        RetireMessage(long long messageId, long long droneId);
        void parseResponse(RedisResponse*);
        std::string parseMessage();
};

// TODO: - Implementations in cpp

class DisconnectMessage: public Message {
    public:
        DisconnectMessage(std::string id, long long droneId);
        DisconnectMessage(long long messageId, long long droneId);
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
        bool hasMessage(long long messageId);
        bool isConnected();
        Message* awaitMessage();
        void setTimeout(long timeout = -1);
        bool flush();
    private:
        Redis* redis;
        int id;
};

#endif