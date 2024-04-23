#ifndef POSTGRESQL_HPP
#define POSTGRESQL_HPP
#include <pqxx/pqxx>
#include <string>
#include <tuple>
#include <mutex>

struct PostgreArgs {
    std::string ip = "127.0.0.1";
    int port = 5432;
    std::string dbname;
    std::string user;
    std::string password;
};

typedef struct PostgreResult {
    bool error;
    std::string errorMessage;
    pqxx::result result;
} PostgreResult;

class Postgre {
    public:
        Postgre(const PostgreArgs args);
        ~Postgre();
        PostgreResult execute(std::string query);
        bool isConnected();
    private:
        pqxx::connection *conn;
        std::mutex transactionMutex;
};

#endif