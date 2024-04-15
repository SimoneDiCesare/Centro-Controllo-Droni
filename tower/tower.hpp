#ifndef TOWER_HPP
#define TOWER_HPP
#include "channel.hpp"

class Tower {
    public:
        Tower();
        ~Tower();
        void connect(std::string ip, int port);
        void start();
    private:
        Channel* channel;
        void handleMessage(Message* message);
        static bool running;
        static void handleSignal(int signal);
};

#endif