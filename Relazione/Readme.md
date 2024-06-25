# to do:
## da aggiungere alla relazione
- [ ] **architettura** di sistema: drone <--> canali redis <--> torre <--> serverpostgre <--> db
- [ ] **Redis**: quando un drone crea un oggetto channel, vengono create due istanze redis: una in **lettura** e una in **scrittura** per evitare latenze di ascolto...
## Activity diagram:
### torre
```
Start:
    inizializzazione
    do:
        paralell(check_dei_droni, message_handler)
    while(Running)
    shotdown_procedure 
Fine

inizializzazione:
    connsessione_server_postgre:
        connesione_db
    connesione_server_redis
    creazione_canali_redis
    suddivisione_area

message_handler:
    wait for a message M:
        
check_dei_droni:
    ogni minuto:
        for all d in DB.drone:
            if last_update(d) >= 2 min and < 5 min:
                ping 
            elif last_update(d) >= 5 min:
                d.state = Dead 
            if d.state = Ready and exist b block(b) and free(block):
                d.block = associa(d)
shotdown_procedure:
    disconnetti i droni 
    pulisci i canali redis 
```
### drone
```
Start:
    inizializzazione
    do:
        paralell(behaviour_loop, message_handler)
    while(Running)
    shotdown_procedure 
Fine


inizializzazione:
    connessione_redis
        creazione canale
    rischiesta associazione
    se dopo un minuto non è stato associato, si spegne


behaviour_loop:
    check_batteria
    switch State s:
        s == Charging:
            se batteria è carica invia infoMessage
            stato = Ready
        s == monitoring  or s == returning:
            moove 
            se posizione == posizione_assegnata:
                invia locationMessage
                stato = waiting
            
check_batteria:
    se (stato == waiting or stato == monitoring) and not batteria_sufficiente:
        
            invia return 
            ritorna alla torre

shotdown_procedure:
    disconnetti i droni 

batteria_sufficiente
    //calcola se la distanza dalla torre è < della distanza percorribile dal drone 
```
## message guarda protocolli comunicazioni.md

```
association: 
    D->T : richiesta di associazione 

    T->D : l'id del drone [è sempre seguito da un infoMessage]
    
    il drone richeide un id temporaneo per comunicare con la torre
ping: 
    controllo se il drone è ancora connesso
    se dopo _n_ ping (5 minuti) il drone non risoponde, il drone è morto
infoMessage: 
    T -> D: la torre richiede le info del drone
    D -> T: le informazioni del drone 
    la torre richiede al drone tutte le sue info
locationMessage: 
    T->D: al torre richiede la nuova posiz



```

## correzioni relazione:
- [ ] Aggiornare gli stati dei droni (vedi globals.hpp)
- [ ] 


## trigger:
 - nessun drone può uscire dal blocco assegnatogli finché è in stato flying [da valutare]