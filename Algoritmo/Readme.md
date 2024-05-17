# Bozza di Readme 

Codice grezzo vagamente ottimizzato ma ancora una bella porcheria.
Lo scopo principale è fungere da pseudo codice per altre applicazioni di controllo di droni.
Per vedere l'esecuzione del codice eseguire il file "control.py".
Il file mostra la mappa utilizzando matplot e salva l'ultima rappresentazione della mappa come png alla voce di "matrice.png"
(**ovviamente se non diversamente commentato**)
c'è una riga finale che calcola i tempi medi di visita e il tempo massimo per le iterazioni imposte all'inizio del loop e stampa il risultato in output.
All'interno del file control c'è un'area segnata con prima delle funzioni in cui si possono impostare i parametri di partenza:
```python
#-----------
#DATI DI PARTENZA
#ogni casella indica un'area quadrata di 20m x 20m 
#per passare da una casella all'altra un drone impiega 2.4 secondi. 
#ogni STEO indica un'iterazione del codice
#in questa iterazione il drone si muove di STEP
#ogni STEP indica 2.4 secondi. 
dim = 10 #per problema originale dim = 300
common_time = 150 #per problema originale dim = 750
Ndrones = 100
origin = (int((dim-1)/2),int((dim-1)/2)) #presumendo che l'area sia QUADRATA
t_iter = 5000
#-----------
```
riga 94-106 del file control.py


        
Ricordati di controllare anche il codice del drone, in cui è riportato il tempo di ricarica:
```python
self.chargetime = 30 + (id**7)%15 #"dalle due alle 3 ore" nel problema originale corrisponde alla seguente:300 + (id**7)%150 il modulo serve a variare
```
riga 10 del file drone.py


## Diagramma di flusso
### Torre:
```
avviata con numero di droni, (per comando virtuale per stop associazione droni)
calcolo dei blocchi, 
associazione droni,
assegnazione blocchi droni.
---CONTOLLI SUL DATABASE
per ogni drone:  
  se lastUpdate < 300 -> Morto
  se Ready -> assegna (x,y) start (blocco)
  se Morto -> disconnetti
  se Charging -> richiedi update   
  se Flying -> last update < 120 -> richiedi update 
  se Diagonal -> lastUpdate < diagTime -> richiedi update
  se Returning -> lastUpdate < diagTime -> richiedi update

---CONTROLLO DEI MESSAGGI  
associazione -> connetti drone; associa id; se Ready 
  -> assegna (x,y) start (blocco)
ping -> aggiorna lastUpdate
infoDrone -> aggiorno drone nel database
location -> aggiorno posDrone; invio nuova posizione
returning -> libera blocco -> cerca sostituto
disconnessione -> libera risorse drone
```


### Droni:
```
avviati in pull di thread
associazione torre
---MOVIMENTO
se time_of_fly <= time_to_return + 30s -> manda "returning"; ritorna
se (x,y) == _destination -> manda "location"
ow vai in _destination; aggiorna _batteria

---CONTROLLO DEI MESSAGGI
associazione -> setta _id
ping -> manda pong
infodrone -> manda _info
location -> aggiorna _destinazione
disconnessione -> libera risorse; rimanda "disconnesso"
```
