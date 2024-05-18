#ifndef CHANNEL_HPP
#define CHANNEL_HPP
#include "redis.hpp"
#include <string>
#include <vector>
#include <tuple>
#include <mutex>

/**
 * @class Message
 * @brief Abstract Class representing a Message used in Tower<->Drone comunication via Redis.
 * 
 * This Abstract Class represents a Message used for info exchanges between the Tower and the Drones via a Redis Connection.
 */
class Message {
    public:
        /**
         * @brief Constructor of Message class.
         * @param id The complete id of the message "m:{cid}:{mid}"
         * NOTE: Used when reading from Redis.
         */
        Message(std::string id);
        /**
         * @brief Constructor of Message class.
         * @param messageId The id of this message.
         * NOTE: Used when writing a message to Redis.
         */
        Message(long long messageId);
        /**
         * @brief Destructor of Message class.
         */
        ~Message();
        /**
         * @brief Virtual method for parsing the content of a RedisResponse to this Message.
         * @param response The response in which the data is contained.
         */
        virtual void parseResponse(RedisResponse* response) = 0;
        /**
         * @brief Virtual method for parsing the content of this message into a list "key value" for Redis.
         * @return A String representing this message.
         */
        virtual std::string parseMessage() = 0;
        /**
         * @brief Get the id of this message.
         * @return The message id.
         */
        long long getMessageId();
        /**
         * @brief Get the channel associated to this message.
         * @return The channel id.
         */
        long long getChannelId();
        /**
         * @brief Get the id of this message in the following format "{cid}:{mid}".
         * @return The id of this message formatted.
         */
        std::string getFormattedId();
        /**
         * @brief Get the type of this message. -1 because this class cannot be instantiated directly.
         * @return The type of this message.
         * NOTE: Each Message extending this class needs to have unique type.
        */
        virtual int getType() {return -1;};
    private:
        long long messageId; ///< The id of the message.
        long long channelId; ///< The id of the channel associated to this message. 
};

/**
 * @class AssociateMessage
 * @brief Class representing an Association Between a Drone and the Tower.
 * 
 * This Class is an implementation of the Message Class.
 * Its used for Associating a Drone to the Tower.
 * The Drone send this message with a temporary id, awaiting a new AssociateMessage with the actual id used by Tower for comunicating with it.
 */
class AssociateMessage : public Message {
    public:
        /**
         * @brief Constructor of AssociateMessage class.
         * @param id The complete id of the message "m:{cid}:{mid}"
         * @param droneId The id of the associating drone.
         * @see Message::Message(std::string)
         */
        AssociateMessage(std::string id, long long droneId);
        /**
         * @brief Constructor of AssociateMessage class.
         * @param messageId The id of this message.
         * @param droneId The id of the associating drone.
         * @see Message::Message(long long)
         */
        AssociateMessage(long long messageId, long long droneId);
        /**
         * @brief Method for parsing the content of a RedisResponse to an AssociateMessage.
         * @param response The response in which the data is contained.
         * 
         * This methods overrides the parseResponse methods of the base class Message.
         * 
         * @see Message::parseResponse()
         */
        void parseResponse(RedisResponse* response);
        /**
         * @brief Method for parsing the content of this message into a list "key value" for Redis.
         * @return A String representing this message.
         * 
         * This method overrides the parseMessage method of the base class Message.
         * 
         * @see Message::parseMessage()
         */
        std::string parseMessage();
        /**
         * @brief Get the drone id
         * @return The drone id
        */
        long long getDroneId();
        /**
         * @brief Get the type of this message.
         * @return 0, this message type.
         * 
         * This method overrides the getType method of the base class Message.
         * 
         * @see Message::getType()
        */
        int getType() {return 0;};
    private:
        long long droneId; ///< The id of the drone.
};

/**
 * @class PingMessage
 * @brief Class representing a Ping from a device to another via Redis.
 * 
 * This Class is an implementation of the Message Class.
 * Its used for Pinging an Actor to another (Tower to Drone or viceversa).
 */
class PingMessage : public Message {
    public:
        /**
         * @brief Constructor of PingMessage class.
         * @param id The complete id of the message "m:{cid}:{mid}"
         * @see Message::Message(std::string)
         */
        PingMessage(std::string id);
        /**
         * @brief Constructor of PingMessage class.
         * @param messageId The id of this message.
         * @see Message::Message(long long)
         */
        PingMessage(long long messageId);
        /**
         * @brief Method for parsing the content of a RedisResponse to a PingMessage.
         * @param response The response in which the data is contained.
         * 
         * This methods overrides the parseResponse methods of the base class Message.
         * 
         * @see Message::parseResponse()
         */
        void parseResponse(RedisResponse*);
        /**
         * @brief Method for parsing the content of this message into a list "key value" for Redis.
         * @return A String representing this message.
         * 
         * This method overrides the parseMessage method of the base class Message.
         * 
         * @see Message::parseMessage()
         */
        std::string parseMessage();
        /**
         * @brief Get the type of this message.
         * @return 1, this message type.
         * 
         * This method overrides the getType method of the base class Message.
         * 
         * @see Message::getType()
        */
        int getType() {return 1;};
};

/**
 * @class DroneInfoMessage
 * @brief Class representing the exchanges of Info between a Drone and the Tower.
 * 
 * This Class is an implementations of the Message class.
 * Its used for exchanges the drone's infos between him and the tower.
 */
class DroneInfoMessage : public Message {
    public:
        /**
         * @brief Constructor of DroneInfoMessage class.
         * @param id The complete id of the message "m:{cid}:{mid}"
         * @see Message::Message(std::string)
         */
        DroneInfoMessage(std::string id);
        /**
         * @brief Constructor of DroneInfoMessage class.
         * @param messageId The id of this message.
         * @see Message::Message(std::string)
         */
        DroneInfoMessage(long long messageId);
        /**
         * @brief Method for parsing the content of a RedisResponse to a DroneInfoMessage.
         * @param response The response in which the data is contained.
         * 
         * This methods overrides the parseResponse methods of the base class Message.
         * 
         * @see Message::parseResponse()
         */
        void parseResponse(RedisResponse*);
        /**
         * @brief Method for parsing the content of this message into a list "key value" for Redis.
         * @return A String representing this message.
         * 
         * This method overrides the parseMessage method of the base class Message.
         * 
         * @see Message::parseMessage()
         */
        std::string parseMessage();
        /**
         * @brief Get the type of this message.
         * @return 2, this message type.
         * 
         * This method overrides the getType method of the base class Message.
         * 
         * @see Message::getType()
        */
        int getType() {return 2;};
        /**
         * @return The id of the drone.
         */
        long long getDroneId();
        /**
         * @return The x position of the drone.
         */
        int getPosX();
        /**
         * @return The y position of the drone.
         */
        int getPosY();
        /**
         * @return The battery autonomy of this drone in seconds.
         */
        long long getBatteryAutonomy();
        /**
         * @return The total battery life of this drone in seconds.
         */
        long long getBatteryLife();
        /**
         * @return The state of this drone.
         */
        int getState();
        /**
         * @brief Set the id of this drone.
         * @param id The new drone id.
         */
        void setDroneId(long long id);
        /**
         * @brief Set the x position of this drone.
         * @param x The new x position.
         */
        void setPosX(int x);
        /**
         * @brief Set the y position of this drone.
         * @param y The new y position.
         */
        void setPosY(int y);
        /**
         * @brief Set the autonomy of the drone in seconds.
         * @param batteryAutonomy The new battery autonomy in seconds.
         */
        void setBatteryAutonomy(long long batteryAutonomy);
        /**
         * @brief Set the total battery life in seconds.
         * @param batteryLife The new batterty life in seconds.
         */
        void setBatteryLife(long long batteryLife);
        /**
         * @brief Set the state of thid drone.
         * @param state The new drone state.
         */
        void setState(int state);
    private:
        long long droneId;         ///< The drone id.
        int posX;                  ///< The x position of the drone.
        int posY;                  ///< The y position of the drone.
        long long batteryAutonomy; ///< The battery autonomy in seconds.
        long long batteryLife;     ///< The total battery duration in seconds.
        int state;                 ///< The drone state.
};

/**
 * @class LocationMessage
 * @brief Class representing a location (x, y) for a drone.
 * 
 * This Class is an implementations of the Message class.
 * Its used for updating the position of a drone.
 */
class LocationMessage : public Message {
    public:
        /**
         * @brief Constructor of LocationMessage class.
         * @param id The complete id of the message "m:{cid}:{mid}"
         * @see Message::Message(std::string)
         */
        LocationMessage(std::string id);
        /**
         * @brief Constructor of LocationMessage class.
         * @param messageId The id of this message.
         * @see Message::Message(std::string)
         */
        LocationMessage(long long messageId);
        /**
         * @brief Method for parsing the content of a RedisResponse to a LocationMessage.
         * @param response The response in which the data is contained.
         * 
         * This methods overrides the parseResponse methods of the base class Message.
         * 
         * @see Message::parseResponse()
         */
        void parseResponse(RedisResponse*);
        /**
         * @brief Method for parsing the content of this message into a list "key value" for Redis.
         * @return A String representing this message.
         * 
         * This method overrides the parseMessage method of the base class Message.
         * 
         * @see Message::parseMessage()
         */
        std::string parseMessage();
        /**
         * @brief Get the type of this message.
         * @return 3, this message type.
         * 
         * This method overrides the getType method of the base class Message.
         * 
         * @see Message::getType()
        */
        int getType() {return 3;};
        /**
         * @return The x location.
         */
        int getX();
        /**
         * @return The y location.
         */
        int getY();
        /**
         * @return The movement type.
        */
        int getMovementType();
        /**
         * @brief Set the location pointed.
         * @param x The x location.
         * @param y The y location.
         */
        void setLocation(int x, int y);
        /**
         * @brief Set the movement type by which the location is reached.
         * @param type The movement type.
         */
        void setMovementType(int type);
    private:
        int x;            ///< The x location.
        int y;            ///< The y location.
        int movementType; ///< The movement type. This parameter represent the way in which the drone should arrive to the location (diagonal or via linear axis).
};

/**
 * @class RetireMessage
 * @brief Class represeting a Retire update from a drone.
 * 
 * This Class is an implementations of the Message class.
 * Its used to notify the Tower that a drone is lower in battery and need to return for charging. 
 */
class RetireMessage : public Message {
    public:
        /**
         * @brief Constructor of RetireMessage class.
         * @param id The complete id of the message "m:{cid}:{mid}"
         * @see Message::Message(std::string)
         */
        RetireMessage(std::string id);
        /**
         * @brief Constructor of RetireMessage class.
         * @param messageId The id of this message.
         * @see Message::Message(std::string)
         */
        RetireMessage(long long messageId);
        /**
         * @brief Method for parsing the content of a RedisResponse to a RetireMessage.
         * @param response The response in which the data is contained.
         * 
         * This methods overrides the parseResponse methods of the base class Message.
         * 
         * @see Message::parseResponse()
         */
        void parseResponse(RedisResponse*);
        /**
         * @brief Method for parsing the content of this message into a list "key value" for Redis.
         * @return A String representing this message.
         * 
         * This method overrides the parseMessage method of the base class Message.
         * 
         * @see Message::parseMessage()
         */
        std::string parseMessage();
        /**
         * @brief Get the type of this message.
         * @return 4, this message type.
         * 
         * This method overrides the getType method of the base class Message.
         * 
         * @see Message::getType()
        */
        int getType() {return 4;};
};

/**
 * @class DisconnectMessage
 * @brief Class representing a disconnection between the tower and a drone.
 * 
 * This Class is an implementations of the Message class.
 * Its used to disconnect a Drone from the Tower and free up resources.
 */
class DisconnectMessage: public Message {
    public:
        /**
         * @brief Constructor of DisconnectMessage class.
         * @param id The complete id of the message "m:{cid}:{mid}"
         * @see Message::Message(std::string)
         */
        DisconnectMessage(std::string id);
        /**
         * @brief Constructor of DisconnectMessage class.
         * @param messageId The id of this message.
         * @see Message::Message(std::string)
         */
        DisconnectMessage(long long messageId);
         /**
         * @brief Method for parsing the content of a RedisResponse to a DisconnectMessage.
         * @param response The response in which the data is contained.
         * 
         * This methods overrides the parseResponse methods of the base class Message.
         * 
         * @see Message::parseResponse()
         */
        void parseResponse(RedisResponse*);
        /**
         * @brief Method for parsing the content of this message into a list "key value" for Redis.
         * @return A String representing this message.
         * 
         * This method overrides the parseMessage method of the base class Message.
         * 
         * @see Message::parseMessage()
         */
        std::string parseMessage();
        /**
         * @brief Get the type of this message.
         * @return 5, this message type.
         * 
         * This method overrides the getType method of the base class Message.
         * 
         * @see Message::getType()
        */
        int getType() {return 5;};
    private:
};

/**
 * @class Channel
 * @brief Class for handling a channel of an actor inside Redis.
 * 
 * This Class handles the exchanges of messages bewteen actors (Tower and Drones) using Messages and a Redis Client.
 */
class Channel {
    public:
        /**
         * @brief Constructor of Channel class.
         * @param id The id of the channel.
         * NOTE: id 0 is reserved to the Tower.
         */
        Channel(long long id);
        /**
         * @brief Destructor of Channel class.
         * 
         * Automatically disconnects from redis client.
         */
        ~Channel();
        /**
         * @brief Connects to a Redis Client.
         * @param ip The server ip. Default is the local host.
         * @param port The server port. Default is the default specified by Redis Docs.
         * @return True if a connection is enstablished. Flase otherwise.
         */
        bool connect(std::string ip = "127.0.0.1", int port = 6379);
        /**
         * @brief Disconnects from a Redis Client.
         * @return True if it disconnected successfully. False otherwise.
         */
        bool disconnect();
        /**
         * @brief Write a message to a channel.
         * @param channelId The channel receiving the message.
         * @param message A Pointer to the message.
         */
        bool sendMessageTo(long long channelId, Message* message);
        /**
         * @brief Delete a message from Redis.
         * @param message A Pointer to the message.
         * @return True if the message was deleted. False otherwise.
         */
        bool removeMessage(Message *message);
        /**
         * @brief Delete this channel from Redis.
         * @return True if the channel was deleted. False othrwise. 
         */
        bool flush();
        /**
         * @brief Read a Message from the queue.
         * @return A Pointer to the read Message.
         * NOTE: This function is blocking.
         */
        Message* readMessage();
        /**
         * @brief Awaits and read a Message from the queue.
         * @param timeout The timeout in seconds.
         * @return A Pointer to the read Message.
         * 
         * It awaits a message until the timeout is expired or a message is read.
         * NOTE: If the timeout is 0, the function will be blocking.
         */
        Message* awaitMessage(long timeout = 0);
        /**
         * @return The channel id. 
         */
        long long getId();
        /**
         * @return True if this channel can execute read queries from redis.
         */
        bool canRead();
        /**
         * @return True if this channel can execute write queries from redis.
         */
        bool canWrite();
        /**
         * @return True if this channel cna read and can write.
        */
        bool isUp();
        /**
         * @brief Change the channel id.
         * @param id The new channel id.
         */
        void setId(long long id);
    private:
        // Params
        std::mutex readingLock; ///< Mutex used for thread safety on reading operaition.
        std::mutex writingLock; ///< Mutex used for thread safety on writing operaition.
        Redis* readingClient;   ///< Redis Connection used for reading operation.
        Redis* writingClient;   ///< Redis Connection used for writing operation.
        long long id;           ///< The channel id.
        // Utility Functions
        /**
         * @brief Send a write command to redis.
         * @param command The redis command.
         * @return A Pointer to a Response from redis.
         */
        RedisResponse* sendWriteCommand(std::string command);
        /**
         * @brief Send a read command to redis.
         * @param command The redis command.
         * @return A Pointer to a Response from redis.
         */
        RedisResponse* sendReadCommand(std::string command);
        /**
         * @brief Read a message from Redis with a specific id.
         * @param messageId The complete message id.
         * @return A Pointer to the Message read.
         */
        Message* readMessageWithId(std::string messageId);
};

#endif