<style>

table, th, td {

  border: 1px solid black;

  border-collapse: collapse;

}

</style>

# Protocollo Comunicazioni
La Torre di Controllo ed i Droni comunicano in maniera asincrona.
Le comunicazioni avvengono tramite Redis.
## Scambio di messaggi
### Canali (Coda dei Messaggi)
Ogni attore ha riservato un canale, il quale è salvato su redis come una lista con chiave = "c:**id**"
> **Channel ID:**  
> L'**id** presente nella chiave del canale è 0 se si tratta della torre,  
> mentre è l'id del drone in caso il canale sia del drone.

Questo canale viene utilizzato in lettura dal proprietario (es: il canale c:0 verrà letto dalla torre), ed in scrittura da chi deve mandare messaggi.  
La lista viene trattata come una coda, utilizzando **"LPUSH c:id message_id"** per inserire un messaggio, ed **"RPOP c:id"**  per leggere un messaggio.
> **Message ID:**  
> Il **message_id** presenta la seguente struttura: **m:id:mid**  
> dove **id** è l'id del canale del mittente, e **mid** è l'id univoco del messaggio 
> rispetto all'id del canale.
### Leggere Messaggi
Una volta estratto il message_id dalla coda del canale, il messaggio è salvato all'interno di redis come un Hash (lista di chiavi-valori)
> **Esempio Creazione Messaggio**
> hset m:32:10 type 2 x 10 y 20  
> questo comando crea il messaggio 10 del drone 32 con i seguenti campi:  
> 1. type 2 => Il tipo del messaggio
> 2. x 10 => La posizione x del drone
> 3. y 20 => La posizione y del drone
## La Classe Channel
Per gestire agevolmente la ricezione e l'invio dei messaggio è stata implementata la classe Channel, la quale gestisce un canale di un attore.
Per inviare e ricevere messaggi, la classe channel presenta due metodi:
#### void sendMessageTo(int channelId, Message& message)
Questo metodo permette di aggiungere alla lista di messaggi di **channelId** il messaggio **message** tramite i due comandi:
```
hset m:{this.id}:{message.id} {message.parseMessage()} // Creazione del messaggio su redis
lpush c:{channelId} m:{this.id}:{message.id} // Aggiunta del messaggio al canale
```
#### Message* awaitMessage(long timeout = -1)
Questo metodo serve per aspettare l'arrivo di un messaggio all'interno di un canale, per poi leggerlo e ritornarlo.  
Il parametro **timeout**, se diverso da -1, imposta un tempo massimo per l'attesa di un messaggio.
### Thread Safety
La Classe channel è thread safe, poiché gestisce le due funzioni **sendMessageTo** e **awaitMessage** tramite due mutex:  
- Un mutex in scrittura, che viene utilizzato per fare il lock durante la creazione e scrittura di un messaggio su un canale
- Un mutex in lettura, che viene lockato in attesa di una risposta da redis quando si tenta di legggere un messaggio dal canale.  

Si è deciso di dividere le operazioni tra due mutex per questione di efficienza, così che mentre un attore è in attesa di un messaggio, possa continuare a fare altre operazioni in scrittura per aggiornare il suo stato agli occhi degli altri attori (nel nostro caso, torna utile per ridurre i tempi di attesa di risposta tra la torre ed i droni)
## La classe Message
La classe Message è una classe rappresentante un messaggio di un canale.
### Costruttori
I costruttori della classe Message sono due:
#### Message(std::string id)
Usato nella creazione di un messaggio già presente.  
Prende come parametro una stringa del formato **id:mid**, nel quale **id** è l'id del canale che ha inviato il messaggio, e **mid** è l'id del messaggio.
#### Message(int id)
Usato nella creazione di un messaggio da creare ed inviare.  
Prende come parametro un intero che rappresenta l'id del messaggio.
___
In aggiunta, questa classe presenta due metodi virtuali che deve implementare ogni tipo di messaggio:
#### void parseResponse(RedisResponse* response)
Questo metodo serve a ricevere una Risposta da un Comando Redis, e tramutarla in messaggio.
Dietro le quinte, quello che viene fatto è:
1. Inviare un comando a redis del tipo **hgetall n:id:mid**
2. Controllare la validità della risposta
3. Creare un messaggio tramite il costruttore **Message(std::string id)**
4. Parsare la risposta ottenuta precedentemente tramite **message.parseResponse(response)**
#### std::string parseMessage()
Questo metodo serve a trasformare i dati contenuti nel messaggio in una stringa della forma:  
**"type type_value p1 v1 p2 v2 p3 v3"**  
Nella quale type_value è un intero che rappresenta il tipo del messaggio, mentre pX kX sono le coppie (parametro, valore) contenute nel messaggio
> **ESEMPIO:**
> La classe PositionMessage, che serve per mandare la posizione di un drone alla torre, presenterà il corpo della funzione parseMessage() di questo tipo:
> ```c++
> return "type 2 x " + std::to_string(this->x) + " y " + std::to_string(this->y);
> ```
___
### Lista Messaggi
<table>

|Classe|Tipo|Parametri|Info|
|:---|:---|:---|:--|
|AssociateMessage|0|drone_id:long long,x:int,y:int|Messagio di associazione drone<->torre|
|PingMessage|1|nessuno|Messaggio di Ping|
|DroneInfoMessage|2|{params}|Scambio parametri drone->torre|
|LocationMessage|3|x:int,y:int,movement_type:int|Nuova posizione per drone dalla torre|
|RetireMessage|4|nessuno|Drone con batteria scarica. Rientro necessario|
|DisconnectMessage|5|nessuno|Disconnette l'associazione drone<->torre|

</table>

#### AssociateMessage
Messaggio inviato dal drone alla torre per richiedere l'associazione al pool di droni.
La torre risponde, ritornando l'id che associerà al drone per riconoscimento all'interno della formazione.\
Il messaggio presenta il campo **drone_id**, che rappresenta l'id temporaneo de drone in richiesta, oppure l'id associato dalla torre in caso di associazione effettuata.\
Inoltre, il messaggio contiene la posizione della torre per gestire con precisione il punto di partenza dei droni.
#### PingMessage
Messaggio inviato dalla torre al drone per verificarne l'esistenza.
Viene generalmente utilizzato quando la torre non riceve update dal drone per più di 2 minuti.\
Questo messaggio non presenta parametri.
#### DroneInfoMessage
Messagio per aggiornare le info di un drone all'interno del db della torre.
Viene generalmente mandato dalla torre al drone, per richiedere un update dello stato del drone e verificarne il funzionamento.\
I parametri del messaggio sono i campi del drone:
- **drone_id**
- **x**
- **y**
- **drone_state**
- **charge_time**
- **battery_autonomy**
#### LocationMessage
Messaggio con duplice scopo:
- Se inviato dalla torre al drone, significa che la torre sta impostando delle nuove coordinate da raggiungere al drone. Dunque, in lettura il drone aggiornerà le coordinate e deciderà cosa fare di conseguenza.
- Se inviato dal drone alla torre, rappresenta un update della posizione del drone, il quale ha completato il movimento assegnatogli in precedenza.

Il messaggio presenta i seguenti parametri:
- **x**: La posizione sull'asse x
- **y**: La posizione sull'asse y
- **type**: Il tipo di movimento da effettuare. Distingue se il drone ha bisogno di effettuare movimenti rapidi, per giungere ad una casella di controllo, oppure movimenti lenti per monitorare la zona.
#### RetireMessage
Messaggio inviato dal drone per notificare la necessità di rientro per low battery.\
In questo caso, la torre invia al drone un LocationMessage con le coordinate precise della torre, e lo mette in stato **retiring** per ottimizzare il percorso di rientro.
#### DisconnectMessage
Messaggio che rimuove l'associazione tra la torre ed il drone. Permette di liberare risorse su redis e sul database.