#ifndef DRONE_HPP
#define DRONE_HPP

#include "channel.hpp"
#include "globals.hpp"
#include <iostream>
#include <chrono>
#include <mutex>

/**
 * @class Drone
 * @brief The Drone class. An actor of the system. It's purpose is to monitor an area
 * 
 * This Class represents a Drone, with its ability to move, handle requests from a tower and monitor an area.
 */
class Drone {
    public:
        /**
         * @brief Costructor of class Drone.
         */
        Drone();
        /**
         * @brief Costructor of class Drone.
         * @param id The id associated to this drone.
         */
        Drone(long long id);
        /**
         * @brief Destructor of class Drone.
         */
        ~Drone();
        /**
         * @brief Connects this drone to a redis channel.
         * @param ip The server ip.
         * @param port The server port.
         */
        bool connectChannel(std::string ip, int port);
        /**
         * @brief Connects this drone to a Control Tower.
         */
        bool connectToTower();
        /**
         * @brief Starts this drone's functionalities.
         * @param executionSpeed The speed factor for simulation.
         *          A factor grather than 1 means the drone moves, charges and disharge faster.
         *          A factor less than 1 means the drone moves, charges and discharges slower.
         */
        void start(float executionSpeed);
        /**
         * @brief Set a destination for the drone.
         * @param x The x location.
         * @param y The y location.
         */
        void moveTo(int x, int y);
        /**
         * @brief Handles a variety of messages received from a tower.
         * @param message The pointer to the received message.
         */
        void handleMessage(Message*);
        /**
         * @brief Move this drone using a delta time for calculating the meters moved.
         * @param delta The time passed from last movement.
         */
        void move(double delta);
        /**
         * @brief A loop function that handles the behaviour of the drone based on its state.
         */
        void behaviourLoop();
        /**
         * @brief Checks if the battery if low or zero, and act consequently.
         */
        void checkBattery();
        /**
         * @brief Set the x location in safety.
         * @see {positionLock}
         * @param posX The new x location.
         */
        void setPosX(double posX);
        /**
         * @brief Set the y location in safety.
         * @see {positionLock}
         * @param posX The new y location.
         */
        void setExecutionSpeed (float eS);
        /**
         * @brief Set the ExecutionSpeed.
         * @see {positionLock}
         * @param eS is Execution Speed.
         */
        void setPosY(double posY);
        /**
         * @brief Set the battery autonomy of this drone.
         * @param batteryAutonomy The new autonomy.
         */
        void setBatteryAutonomy(double batteryAutonomy);
        /**
         * @brief Set the battery life of this drone.
         * @param batteryLife The new battery life.
         */
        void setBatteryLife(double batteryLife);
        /**
         * @brief Set the state of this drone in safety.
         * @see {stateLock}
         * @param state The new battery life.
         */
        void setState(DroneState state);
        /**
         * @return The id of this drone.
         */
        long long getId();
        /**
         * NOTE: This function is thread safe. @see {locationLock}
         * @return The x location of this drone.
         */
        double getPosX();
        /**
         * NOTE: This function is thread safe. @see {locationLock}
         * @return The y location of this drone.
         */
        double getPosY();
        /**
         * @return The autonomy of this drone.
         */
        double getBatteryAutonomy();
        /**
         * @return The batterylife of this drone.
         */
        double getBatteryLife();
        /**
         * @return The chargeTime of this drone.
         */
        long long getRechargeTime();
        /**
         * NOTE: This function is thread safe. @see {locationLock}
         * @return The state of this drone.
         */
        DroneState getState();
        /**
         * @return The constant velocity of this drone.
         */
        int getVelocity();
    private:
        // Params
        bool running;                   ///< Checks if this drone is up.
        Channel *channel;               ///< The redis channel used for comunications.
        std::mutex messageCounterLock;  ///< Mutex for generating messages ids in safety.
        long long messageCounter;       ///< Counter for messages ids.
        std::mutex destinationLock;     ///< Mutex for r/w drone destination in safety.
        int destX;                      ///< X destination of drone.
        int destY;                      ///< Y destination of drone.
        long long id;                   ///< id of drone.
        std::mutex positionLock;        ///< Mutex for r/w drone position in safety.
        double posX;                    ///< X location of drone.
        double posY;                    ///< Y location of drone.
        int towerX;                     ///< Assocciated Tower X location.
        int towerY;                     ///< Assocciated Tower Y location.
        double batteryAutonomy;         ///< Battery autonomy of drone.
        double batteryLife;             ///< Total battery autonomy at 100% of drone.
        long long charge;               ///< Used when in charging. Counts the time passed in charging.
        long long rechargeTime;         ///< Total recharge time needed from drone to be ready.
        std::mutex stateMutex;          ///< Mutex for r/w drone state in safety. 
        DroneState state;               ///< State of drone.
        int velocity;                   ///< Velocity of drone.
        float executionSpeed;   	    ///< Execution Speed of simulation.
        // Utility functions
        /**
         * @brief Generate a random id for a new drone using the current timestamp.
         * @return A Pseudo-Unique id
         */
        static long long createId();
        /**
         * @brief Logs an info message for this drone.
         * @param message The message to log.
         */
        void logi(std::string message);
        /**
         * @brief Logs an error message for this drone.
         * @param message The message to log.
         */
        void loge(std::string message);
        /**
         * @brief Logs a debug message for this drone.
         * @param message The message to log.
         */
        void logd(std::string message);
        /**
         * @brief Logs a warning message for this drone.
         * @param message The message to log.
         */
        void logw(std::string message);
        /**
         * @brief Generates the next message id, incrementing the @ref{messageCounter} of this drone.
         * @return A pseudo-unique message id. 
         */
        long long generateMessageId();
};

#endif // DRONE_HPP