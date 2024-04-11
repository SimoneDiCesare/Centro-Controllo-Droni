#ifndef TOWER_HPP
#define TOWER_HPP
#include "channel.hpp"

class Tower {
    public:
        Tower();
        ~Tower();
        void start();
    private:
        Channel* channel;
};

#endif