#include "ClassDroni.hpp"
#include <iostream>

Droni::Droni(int X, int Y, char16_t stato, int batteria)
{   
    ID = 0;
    MaxPwr = 100;
    Vel = 0; // i droni partono tutti dalla base ?!
    RoA = 6;
    Bat = batteria;
    PosX = X;
    PosY = Y;
    Stt = stato;
}

Droni::~Droni()
{
}

