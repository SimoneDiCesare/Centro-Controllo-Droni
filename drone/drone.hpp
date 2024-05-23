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
        void accelerate(int amount);
        void handleMessage(Message*);
        void movement();
        // Setter
        void setPosX(int posX);
        void setPosY(int posY);
        void setBatteryAutonomy(long long batteryAutonomy);
        void setBatteryLife(long long batteryLife);
        void setState(DroneState state);
        // Getter
        long long getId();
        int getPosX();
        int getPosY();
        long long getBatteryAutonomy();
        long long getBatteryLife();
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
        int destX;
        int destY;
        long long id;
        int posX;
        int posY;
        long long batteryAutonomy;
        long long batteryLife;
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