#ifndef REDIS_HPP
#define REDIS_HPP
#include <string>
#include <vector>
#include <mutex>

/* Comandi Utili:
 * HSET id k1 v1 k2 v2 k3 v3 ... => :n (Numero di campi inseriti)
 * HGET id k => $n\nv (numero di caratteri[a capo]stringa)
 * LPUSH id v1 v2 v3 ... => :n (numero di valori inseriti)
 * RPOP id => $n\nv (numero di caratteri[a capo]stringa)
 * SET id v => +OK
 * GET id => $n\nv (numero di caratteri[a capo]stringa)
 * I messaggi hanno dei dati, un id ed un destinatario
 * HSET m:s_id:m_id type t k1 v1 k2 v2 ...
 * s_id è l'id del sender (torre avrà sempre id 0)
 * m_id è l'id messaggio a seconda di s_id, che sarà autoincrementale
 * Canali di comunicazione
 * LPUSH c:id s_id:m_id s_id:m:id ...
 * Lista che inserisce in coda e estrae in testa con RPOP c:id
 * Contiene s_id:m_id, che servirà poi per estrarre il messaggio effettivo
 * HGET m:s_id:m_id type
 * Una volta ottenuto il tipo, si potranno estrarre i dati del messaggio
*/

/**
 * Response Type: 0 = nil, 1 = int, 2 = string, 3 = list
*/

enum RedisResponseType {
    NONE,
    STRING,
    ERROR,
    INTEGER,
    VECTOR,
    NLL
};

class RedisResponse {
    public:
        RedisResponse();
        ~RedisResponse();
        std::string getContent();
        std::vector<std::string> getVectorContent();
        std::string getError();
        RedisResponseType getType();
        bool hasError();
        void setError(std::string);
        void setContent(std::string);
        void setType(RedisResponseType);
    private:
        std::string content;
        std::string errorString;
        RedisResponseType type;
        std::vector<std::string> vectorContent;
        bool error;
};

class Redis {
    public:
        Redis();
        ~Redis();
        bool connect(std::string host, int port);
        RedisResponse* sendCommand(std::string command);
        bool isConnected();
        bool setTimeout(long sec);
    private:
        bool connected;
        int sockFd;
        std::mutex fdMutex;
};

#endif
