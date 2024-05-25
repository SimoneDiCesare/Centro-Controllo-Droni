#ifndef DRONE_HPP
#define DRONE_HPP
#include <iostream>
#include <chrono>
#include <mutex>
#include "channel.hpp"
#include "globals.hpp"

//Coordinate XY -> PosX e PosY
//Stato InRicarica / InAzione -> Stt 
//Ricarica massima -> Max Power -> MaxPwr
//Velocita -> Vel
//Batteria -> Bat
//Raggio d'azione -> range of action -> RoA (espressa come raggio ?!)
//ID
//costruttore / distruttore

//[tempo][tipo mersaggio*][chi][a chi(opzionale)][cosa fa]
//*ex mesaggio errore o di info

class Drone {
    public:
        // Constructors and Deconstructors
        Drone();
        Drone(long long id);
        ~Drone();
        // Drone functions
        bool connectChannel(std::string ip, int port);
        bool connectToTower();
        void start();
        void moveTo(int x, int y);
        void handleMessage(Message*);
        void move(double delta);
        void behaviourLoop();
        void checkBattery();
        // Setter
        void setPosX(double posX);
        void setPosY(double posY);
        void setBatteryAutonomy(double batteryAutonomy);
        void setBatteryLife(double batteryLife);
        void setState(DroneState state);
        // Getter
        long long getId();
        double getPosX();
        double getPosY();
        double getBatteryAutonomy();
        double getBatteryLife();
        DroneState getState();
        int getRangeOfAction();
        int getVelocity();
    private:
        // Process params
        Channel *channel;
        bool running;
        long long messageCounter;
        std::mutex messageCounterLock;
        // Drone infos
        std::mutex destinationLock;
        int destX;
        int destY;
        long long id;
        std::mutex positionLock;
        double posX;
        double posY;
        int towerX;
        int towerY;
        double batteryAutonomy;
        double batteryLife;
        long long charge;
        long long rechargeTime;
        std::mutex stateMutex;
        DroneState state;
        int rangeOfAction; // Useful? => Constant can handle this?
        int velocity; // Useful? => Costant for semplicity?
        // Utility functions
        static long long createId();
        void logi(std::string message);
        void loge(std::string message);
        void logd(std::string message);
        long long generateMessageId();
};

#endif // DRONE_HPP