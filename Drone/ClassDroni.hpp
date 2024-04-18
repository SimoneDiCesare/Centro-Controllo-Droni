#include <iostream>
#include "channel.hpp"

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
    //Set
    void SetID(int id);
    void SetStt(char16_t Stato); 

    void Movimento(int X, int Y);
    void Start();

    //Get
    char16_t GetStato();
    int GetBatteria();
    int GetVelocita();
    int GetRaggio();
    int GetPosX ();
    int GetPosY ();
    // Accelerazione serve anche per fermare il Drone 
    void Accelerazione(int vel);
private:
    Channel* channel;
};
