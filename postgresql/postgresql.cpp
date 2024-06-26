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
    return formattedArgs;
}



Postgre::Postgre(const PostgreArgs args) : transactionMutex() {
    std::string formattedArgs = buildArgs(args);
    this->conn = new pqxx::connection(formattedArgs);
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
        this->conn->close(); // richiede pqxx 7. altrimenti cambia "close" con "disconnect"
    }
}