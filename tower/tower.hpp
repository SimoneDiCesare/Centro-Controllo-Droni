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
    long long id;               ///< id of drone.
    int posX;                   ///< X location of drone.
    int posY;                   ///< Y location of drone.
    long long batteryAutonomy;  ///< Battery autonomy of drone.
    long long chargeTime;      ///< Total battery autonomy at 100% of drone.
    DroneState droneState;      ///< State of drone.
    long long lastUpdate;       ///< The last time this drone has sent a message.
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
        Tower(int droneCount, int areaWidth, int areaHeight);
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
         * @brief Creates trigger for the db.
         * @return True if the triggers are created succesfully.
         */
        bool createTrigger();
        /**
         * @brief Connects this Tower to a postgres db.
         * @param args The arguments for the db connection.
         * @return True if the connection is enstablished.
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
         * @brief Function executed each step for checking each drone state.
         */
        void droneCheckLoop();
        /**
         * @brief For simulation purpose: increments the values of the area for calculating a weight of each cells. 
         */
        void areaUpdateLoop();
        /**
         * @brief For simulation purpose: shows up the grid with a color associated from blue to red for how much time has passed from one controll in a cell.
         */
        void drawGrid();
        /**
         * @brief Retrieve drone from db with id.
         * @param id The id to find.
         * @return A Drone with the specified id.
         */
        Drone getDrone(long long id);
        /**
         * @brief Retrieve all connected drones from db.
         * @return A list of all connected drones.
         */
        std::vector<Drone> getDrones();
        /**
         * @brief Loop that checks every minute if the drones are responsive and operative.
         */
        void checkDrones();
        /**
         * @brief Calcolate the next location for a drone.
         * @param drone The drone to be assigned.
         */
        void calcolateDronePath(Drone drone);
        /**
         * @brief Handle a system signal
         * @param signal The signal to handle.
         */
        static void handleSignal(int signal);
        // Handle functions
        /**
         * @brief Handle a drone ping.
         * @param message The ping message.
         */
        void handlePing(PingMessage* message);
        /**
         * @brief Handle a drone association.
         * @param message The association message.
         */
        void handleAssociation(AssociateMessage* message);
        /**
         * @brief Handle a drone info update.
         * @param message The info message.
         */
        void handleInfoMessage(DroneInfoMessage* message);
        /**
         * @brief Handle a location reached by a drone.
         * @param message The location message.
         */
        void handleLocationMessage(LocationMessage* message);
        /**
         * @brief Handle a drone retire update.
         * @param message The retire message.
         */
        void handleRetireMessage(RetireMessage* message);
        /**
         * @brief Handle a drone disconnection.
         * @param message The disconnect message.
         */
        void handleDisconnection(DisconnectMessage* message);
        /**
         * @brief Associate a drone to a new block.
         * @param drone The drone to associate.
         * 
         * This function gets an unassigned block with a max value inside it, and associate it to a drone.
         */
        void associateBlock(Drone drone);
        /**
         * @brief Calculate statistics for simulation purpose.
        */
        void calculateStatistics();
        // Params
        static bool running;            ///< Checks if a tower is online. Is static for hanglind system signals.
        Channel* channel;               ///< The redis channel used for comunications.
        Postgre* db;                    ///< The postgre db used for storing useful data.
        long long messageCounter;       ///< Counter for generating uniques messages ids.
        std::mutex messageCounterLock;  ///< Mutex used for r/w messageCounter in safety.
        Area *area;                     ///< The area monitored by this tower.
        int x;                          ///< The x location on the area of the tower.
        int y;                          ///< The y location on the area of the tower.
        int areaWidth;                  ///< The area width.
        int areaHeight;                 ///< The area height.
        int startTime;                  ///< The start time of simulation.
        std::vector<unsigned long long> avgs;        ///< Simulation Purpose: Al avgg calculated.
};

#endif  // TOWER_HPP