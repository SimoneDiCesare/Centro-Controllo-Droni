#ifndef TOWER_HPP
#define TOWER_HPP
#include "channel.hpp"

class Tower {
    public:
        Tower();
        ~Tower();
        void connect(std::string ip, int port);
        void start();
        bool isRunning();
    private:
        void handleMessage(Message* message);
        static bool running;
        static void handleSignal(int signal);
        Channel* channel;
        int messageCount;
        // Handle functions
        void handleAssociation(AssociateMessage*);
};

#endif