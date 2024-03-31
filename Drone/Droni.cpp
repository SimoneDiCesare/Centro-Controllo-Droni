#include "ClassDroni.hpp"
#include <iostream>

Droni::Droni(int X, int Y, char16_t stato, int batteria)
{
    AsseganazioneID();
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

void Droni::AsseganazioneID()
{
    //Prende il tempo corrente e lo trasforma e lo converte nell'id
    auto now = std::chrono::system_clock::now();
    auto now_seconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
    auto epoch = now_seconds.time_since_epoch();
    std::chrono::seconds::rep current_time_seconds = epoch.count();
    ID = current_time_seconds;
}