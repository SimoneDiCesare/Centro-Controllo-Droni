# Protocollo Comunicazioni
La Torre di Controllo ed i Droni comunicano in maniera asincrona.
## Struttura generale dei pacchetti
Ogni pacchetto inviato deve seguire la seguente struttura generale:
- **Tipologia**: 1 Byte con offset 0x00 che identifica la tipologia del pacchetto inviato/ricevuto.
- **Dimensione**: 2 Byte (u16) con offset 0x01 che identifica la dimensione dei dati inviati.
- **Dati**: Il vettore dei dati, lungo quanto specificato dalla **dimensione**. I dati contenuti devono seguire le specifiche della tipologia.

## Inizializzazzione
Durante la fase di inizializzazzione, ossia quando un drone deve connettersi per la prima volta ad una torre per essere operativo, vengono sfruttati 2 pacchetti.
### Richiesta Associazione
Il drone invia un pacchetto di tipo **0x01** e con dimensione **0x0000**, e si mette in attesa di una risposta.
### Risposta Associazione
La torre invia un pacchetto di tipo **0x01** in risposta ad una Richiesta d'Associazione di un drone, e può avere due casistiche:
- Dimensione a 0x0000: La torre non accetta l'associazione del drone.
- Dimensione a 0x0004: La torre accetta il drone, e gli restituisce un identificativo u16 con il quale viene registrato.

## Invio Messaggi
Una volta che un drone è associato ad una torre, questo si metterà in attesa di istruzioni da essa.
Il drone può ricevere diversi tipi di istruzioni:
### Richiesta Spostamento
La torre invierà un pacchetto di tipo **0x02** al drone, che conterrà le coordinate da raggiungere, ossia i due valori (x,y) come u16.
### Richiesta Info
La torre invierà un pacchetto di tipo **0x03** al drone, richiedendo in dietro lo stato del drone.
### Risposta Info
Il drone invierà, in risposta al pacchetto **0x03**, anch'esso un pacchetto di tipo **0x03**, con all'interno le seguenti informazioni:
- **ID Drone**: u16
- **Posizione**: (x, y) 2*u16
- **Area Ricoperta**: (x1, y1, x2, y2) 4*u16
- **Batteria**: u8
- **Stato**: u8 (Rappresenta in che stato si trova il drone: (CHARGING,READY,MONITORING)

### Richiesta Rientro
Quando il drone avrà scarsa autonomia rimasta, invierà una richiesta di rientro alla torre con codice **0x04**.
### Risposta Rientro
Se la torre riceverà una richiesta di rientro, invierà al drone un messaggio di conferma con codice **0x04**, e si occuperà di sostituirlo con un nuovo drone con stato READY.

## Tabella Riassuntiva

| Codice | Dati | Info |
|:------:|:-----|:-----|
|0x01|Nessuno, oppure ID Drone|Richiesta e Risposta Associazione Torre-Drone|
|0x02|Coordinate (x,y)|Richiesta Spostamento Drone|
|0x03|Nessuno, Oppure Info del Drone|Richiesta e Risposta Info Drone|
|0x04|Nessuno|Richiesta e Risposta Rientro|

# Implementazione
La Torre di Controllo ed i Droni si scambiano i vari messaggi attraverso Redis.
Redis è configurato per funzionare sulla porta **SCEGLIERE PORTA**, e gestisce delle liste di messaggi associati agli attori del sistema.
Ogni attora ha associato un ID (la torre avrò sempre ID 0, mentre i droni avranno un ID assegnato dalla torre in fase di inizializzazione).
L'ID è usata come chiave Redis, e contiene come contenuto una lista di Messaggi, i quali presentano la struttura descritta in precedenza.
Il funzionamento base è il seguente:
1. L'Attore controlla se associato al suo ID è presente una lista di messaggi non vuota
2. L'Attore legge il primo elemento della lista, rimuovendolo da essa stessa.
3. L'Attore elabora il pacchetto.
4. Se è presente un'altro elemento all'interno della lista, si riparte dal numero 2.
> **NOTA**:
> L'Attore continuerà a svolgere le sue funzioni a prescindere dalla presenza o meno di contenuto all'interno di Redis, a patto che abbia azioni da svolgere.
