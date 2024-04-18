# Database
All'interno del progetto viene sfruttato un  database **PostreSql** per salvare dati utili in maniera persistente all'interno della torre.
## Struttura del Database
Il Database presenta un'unica tabella: **drone**, la quale contiene le informazioni dei vari droni con il seguente schema:

|Nome            |Tipo     |Info                                     |
|:--------------:|:-------:|:----------------------------------------|
|id              |int 64bit|Id > 0 univoco del drone                 |
|x               |integer  |coordinata x del drone all'ultimo update |
|y               |integer  |coordinata y del drone all'ultimo update |
|battery_autonomy|interval |durata della batteria all'ultimo update  |
|battery_life    |interval |durata massima della batteria del drone  |
|dstate          |enum     |stato del drone all'ultimo update        |
|last_update     |timestamp|timestamp dell'ultimo update             |

L'Enum del campo di state è definita in questa maniera:
```sql
CREATE TYPE DSTATE AS ENUM('waiting', 'charging', 'monitoring')
```

>**TODO**:
> - Modificare la tabella coerentemente con il codice e il db
> - Aggiornare la documentazione con le reali tabelle

## Implementazione
Vengono sfruttate la libreria **pqxx** per creare una connessione ed eseguire query con il database postgre.\
L'eseguibile della torre prende come parametro facoltativo un file di properties, con la seguente struttura:
```ini
[Opzione]=Valore
```
Un esempio di file base è il seguente:
```ini
[Ip]=http://mydomain.com/db
[User]=root
```
### Lista DB File Properties 
|Opzione |Tipo   |Info                                        |
|:------:|:-----:|:-------------------------------------------|
|Ip      |String |Ip alla connessione postgre                 |
|Port    |Integer|Porta d'accesso del db (solo in caso di url)|
|User    |String |Username per accedere al db                 |
|Password|String |Password in chiaro per accedere al db       |
|DbName  |String |Nome del db da utilizzare                   |

> **NOTA:**\
> L eproperties di base sono quelle di default di postgre (ip e port), con un utente chiamato **tower** senza password.
___
Una volta lette le properties, la torre crea una connessione con il db (in caso di fail, l'eseguibile si interrompe con un errore).\
Una volta connessa, la torre controllerà l'esistenza della tabella **drone**, e si metterà in attesa di ricevere dei droni per iniziare il controllo dell'area.\
Ogni qualvolta che un drone si connette alla torre, viene salvata la nuova entry del drone del db.
> **ID DRONE:**\
> Il protocollo di comunicazione prevede che il drone si autocrei un proprio id da > mandare alla torre. Di conseguenza, la torre dovrà controllare se l'id autogenerato dal drone sia valido all'interno del db (non preesistente), e in caso dovrà assegnarne uno nuovo.

> **TODO:**
> - La torre necessita di un db vuoto, poicé i droni vengono generati a runtime in questa simulazione. Controllare se si necessità di resettare il db ad ogni simulazione (per ora **SI**)