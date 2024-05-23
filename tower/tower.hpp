#ifndef TOWER_HPP
#define TOWER_HPP
#include "channel.hpp"
#include "postgresql.hpp"
#include "area.hpp"
#include "globals.hpp"
#include <vector>
#include <mutex>
#include <string>

/**
 * @struct Drone
 * @brief Struct for associating the db entries to a c struct
 * 
 * This struct is used by the tower to retrieve and store db data
 */
typedef struct Drone {
    long long id;
    int posX;
    int posY;
    std::chrono::seconds batteryAutonomy;
    std::chrono::seconds batteryLife;
    DroneState droneState;
    std::chrono::seconds lastUpdate;
} Drone;

/**
 * @class Tower
 * @brief The Tower class, the actor that handles and coordinates drones.
 * 
 * This Class is used for handling drones into a grid
 */
class Tower {
    public:
        /**
         * @brief Constructor of Tower class.
         */
        Tower();
        /**
         * @brief Destructor of Tower class.
         */
        ~Tower();
        /**
         * @brief Connects this Tower to a redis channel.
         * @param ip The server ip
         * @param port The server port
         */
        bool connectChannel(std::string ip, int port);
        /**
         * @brief Connects this Tower to a postgres db.
         * @param args The arguments for the db connection. 
         */
        bool connectDb(const PostgreArgs args);
        /**
         * @brief Starts the tower process.
         */
        void start();
        /**
         * @return True if the tower is up and running. Flase otherwise.
         */
        bool isRunning();
    private:
        /**
         * @brief Handles a redis message
         * @param message The message received
         * 
         * This function handles the message read from the redis channel.
         */
        void handleMessage(Message* message);
        /**
         * @brief Generate a unique id from messages
         * @return A message id
         * 
         * This function generates sequencial ids for redis messages.
         */
        long long generateMessageId();
        /**
         * @brief TODO
         */
        void droneCheckLoop();
        /**
         * @brief Retrieve drone from db with id.
         * @param id The id to find.
         * @return A Drone with the specified id.
         */
        Drone getDrone(long long id);
        std::vector<Drone> getDrones();
        void checkDrones();
        void calcolateDronePath(Drone);
        static bool running;
        static void handleSignal(int signal);
        // Handle functions
        void handlePing(PingMessage*);
        void handleAssociation(AssociateMessage*);
        void handleInfoMessage(DroneInfoMessage*);
        void handleLocationMessage(LocationMessage*);
        void handleRetireMessage(RetireMessage*);
        void handleDisconnection(DisconnectMessage*);
        // Params
        Channel* channel;
        Postgre* db;
        long long messageCounter;
        std::mutex messageCounterLock;
        // Area
        Area *area;
        int x;
        int y;
        int areaWidth;
        int areaHeight;
};

#endif // TOWER_HPP