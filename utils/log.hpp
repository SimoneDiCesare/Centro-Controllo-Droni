#ifndef LOG_HPP
#define LOG_HPP
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <mutex>

static bool verbose = false;
static std::ofstream logfile;
static std::mutex logMutex;

std::string getCurrentTimeFormatted();

void log(std::string who, std::string what, std::string message);

void logInfo(std::string who, std::string message);

void logError(std::string who, std::string message);

void logDebug(std::string who, std::string message);

void logClose();

void logOpen(std::string file);

void logVerbose(bool b);
#endif