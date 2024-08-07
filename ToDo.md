# to do:
## nella relazione
- [x] Aggiungere monitor 
- [x] Aggiungere risultati sperimentali 
- [x] modificare parte criterio di assegnamento
- [x] database

## Nel simulatore python
- [x] modificare divisioni blocchi python 
- [x] simulazioni test 

## Nel resto del progetto
- [x] capire se i monitor sono giusti -> Non credo sia na cosa fattibile
- [ ] scrivere a runtime tempo di visita medio e tempo di visita massimo 
- [x] scrivere spostamenti dei droni in un log dedicato
- [ ] pulire la repository da file vecchi
- [ ] Scrivere un bel [README](README.md)

## domande di Armando 
- [x] perché ci sono due makefile? uno in [makefile1](libpqxx-7.7.5/build/include/Makefile) e [makefile2](makefile)\
    Il makefile1 credo sia robaccia che si è portato dietro Pandro dal suo progetto, ma non serve. Il vero make è il secondo che hai detto
- [x] [Database.md](Database.md) è aggiornato? Se è aggiornato lo aggiungo alla relazione.\
      Il file è aggiornato si, può essere implementato.


# Appunti Simone
## PREMESSA
Di definizioni, così come di ingegneria del software in generale, non ne sò nulla, quindi quando parli di requisiti o simili vado ad intuito sulle correzioni, ma non è detto che abbiano senso.\
In più mi scuso se alcune correzioni che ti faccio in realtà sono per colpa mia che non ho tenuto aggiornato alcuni documenti ahahaha.
## Correzioni
- [x] La relazione, quando la apro, mi dà come titolo (quello visto dalla barra del brower/quello visto in alto a destra nella previsualizzazione delle pagine) "Overleaf Example", non so se puoi cambiarla
- [x] Nella Figura 1 è improprio dire "canale redis lettura/scrittura", poiché sono dei socket redis quelli lettura/scrittura, mentre il canale è sempre e solo uno per attore.
- [x] Al punto 2.1 credo sia improprio mettere i blocchi dentro questa sezione, poiché non fanno parte dei requisiti utente, bensì sono un oggetto sfruttato dalla torre per gestire l'area.\
  Mentre per i droni credo vadano tolti i parametri di ID e di stato, poiché sono sfruttati dal sistema ma non sono cose che il sistema deve sapere a priori.
- [x] Al punto 2.1, riga "punto di partenza (o ultimo punto visitato)"\
  In realtà è più corretto dire "punto di partenza e ultimo punto visitato", poiché il sistema tiene queste due cose distinte (startX/Y e lastX/Y).
- [x] Al punto 2.2, requisito 2.1.2 la cambierei così:\
  "Un drone pu`o volare [...] e dalla sua posizione attuale a quella della torre __più un margine di errore__.".
- [x] Al punto 2.4, sottopunto 4\
  La dimensione di una cella non è più 20x20, ma 14x14.
- [x] La descrizione della Figura 4 mi viene troncata, non so se è colpa di Chrome o boh.
- [x] Figura 4:\
  Nella Torre, dopo la dissociazione di un drone dal blocco, è più opportuno mandare la freccia a "esiste un blocco disponibile?", perché non è detto che il drone venga assegnato ad un blocco.\
  La Torre non si spegne in automatico se tutti i droni sono morti, ma unicamente se viene spenta dall'esterno.\
  Nel Drone, nell'if "posizione successiva ricevuta" ti manca l'else, che dovrebbe tornare ad "aspetta la prossima posizione".\
  Nel drone, una volta che torna alla torre perché scarico non è che muore e basta, ma si mette in ricarica e poi riparte, non so se deve essere visualizzato in un chart del genere o meno.
- [x] Figura 5:\
  Manca l'else allo spegnimento della torre.
- [x] Sezione 2.4.2, sotto __Divisione dei Blocchi__, hai scritto "matrice di case quadrate", penso intendessi celle quadrate.
- [x] Sezione 3.1, alla riga: "Il contenuto del messaggio di ottiene".\
  Credo che sia sbagliata una formulazione di questo tipo, perché il messaggio non si ottiene inserendo l'HashMap, bensì leggendo l'HashMap. Credo tu intendessi dire una cosa simile:\
  "Il contenuto del messaggio è salvato in una HashMap con chiave l'id descritto in precedenza".
- [x] Sezione 3.2, alla riga: "I messaggi vengono letti e scritti dalla coda utilizzando ”LPUSH c:id[...]".\
  Credo sia più corretto dire: "I messaggi vengono aggiunti alla coda utilizzando ”LPUSH c:id message_id” e salvati con ”HSET message_id {params}”.
  In fase di lettura invece, viene utilizzato il metodo ”RPOP c:id” per estrarre il message_id, e poi ”HGETALL message_id” per recuperarne il contenuto".
- [x] Sezione 3.2.\
  Credo sia opportuno aggiungere una frase prima della descrizione delle due funzioni di lettura e scrittura dei messaggi, una cosa del tipo: "Per fare ciò, vengono implementate le seguenti funzioni all'interno della classe".
- [x] Sezione 3.2.\
  Quando descrivi la signature della funzione awaitMessage devi apportare un po' di modifiche:
  1. Il parametro timeout di default non è più -1, ma 0.
  2. Renderei questa descrizione simile a quella dei comando sendMessage, aggiungendo uno specchietto come quello sopra che contiene questo:
     // Letture dell'id da redis -> restituisce un id sula variabile message_id
     BRPOP c:{this.id} timeout
     \
     // Lettura contenuto del messaggio
     HGETALL message_id
  3. Direi anche che se il timeout è uguale a 0, allora la funzione diventa bloccante.
- [x] Sezione 3.2.1, riga: "Si è deciso di dividere le operazioni tra due mutex[...]".\
  Non è per questione di efficenza, anche perché sinceramente non so se è più efficiente o meno, bensì è per questione di responsività. In questo modo un attore può leggere senza che la fase di scrittura sia di intralcio, e viceversa.
- [x] Sezione 3.3.\
  Dividerei la sezione in cui parli delle funzioni dal quella dei costruttori, facendo due subsection __Costruttori__ e __Funzioni__.
- [x] Sezione 3.3.1.\
  Quando parli della funzione parseReesponse, la lista puntata è errata.
  L'unica cosa che fa la funzione è prendere una risposta redis che DEVE essere di tipo VECTOR, e leggerne i parametri a coppie. Per esempio, se si leggesse un messaggio LocationMessage, la sua funzione di parsing legge all'interno della riposta la chiave x, e setta la sua x a quella nella risposta.
- [x] Sezione 3.3.1.\
  L'esempio sotto la funzione parseMessage contiene un nome errato, poiché non esiste nessun messaggio che si chiama PostionMessage, ma la funzione che cerchi è quella di LocationMessage ed è questa qui:
  ```C++
  std::string LocationMessage::parseMessage() {
    std::string data = "type " + std::to_string(this->getType());
    data = data + " x " + std::to_string(this->x);
    data = data + " y " + std::to_string(this->y);
    data = data + " movement_type " + std::to_string(this->movementType);
    return data;
  }
  ```
- [x] Mi sembra manchi la parte in cui espliciti che nel db non è presente la tabella dei Blocchi perché tanto viene gestita dalla torre a livello di codice C.
