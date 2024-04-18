#include "postgresql.hpp"
#include <iostream>
#include <pqxx/pqxx>
#include <string>
#include <tuple>

std::string buildArgs(const PostgreArgs args) {
    std::string formattedArgs = "dbname=";
    if (args.dbname.compare("") == 0) {
        formattedArgs += "towerdb";
    } else {
        formattedArgs += args.dbname;
    }
    formattedArgs += " user=";
    if (args.user.compare("") == 0) {
        formattedArgs += "tower";
    } else {
        formattedArgs += args.user;
    }
    if (args.password.compare("") != 0) {
        formattedArgs += " password=" + args.password;
    }
    formattedArgs += " hostaddr=" + args.ip;
    formattedArgs += " port=" + std::to_string(args.port);
    std::cout << formattedArgs << "\n";
    return formattedArgs;
}

// TODO: - Add try catch to connection

Postgre::Postgre(const PostgreArgs args) {
    std::string formattedArgs = buildArgs(args);
    this->conn = new pqxx::connection(formattedArgs);
    if (!this->conn->is_open()) {
        std::cout << "Can't connect to postgre\n";
    } else {
        std::cout << "Connected to db\n";
    }
}

Postgre::~Postgre() {
    if (this->isConnected()) {
        this->conn->close();
        std::cout << "Disconnected\n";
    }
}

std::tuple<bool, pqxx::result> Postgre::execute(std::string query) {
    try {
        pqxx::work txn(*(this->conn));
        pqxx::result result = txn.exec(query);
        txn.commit();
        return std::make_tuple(true, result);
    } catch (std::exception& e) {
        std::cout << "ERROR: " << e.what() << "\n";
        return std::make_tuple(false,pqxx::result());
    }
}

bool Postgre::truncateTable(std::string tableName) {
    auto [success, _] = this->execute("TRUNCATE TABLE " + tableName);
    return success;
}

bool Postgre::isConnected() {
    return this->conn->is_open();
}