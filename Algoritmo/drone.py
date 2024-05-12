
import numpy as np


class Drone:
    def __init__(self, max_time_of_fly, position, id):
        self.id = id
        self.time_of_fly = max_time_of_fly
        self.position = position
        self.chargetime = 30 + (id**7)%15 #"dalle due alle 3 ore" nel problema originale corrisponde alla seguente:300 + (id**7)%150 il modulo serve a variare
        self.state = "Ready" #STATI: Ready: il drone è carico e fermo alla base, pronto per partire
                             #       Flyng: il drone è in volo, time_of_fly > 0
                             #       Charging: il drone si sta ricaricando,  time_of_fly == 0, position == origin
                             #       Death: il drone si è scaricato in un punto diverso dall'origine. time_of_fly == 0, position != origin
                             #       --- Nella versione con movimenti diagonali si aggiungono:
                             #       Moovin: il drone sta volando verso lo Start del blocco assegnato
                             #       Transferring: il drone si sta spostando da un blocco ad un altro
                             #       Diagonal: il drone si sta muovendo in diagonale, [dalla torre allo start o dal blocco alla torre]
                             #       --- Nella versione senza movimenti diagonali si aggiunge:
                             #       Sarting: il drone si sta muovendo verso la diresione che gli è stata assegnata (tramite go_to(...) per iniziare a scansionare un bloccpo o per far ritonro alla torre)
    def __str__(self):
        return f"id: {self.id},Pos {self.position}, time of fly {self.time_of_fly}, st: {self.state}, cT: {self.chargetime}"
    

    def distance(self, point):
        return abs(point[0] - self.position[0]) + abs(point[1] -self.position[1])
    