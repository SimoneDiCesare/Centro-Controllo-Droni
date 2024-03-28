#include <iostream>
#include <chrono>
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

class Droni
{
private:
    int PosX;
    int PosY;
    char16_t Stt;
    int MaxPwr;
    int Vel;
    int Bat;
    int RoA;
    int ID;
    
public:
    Droni(int X, int Y, char16_t stato, int batteria);
    ~Droni();
private:
    void AsseganazioneID();
};

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