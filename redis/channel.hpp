#ifndef CHANNEL_HPP
#define CHANNEL_HPP
#include "redis.hpp"
#include <string>
#include <vector>
#include <tuple>
#include <mutex>

class Message {
    public:
        Message(std::string id);
        Message(long long messageId);
        ~Message();
        virtual void parseResponse(RedisResponse*) = 0;
        virtual std::string parseMessage() = 0;
        long long getMessageId();
        long long getChannelId();
        std::string getFormattedId();
        virtual int getType() {return -1;};
    private:
        long long messageId;
        long long channelId;
};

class PingMessage : public Message {
    public:
        PingMessage(std::string id);
        PingMessage(long long messageId);
        void parseResponse(RedisResponse*);
        std::string parseMessage();
        int getType() {return 0;};
};

class AssociateMessage : public Message {
    public:
        AssociateMessage(std::string id, long long droneId);
        AssociateMessage(long long messageId, long long droneId);
        void parseResponse(RedisResponse*);
        std::string parseMessage();
        long long getDroneId();
        int getType() {return 1;};
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
        // Getter
        long long getDroneId();
        int getPosX();
        int getPosY();
        long long getBatteryAutonomy();
        long long getBatteryLife();
        int getState(); 
        int getType() {return 2;};
        // Setter
        void setDroneId(long long id);
        void setPosX(int x);
        void setPosY(int y);
        void setBatteryAutonomy(long long batteryAutonomy);
        void setBatteryLife(long long batteryLife);
        void setState(int state);
    private:
        // TODO: - Fill Fields
        long long droneId;
        int posX;
        int posY;
        long long batteryAutonomy;
        long long batteryLife;
        int state;
        // int rangeOfAction;
        // int velocity;
};

class LocationMessage : public Message {
    public:
        LocationMessage(std::string id);
        LocationMessage(long long messageId);
        void parseResponse(RedisResponse*);
        std::string parseMessage();
        std::tuple<char, int> getLocation(int i);
        int getStepCount();
        int getType() {return 3;};
    private:
        std::vector<std::tuple<char,int>> locations;
};

class RetireMessage : public Message {
    public:
        RetireMessage(std::string id, long long droneId);
        RetireMessage(long long messageId, long long droneId);
        void parseResponse(RedisResponse*);
        std::string parseMessage();
        int getType() {return 4;};
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
        // Constructor and Destructor
        Channel(long long id);
        ~Channel();
        // Connection Functions
        bool connect(std::string ip = "127.0.0.1", int port = 6379);
        bool disconnect();
        // Writing Operations
        bool sendMessageTo(long long channelId, Message* message);
        bool removeMessage(Message *message);
        bool flush();
        // Reading Operations
        Message* readMessage();
        Message* awaitMessage(long timeout = 0);
        // Getter
        long long getId();
        bool canRead();
        bool canWrite();
        bool isUp();
        // Setter
        void setId(long long id);
    private:
        // Params
        std::mutex readingLock;
        std::mutex writingLock;
        Redis* readingClient;
        Redis* writingClient;
        long long id;
        // Utility Functions
        RedisResponse* sendWriteCommand(std::string command);
        RedisResponse* sendReadCommand(std::string command);
        Message* readMessageWithId(std::string messageId);
};

/*class Channel {
    public:
        Channel(long long id);
        ~Channel();
        bool connect(std::string ip = "127.0.0.1", int port = 6379);
        bool sendMessageTo(long long channelId, Message& message);
        bool hasMessage(long long messageId);
        bool removeMessage(Message *message);
        bool isConnected();
        Message* awaitMessage(long timeout = 0);
        bool flush();
        void setId(long long id);
    private:
        Redis* redis;
        long long id;
};*/

#endif