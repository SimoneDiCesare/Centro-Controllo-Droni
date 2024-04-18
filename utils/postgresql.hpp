#ifndef POSTGRESQL_HPP
#define POSTGRESQL_HPP
#include <pqxx/pqxx>
#include <string>
#include <tuple>

struct PostgreArgs {
    std::string ip = "127.0.0.1";
    int port = 5432;
    std::string dbname;
    std::string user;
    std::string password;
};

class Postgre {
    public:
        Postgre(const PostgreArgs args);
        ~Postgre();
        std::tuple<bool, pqxx::result> execute(std::string query);
        bool truncateTable(std::string tableName);
        bool isConnected();
    private:
        pqxx::connection *conn;
};

#endif