#include "wrapper.h"
#include "datastructures.h"
#include "signal.h"

//handler passato alla funzione signal: forza la chiusura tutti i descrittori di socket quando da terminale si digita ctrl+c (SIGINT)
void ctrlC_Handler (int);

//globali per la signal
int listenfd, servVfd, connfd;

int main(int argc, char** argv)
{
    struct sockaddr_in  myservaddr;    //struttura indaddress_connect_initializeririzzo CV usato per l'ascolto
    struct sockaddr_in  serv_v_addr;   //struttura indirizzo server
    char                buf[BUF_SIZE]; //conterrà i dati inviati dal client
    GreenPass           tmpGreenPass;  //struttura temporanea

    int     pid;

    // Sarebbe più opportuno gestire tutto con i messaggi su socket e non con i segnali
     if (signal(SIGINT, ctrlC_Handler) == SIG_ERR)
        printf("Non posso SIGINT\n");

    /* si inizializza il descrittore di socket in ascolto per eventuali connessioni
    inizializza la propria struttura indirizzo su una porta specifica utilizzata dal client
    associa il descrittore di ascolto alla struttura inizializzata al passo precedente
    si mette in ascolto su quel descrittore */
    listenfd = Socket(AF_INET, SOCK_STREAM, 0);
    address_initializer(&myservaddr, CENTRO_VACCINALE_PORT);
    Bind(listenfd, (struct sockaddr *) &myservaddr, sizeof(myservaddr));
    Listen(listenfd, MAX_CLIENT_NUM);
    printf("1 - Centro vaccinale in ascolto su porta %d\n", CENTRO_VACCINALE_PORT);

    /* preparo .. per connettermi al server v
    inizia la struttura indirizzo per il serverV
    richiede connessione al serverV */
    servVfd = Socket(AF_INET, SOCK_STREAM, 0);    
    address_connect_initializer (&serv_v_addr, SERVER_V_IP, SERVER_V_PORT_CV);
    Connect(servVfd, (struct sockaddr *) &serv_v_addr, sizeof(serv_v_addr));

    for(;;)
    {
        // accetta una connessione su listen fd da parte di un client, ed apre un nuovo canale di comunicazione con quel client su connfd
        connfd = Accept(listenfd, (struct sockaddr *) NULL, NULL);

        // legge i dati inviati dal client
        int bytesRead = FullRead(connfd, &buf, BUF_SIZE);

        if (bytesRead == 0)
        {
            // Il client ha chiuso la connessione
            close(connfd);
        }
        else if (bytesRead > 0)
        {
            if ((pid = fork()) == 0)
            {
                // sono il processo figlio e non devo rimanere in ascolto: deve invece comunicare i dati inviati dal client al serverV
                close(listenfd);
            
                // imposta l'ultimo carattere con quello di fine stringa
                buf[bytesRead]=0;

                strcpy(tmpGreenPass.code, buf);
                tmpGreenPass.valid = VALIDITA_GREENPASS;
                tmpGreenPass.t = time(NULL);
                
                //dopo aver inserito le informazioni (passo precedente) ricevuto nella struttura greenpass temporanea, stampa quelle info sul terminale
                printf("2 - Ricevuto nuovo Green pass %s, valido per %d giorni... lo invio a ServerV\n", tmpGreenPass.code, tmpGreenPass.valid);
                //invia questa struttura al server
                FullWrite(servVfd, &tmpGreenPass, sizeof(GreenPass));
                
                //chiude canale di comunicazione col client
                close(connfd);
                exit(0);
            }
            else
            {
                //sono il processo padre
                close(connfd);
            }

        }
    }

    //chiude canale di comunicazione col serverV
    close(servVfd);
    return 0;
}

void ctrlC_Handler (int signo) {
    if (signo == SIGINT) {
        printf("\nRicevuto Ctrl + C\n");
        close(listenfd);
        close(connfd);
        close(servVfd);
        printf("FD socket chiusi con successo\n");
        exit(-1);
    }
    
}



