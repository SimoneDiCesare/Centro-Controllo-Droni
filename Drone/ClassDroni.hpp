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

enum droniState{WAITING, CHARGING, MONITORING}; 

class Droni
{
private:
    int PosX;
    int PosY;
    droniState Stt;
    int MaxPwr;
    int Vel;
    int Bat;
    int RoA;
    int ID;
    
public:
    Droni();
    ~Droni();

    //Set
    int SetID(int id);
    void SetStt(droniState Stato); 

    void Movimento(int X, int Y);
    void Start();
    void connect(std::string ip, int port);
    void handleMessage(Message* message);
    void handleSignal(int signal);

    //Get
    droniState GetStato();
    int GetBatteria();
    int GetVelocita();
    int GetRaggio();
    int GetPosX ();
    int GetPosY ();

    // Accelerazione serve anche per fermare il Drone ?
    void Accelerazione(int vel);
private:
    Channel* channel;
    bool running;
    int timeID();
};
