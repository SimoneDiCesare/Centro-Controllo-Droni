#include <iostream>
#include <chrono>
#include <mutex>
#include "channel.hpp"

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



enum DroneState {
    WAITING,
    CHARGING,
    MONITORING
};

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
        // Setter
        void setPosX(int posX);
        void setPosY(int posY);
        void setBatteryAutonomy(std::chrono::seconds batteryAutonomy);
        void setBatteryLife(std::chrono::seconds batteryLife);
        void setState(DroneState state);
        // Getter
        long long getId();
        int getPosX();
        int getPosY();
        std::chrono::seconds getBatteryAutonomy();
        std::chrono::seconds getBatteryLife();
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
        long long id;
        int posX;
        int posY;
        std::chrono::seconds batteryAutonomy;
        std::chrono::seconds batteryLife;
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