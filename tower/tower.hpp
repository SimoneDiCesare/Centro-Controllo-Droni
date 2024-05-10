#ifndef TOWER_HPP
#define TOWER_HPP
#include "channel.hpp"
#include "postgresql.hpp"
#include <vector>
#include <mutex>
#include <string>

typedef struct Drone {
    long long id;
    int posX;
    int posY;
    std::chrono::seconds batteryAutonomy;
    std::chrono::seconds batteryLife;
    std::string droneState;
    std::chrono::seconds lastUpdate;
} Drone;

class Tower {
    public:
        Tower();
        ~Tower();
        bool connectChannel(std::string ip, int port);
        bool connectDb(const PostgreArgs args);
        void start();
        bool isRunning();
    private:
        void handleMessage(Message* message);
        long long generateMessageId();
        // Drones Functionalities
        void droneCheckLoop();
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
        // Params
        Channel* channel;
        Postgre* db;
        long long messageCounter;
        std::mutex messageCounterLock;
        // Area
        int areaWidth;
        int areaHeight;
        std::vector<std::vector<int>> area;
};

#endif // TOWER_HPP