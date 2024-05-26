#ifndef REDIS_HPP
#define REDIS_HPP
#include <string>
#include <vector>

/**
 * @enum RedisResponseType
 * @brief Enum for defining the possible redis responses.
 * 
 * This enum provides constants to determin which type of content a @ref{RedisResponse} has.
 */
enum RedisResponseType {
    NONE,       ///< No response (timeout?) (default?)
    STRING,     ///< Simple string
    BULK_STRING,///< Bulk String
    ERROR,      ///< Error
    INTEGER,    ///< Integer
    VECTOR,     ///< Bulk String vector (used for lists and hset)
    NLL         ///< Null value
};

/**
 * @class RedisResponse
 * @brief Class for handling a redis response
 * 
 * This class provides a way to interact with data received from a redis command.
 */
class RedisResponse {
    public:
        /**
         * @brief Costructor of RedisResponse class.
         */
        RedisResponse();
        /**
         * @brief Destructor of RedisResponse class.
         */
        ~RedisResponse();
        /**
         * @brief Get the simple/bulk_string response from redis.
         * @return a string containing the simple/bulk_string response content.
         */
        std::string getContent();
        /**
         * @brief Get the vector response from redis.
         * @return a vector containing strings from response content.
         */
        std::vector<std::string> getVectorContent();
        /**
         * @brief Get the error message from redis.
         * @return a string containing the error response content.
         */
        std::string getError();
        /**
         * @brief Get the @ref{RedisResponseType} of the response.
         * @return the type of the response.
         */
        RedisResponseType getType();
        /**
         * @brief Check if the Response contains an error.
         * @return true if the response contains an error. False otherwise.
         */
        bool hasError();
        /**
         * @brief Set the response to an error with the specified error message.
         * @param error The error message.
         */
        void setError(std::string error);
        /**
         * @brief Set the content of the response.
         * @param content The content as string.
         */
        void setContent(std::string content);
        /**
         * @brief Set the type of the response.
         * @param type The response type.
         */
        void setType(RedisResponseType type);
    private:
        std::string content;                    ///< String content of the response
        std::string errorString;                ///< String error of the response
        RedisResponseType type;                 ///< Type of the response
        std::vector<std::string> vectorContent; ///< Contents of the response as string vector
        bool error;                             ///< Checks if the response contains an error
};

/**
 * @class Redis
 * @brief Class for handling a redis client connection.
 *
 * This class provides methods for sending command to a redis server, and handling the responses via the class @ref{RedisResponse}.
 * NOTE: This class is not Thread safe. Its only purpose is to be a wrapper for a redis connection.
 */

class Redis {
    public:
        /**
         * @brief Costructor of Redis class.
         */
        Redis();
        /**
         * @brief Destructor of Redis class.
         */
        ~Redis();
        /**
         * @brief Method for connecting to a Redis Server
         * @param host The server host
         * @param port The server port 
         */
        bool connect(std::string host, int port);
        /**
         * @brief Send a Redis command to server.
         * @param command The redis command.
         * @return The redis response to the command. 
         */
        RedisResponse* sendCommand(std::string command);
        /**
         * @brief Check the status of this Client connection.
         * @return true if the client is connected to a redis server. False otherwise.
         */
        bool isConnected();
        /**
         * @brief Set a reading timeout to the connection socket.
         * @param sec The timeout in seconds.
         * @return true if the timeout was setted. False otherwise.
         */
        bool setTimeout(long sec);
    private:
        bool connected; ///< Checks if the Client is connected to a Redis Server.
        int sockFd;     ///< Socket for writing to/reading from the Redis Server.
};

#endif  // REDIS_HPP
