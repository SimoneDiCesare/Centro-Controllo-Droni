# Centro-Controllo-Droni
Un progetto universitario che simula il funzionamento di un centro di controllo del centro di controllo per una formazione di droni che sorveglia una data area.

# Compilare il Progetto
Si può buildare l'interezza degli eseguibili tramite:\
`make`\
Il quale genera i seguenti eseguibili nella cartella bin:
- **tower**: L'eseguibile del centro di controllo.\
            Come parametri prende il numero di droni, la dimensione dell'area (larghezza e altezza) e un fattore di tolleranza per il controllo celle.
- **drone**: L'eseguibile di un singolo drone.
- **log_monitor**: Il monitor per il controllo automatico dei log di vari errori.
- **tester**: Un eseguibile che permette di simulare il funzionamento di torre e droni.
- **tower_tester**: Un eseguibile che permette di simulare il funzionamento della torre.
- **drone_tester**: Un eseguibile che permette di simulare il funzionamento di uno sciame di droni.
- **tower_gui**: Un eseguibile alternativo della torre con una gui implementata
> **NOTA**
> 1. La gui della torre è sperimentale, e potrebbe rallentare di molto la torre ed 
> inficiare le sue prestazioni.
> 2. Il tower_tester e il drone_tester non hanno valore singolarmente, poiché una torre senza droni non ha nulla da controllare, ed un drone senza la torre non sa cosa fare.

### Nel dettaglio
Le uniche librerie esterne utilizzate sono:
- **libpqxx 7.9**: Libreria per connettersi ed interfacciarsi con un database postgresql.
- **raylib 5.0**: Libreria utilizzata per li visualizzazione in tempo reale dell'area.

# La Simulazione
Avviando il tester è possibile simulare per 5' una torre di controllo che si interfaccia con i droni per il controllo di una area.\
Questa simulazioe, essendo pensata per girare su un'unica macchina, presenta parametri scalati di simulazione (area da sorveliare, quantità di droni, ecc...).\
Al termine della simulazione viene avviato automaticamente il log_monitor sui log prodotti dalla torre e dai droni, così da controllare e riportare eventuali errori difficili da scovare ad occhio nudo.

# All'interno della Repository
In questa repository si trovano 3 cartelle principali:
- **src**: Contiene il codice sorgente di tutto il progetto, monitor e tester compresi.
- **Algoritmo**: Contiene im simulatore python che emula il comportamento del sistema per mostrare in maniera rapida e chiara come e quali comandi i droni ricevono dalla torre, così da ricavarne (con margine di errore) dati sperimentali.
- **Relazione**: Contiene il file **relazione.pdf**, ossia l'intera descrizione del progetto, e i vari file che permettono di ricostruire tramite LaTeX la relazione stessa.