#include "log.hpp"
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <mutex>

std::string getCurrentTimeFormatted() {
    // Get current time
    std::time_t now = std::time(nullptr);
    std::tm* timeinfo = std::localtime(&now);

    // Format time according to [dd:mm:yyyy-hh:mm:ss] format
    std::ostringstream oss;
    oss << "[" << std::setfill('0') << std::setw(2) << timeinfo->tm_mday << "-"
        << std::setfill('0') << std::setw(2) << 1 + timeinfo->tm_mon << "-"
        << 1900 + timeinfo->tm_year << " "
        << std::setfill('0') << std::setw(2) << timeinfo->tm_hour << ":"
        << std::setfill('0') << std::setw(2) << timeinfo->tm_min << ":"
        << std::setfill('0') << std::setw(2) << timeinfo->tm_sec << "]";

    return oss.str();
}

void log(std::string who, std::string what, std::string message) {
    std::lock_guard<std::mutex> lock(logMutex);
    std::string info = getCurrentTimeFormatted() + "[" + who + "][" + what + "] " + message;
    if (logfile.is_open()) {
        logfile << info << std::endl;
    }
    if (verbose) {
        std::cout << info << std::endl;
    }
}

void logInfo(std::string who, std::string message) {
    log(who, "INFO", message);
}

void logError(std::string who, std::string message) {
    log(who, "ERROR", message);
}

void logDebug(std::string who, std::string message) {
    if (debugging) {
        log(who, "DEBUG", message);
    }
}

void logWarning(std::string who, std::string message) {
    log(who, "WARNING", message);
}

void logClose() {
    std::lock_guard<std::mutex> lock(logMutex);
    logfile.close();
}

void logVerbose(bool b) {
    verbose = b;
}

void logDebugging(bool b) {
    debugging = b;
}

void logOpen(std::string file) {
    std::lock_guard<std::mutex> lock(logMutex);
    if (logfile.is_open()) {
        std::cout << "Closing Previous Log\n";
        logClose();
    }
    logfile.open(file, std::ios_base::app);
    if (!logfile.is_open()) {
        std::cerr << "Can't open log file: " << file << std::endl;
    }
}