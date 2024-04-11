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

class Channel {
    public:
        Channel(int id);
        ~Channel();
        bool connect(std::string ip = "127.0.0.1", int port = 6379);
        bool sendMessageTo(int channelId, Message& message);
        bool isConnected();
        Message* awaitMessage(long timeout = -1);
        bool flush();
    private:
        Redis* redis;
        int id;
};

#endif