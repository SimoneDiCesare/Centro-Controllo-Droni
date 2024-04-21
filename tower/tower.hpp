#ifndef TOWER_HPP
#define TOWER_HPP
#include "channel.hpp"
#include "postgresql.hpp"

class Tower {
    public:
        Tower();
        ~Tower();
        bool connect(std::string ip, int port);
        bool connectDb(const PostgreArgs args);
        void start();
        bool isRunning();
    private:
        void handleMessage(Message* message);
        static bool running;
        static void handleSignal(int signal);
        long long generateMessageId();
        // Handle functions
        void handlePing(PingMessage *message);
        void handleAssociation(AssociateMessage*);
        // Params
        Channel* channel;
        Postgre* db;
};

#endif