#ifndef LOG_HPP
#define LOG_HPP
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <mutex>

/**
 * A Thread-safe implementation of a logger.
 * It uses a static stream for sharing a log file in a process.
 */

static bool verbose = false;    ///< If the logs should be printed also on the console.
static bool debugging = false;  ///< If it should be log also the debug messages.
static std::ofstream logfile;   ///< The stream usef for writing to the log file.
static std::mutex logMutex;     ///< Mutex for stream safety.

/**
 * @brief Formats the current timestamp in the format [dd-MM-YYY HH:mm:ss]
 * @return Current Timestamp as text
 */
std::string getCurrentTimeFormatted();

/**
 * @brief Logs a message into the logfile.
 * @param who The actor writing the message.
 * @param what The type of the message.
 * @param message The message to write.
 */
void log(std::string who, std::string what, std::string message);

/**
 * @brief Logs an info message into the logfile.
 * @param who The actor writing the message.
 * @param message The message to write.
 * 
 * This function calls @see{log(std::string, std::string, std::string)} with the second paramtere as "INFO".
 */
void logInfo(std::string who, std::string message);

/**
 * @brief Logs an error message into the logfile.
 * @param who The actor writing the message.
 * @param message The message to write.
 * 
 * This function calls @see{log(std::string, std::string, std::string)} with the second paramtere as "ERROR".
 */
void logError(std::string who, std::string message);

/**
 * @brief Logs a debug message into the logfile.
 * @param who The actor writing the message.
 * @param message The message to write.
 * 
 * This function calls @see{log(std::string, std::string, std::string)} with the second paramtere as "DEBUG".
 */
void logDebug(std::string who, std::string message);

/**
 * @brief Logs a warning message into the logfile.
 * @param who The actor writing the message.
 * @param message The message to write.
 * 
 * This function calls @see{log(std::string, std::string, std::string)} with the second paramtere as "WARNING".
 */
void logWarning(std::string who, std::string message);

/**
 * @brief Close the logfile stream.
 */
void logClose();

/**
 * @brief Opens a stream for logging messages.
 * @param file The file to open.
 */
void logOpen(std::string file);

/**
 * @brief Sets it this log should be verbose or not.
 * @param b The new verbose value.
 */
void logVerbose(bool b);

/**
 * @brief Sets if this log should print debug messages.
 * @param b The new debugging value.
 */
void logDebugging(bool b);

#endif  // LOG_HPP