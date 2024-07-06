#ifndef POSTGRESQL_HPP
#define POSTGRESQL_HPP
#include <pqxx/pqxx>
#include <string>
#include <tuple>
#include <mutex>

#define CURRENT_TIMESTAMP std::string("CAST(EXTRACT(EPOCH FROM CURRENT_TIMESTAMP) AS BIGINT)")

/**
 * @struct PostgreArgs
 * @brief Struct used for storing Postgre connection arguments.
 * 
 */
struct PostgreArgs {
    std::string ip = "127.0.0.1";   ///< The server port.
    int port = 5432;                ///< The server port.
    std::string dbname;             ///< The name of the db used.
    std::string user;               ///< The username for login into the db.
    std::string password;           ///< The password for login into the db.
};

/**
 * @struct PostgreResult
 * @brief Struct used for storing a response from a Postgre Query.
 */
typedef struct PostgreResult {
    bool error;                 ///< Checks if this response contains an error.
    std::string errorMessage;   ///< The error message of this response.
    pqxx::result result;        ///< The result of a query.
} PostgreResult;

/**
 * @class Postgre
 * @brief This class represent a connection to a Postgre db. 
 * 
 * With this class is possible to connect to a Postgre db, and execute query.
 */
class Postgre {
    public:
        /**
         * @brief Costructor of class Postgre.
         * @param args The argument for connecting to a Postgre db.
         */
        Postgre(const PostgreArgs args);
        /**
         * @brief Destructor of class Postgre.
         */
        ~Postgre();
        /**
         * @brief Executes a query into the connected db.
         * @param query The query to execute
         * @return The db response as PostgreResult
         */
        PostgreResult execute(std::string query);
        /**
         * @return True if a connection to a Postgre db is enstablished.
         */
        bool isConnected();
        /**
         * Disconnect to the Postgre db.
         */
        void disconnect();
    private:
        pqxx::connection *conn;         ///< The connection to a db.
        std::mutex transactionMutex;    ///< Mutex used for running transaction on db in safety.
};

#endif  // POSTGRESQL_HPP