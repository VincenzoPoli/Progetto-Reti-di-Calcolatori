//Inizializza la struttura indirizzo passata per riferimento in modo tale da accettare una connessione da qualsiasi ip sulla porta specificata
void address_initializer (struct sockaddr_in* addr, int port){
    addr->sin_family = AF_INET;
    addr->sin_addr.s_addr = htonl(INADDR_ANY);
    addr->sin_port = htons(port);
}

//Inizializza un'altra struttura indirizzo del CV in modo tale da richiedere una connessione al serverV
void address_connect_initializer (struct sockaddr_in* addr, char* ip, int port)
{
    addr->sin_family = AF_INET;
    addr->sin_port   = htons(port);
    
    if (inet_pton(AF_INET, ip, &addr->sin_addr) < 0) {
        exit (1);
    }
}
