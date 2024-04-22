#ifndef TOWER_HPP
#define TOWER_HPP
#include "channel.hpp"
#include "postgresql.hpp"
#include <mutex>

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
        static bool running;
        static void handleSignal(int signal);
        long long generateMessageId();
        // Handle functions
        void handlePing(PingMessage*);
        void handleAssociation(AssociateMessage*);
        void handleInfoMessage(DroneInfoMessage*);
        // Params
        Channel* channel;
        Postgre* db;
        long long messageCounter;
        std::mutex messageCounterLock;
};

#endif