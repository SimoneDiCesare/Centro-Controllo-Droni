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
    // std::cout << formattedArgs << "\n";
    return formattedArgs;
}

// TODO: - Add try catch to connection

Postgre::Postgre(const PostgreArgs args) : transactionMutex() {
    std::string formattedArgs = buildArgs(args);
    this->conn = new pqxx::connection(formattedArgs);
    // if (!this->conn->is_open()) {
    //     std::cout << "Can't connect to postgre\n";
    // } else {
    //     std::cout << "Connected to db\n";
    // }
}

Postgre::~Postgre() {
    if (this->isConnected()) {
        this->disconnect();
    }
}

PostgreResult Postgre::execute(std::string query) {
    PostgreResult r;
    this->transactionMutex.lock();
    try {
        pqxx::work txn(*(this->conn));
        r.result = txn.exec(query);
        r.error = false;
        txn.commit();
    } catch (std::exception& e) {
        r.result = pqxx::result();
        r.error = true;
        r.errorMessage = e.what();
    }
    this->transactionMutex.unlock();
    return r;
}

bool Postgre::isConnected() {
    return this->conn->is_open();
}

void Postgre::disconnect() {
    if (this->conn != nullptr) {
#ifdef PQXX_MAJOR_VERSION
    #if PQXX_MAJOR_VERSION < 7
        this->conn.disconnect();
    #else
        this->conn.close();
    #endif
#else
    this->conn->close(); // Use latest version anyway
#endif
    }
}