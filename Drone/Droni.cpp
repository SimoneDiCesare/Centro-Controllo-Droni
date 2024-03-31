#include "ClassDroni.hpp"
#include <iostream>

Droni::Droni(int X, int Y, char16_t stato, int batteria)
{   
    ID = 0;
    MaxPwr = 100;
    Vel = 0; // i droni partono tutti dalla base ?!
    RoA = 6;
    Bat = batteria; //come simulare la batteria che si esauruisce la batteria ? ldopo ogni movimento
    PosX = X;
    PosY = Y;
    Stt = stato;
}

Droni::~Droni()
{
}

void Droni::SetID(int id){
    ID = id;
}

void Droni::Movimento(int X, int Y){
    // aggiungere controllo per non fare uscire i droni dai confini della griglia 
    PosX = X;
    PosY = Y;
}

int Droni::GetBatteria(){
    return Bat;
}

char16_t Droni::GetStato(){
    return Stt;
}

int Droni::GetVelocita(){
    return Vel;
}

int Droni::GetRaggio(){
    return RoA;
}

//chiedere se vale la pena fare una struttura per rappresentare la posizione
int Droni::GetPosX(){
    return PosX;
}

int Droni::GetPosY(){
    return PosY;
}

