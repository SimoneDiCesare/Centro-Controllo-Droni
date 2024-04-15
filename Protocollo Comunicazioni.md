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
### Coda dei Messaggi (Canali)
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
> 1. type 2 => Il tipo del messaggio (un messaggio di update sulla posizione)
> 2. x 10 => La posizione x del drone
> 3. y 20 => La posizione y del drone
## La Classe Channel
Per gestire agevolmente la ricezione e l'invio dei messaggio è stata implementata la classe Channel, la quale gestisce un canale di un attore e presenta due metodi principali:
#### void sendMessageTo(int channelId, Message& message)
Il metodo in questione esegue sul channelId (l'id canale del destinatario) il comando:
```
hset m:{this.id}:{message.id} {message.parseMessage()}
```

E poi crea l'hash del messaggio:
```
hset m:{this.id}:{message.id} {message.parseMessage()}
```
#### Message* awaitMessage(long timeout = -1)
Questo metodo serve per aspettare l'arrivo di un messaggio all'interno di un canale, per poi leggerlo e ritornarlo.  
Il parametro **timeout**, se diverso da -1, imposta un tempo massimo per l'attesa di un messaggio.

## La classe Message
La classe Message è una classe rappresentante un messaggio da inviare/ricevuto.
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
|PingMessage|0|nessuno|Messaggio di Ping|
|AssociateMessage|1|drone_id:int|Messagio di associazione drone<->torre|
|DroneInfoMessage|2|{params}|Scambio parametri drone->torre|
|LocationMessage|3|x:int,y:int|Nuova posizione per drone dalla torre|

</table>

> **DA AGGIUNGERE:**
> - Messaggio di Rientro: Drone con batteria scarica deve rientrate
> - **Verificare Altro**