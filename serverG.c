#include "wrapper.h"
#include "datastructures.h"
#include "signal.h"

//handler passato alla funzione signal: forza la chiusura tutti i descrittori di socket quando da terminale si digita ctrl+c (SIGINT)
void ctrlC_Handler (int);

int VerificaValiditaGreenPassFromServerV(char *codiceTessera);
int LeggiUtenteContagiato(UtenteContagiato *uc, int indice);
int VerificaContagio(char *codiceTessera);
void GestioneDescrittoreClientT();
void GestioneDescrittoreClientS();
int GetMaxFDP1();

//globali per la signal
int listenfd_ct, listenfd_cs, servVfd;
int connfd_ct,   connfd_cs;

int main(int argc, char** argv)
{
    struct sockaddr_in  myservaddr;    //struttura indirizzo CV usato per l'ascolto
    char                buf[BUF_SIZE]; //conterrà i dati inviati dal client
    GreenPass           tmpGreenPass;  //struttura temporanea

    int                 pid;
    int                 maxfd;
    fd_set              readset;

    //Sarebbe più opportuno gestire tutto con i messaggi su socket e non con i segnali
    if (signal(SIGINT, ctrlC_Handler) == SIG_ERR)
        printf("Non posso SIGINT\n");

    //*** ServerG x client S ***
    /* si inizializza il descrittore di socket in ascolto per eventuali connessioni
    inizializza la propria struttura indirizzo su una porta specifica utilizzata dal client
    associa il descrittore di ascolto alla struttura inizializzata al passo precedente
    si mette in ascolto su quel descrittore */
    listenfd_ct = Socket(AF_INET, SOCK_STREAM, 0);
    address_initializer(&myservaddr, SERVER_G_PORT_CT);
    Bind(listenfd_ct, (struct sockaddr *) &myservaddr, sizeof(myservaddr));
    Listen(listenfd_ct, MAX_CLIENT_NUM);
    printf("1 - Server G in ascolto su porta %d\n", SERVER_G_PORT_CT);


    //*** ServerG x client T ***
    /* si inizializza il descrittore di socket in ascolto per eventuali connessioni
    inizializza la propria struttura indirizzo su una porta speCSfica utilizzata dal client
    associa il descrittore di ascolto alla struttura inizializzata al passo precedente
    si mette in ascolto su quel descrittore */
    listenfd_cs = Socket(AF_INET, SOCK_STREAM, 0);
    address_initializer(&myservaddr, SERVER_G_PORT_CS);
    Bind(listenfd_cs, (struct sockaddr *) &myservaddr, sizeof(myservaddr));
    Listen(listenfd_cs, MAX_CLIENT_NUM);
    printf("2 - Server G in ascolto su porta %d\n", SERVER_G_PORT_CS);

    
    maxfd = GetMaxFDP1();

    for(;;)
    {
        /* inizializza il readset a tutti 0
            imposta il readset aggiungendo il descrittore per il ClientT
            imposta il readset aggiungendo il descrittore per il ClientS
            funzione max per ricevere il FD più grande */
        FD_ZERO(&readset);
        FD_SET(listenfd_ct, &readset);
        FD_SET(listenfd_cs, &readset);

        if (Select(maxfd, &readset, NULL, NULL, NULL) > 0)
        {
            if (FD_ISSET(listenfd_ct, &readset))
            {
                /* se CS si è sbloccati per una richiesta dal ClientT
                il clientT è in grado di validare / invalidare un green pass a seconda se CS comunica che l utente è contagiato o meno
                si accetta la connessione e si apre canale di comunicazione col CT 
                il figlio gestirà il servizio per il CT */
                connfd_ct = Accept(listenfd_ct, (struct sockaddr *) NULL, NULL);
                if ((pid = fork()) == 0)
                {
                    /* sono il processo figlio
                    il figlio non deve rimanere in ascolto, deve invece scrivere su file i dati ricevuti dal CT */
                    close(listenfd_ct);
                    close(listenfd_cs);

                    // funzione che gestisce il servizio precedentemente CStato
                    GestioneDescrittoreClientT();

                    // chiude canale di comunicazione col CT
                    close(connfd_ct);
                    exit(0);
                }
                else
                {
                    //sono il processo padre, io non devo fare nulla, se non rimanere in ascolto
                    close(connfd_ct);
                }
            }
            if (FD_ISSET(listenfd_cs, &readset))
            {               
                /* se CS si è sbloccati per una richiesta dal ClientS
                il ClientT è in grado di validare / invalidare un green pass a seconda se CS comunica che l utente è contagiato o meno
                si accetta la connessione e si apre canale di comunicazione col CS 
                il figlio gestirà il servizio per il CS */
                connfd_cs = Accept(listenfd_cs, (struct sockaddr *) NULL, NULL);
                if ((pid = fork()) == 0)
                {
                    /* sono il processo figlio
                    il figlio non deve rimanere in ascolto, deve invece scrivere su file i dati ricevuti dal CS */
                    close(listenfd_ct);
                    close(listenfd_cs);

                    // funzione che gestisce il servizio precedentemente citato
                    GestioneDescrittoreClientS();

                    // chiude canale di comunicazione col CS
                    close(connfd_cs);
                    exit(0);
                }
                else
                {
                    // sono il processo padre, io non devo fare nulla, se non rimanere in ascolto
                    close(connfd_cs);
                }
            }
        }

    }

    // chiude canale di comunicazione col serverV
    close(servVfd);
    return 0;
}

int VerificaValiditaGreenPassFromServerV(char *codiceTessera)
{
    int                 maxfd;
    fd_set              readset, writeset;
    int                 valid;
    struct sockaddr_in  serv_v_addr;   //struttura indirizzo server

    /* si dichiara un descrittore di socket
    si inizia la struttura indirizzo per il serverV 
    si richiede connessione al serverV */
    servVfd = Socket(AF_INET, SOCK_STREAM, 0);    
    address_connect_initializer (&serv_v_addr, SERVER_V_IP, SERVER_V_PORT_SG);
    Connect(servVfd, (struct sockaddr *) &serv_v_addr, sizeof(serv_v_addr));

    printf("Connesso al server V\n");

    FullWrite(servVfd, codiceTessera, strlen(codiceTessera) + 1);
    printf("\nIl codice della tessera: %s, è stata inviata al serverV. Numero caratteri inviati: %ld\n", codiceTessera, strlen(codiceTessera) + 1);

    // imposta il readset aggiungendo il descrittore per il ServerV
    FD_ZERO(&readset);
    FD_SET(servVfd, &readset);
    maxfd = servVfd + 1;

    if (Select(maxfd, &readset, NULL, NULL, NULL) > 0)
    {
        //se ci si è sbloccati per una richiesta dal ServerV
        if (FD_ISSET(servVfd, &readset))
        {
            FullRead(servVfd, &valid, sizeof(valid));
            printf("IL server V ha detto che %s ha un green pass %s\n", codiceTessera, StatoGreenPass[valid]);
        }
    }

    close(servVfd);
    return valid;  
}

int LeggiUtenteContagiato(UtenteContagiato *uc, int indice)
{
    uc->codice[0] = 0;
    uc->contagiato = NONE;

    if (LeggiFile("UtentiContagiati.bin", (UtenteContagiato*)uc, (-1) * sizeof(UtenteContagiato) * (indice + 1), sizeof(UtenteContagiato), SEEK_END) == -1)
    {
        printf("No dati da leggere\n");
        return -1;
    }

    //printf("Letto Utente %s .. Codice tessera: %s\n", StatoContagio[uc->contagiato], uc->codice);

    return 0;
}

void GestioneDescrittoreClientT()
{
    char                buf[BUF_SIZE];
    UtenteContagiato*   tmpUtenteContagiato;
    
    int connok = 1;
    while (connok == 1)
    {
        //legge dal CT
        int bytesRead = FullRead(connfd_ct, &buf, sizeof(UtenteContagiato));
        if (bytesRead == 0)
        {
            //Il client ha chiuso la connessione
            printf("\nIl client T ha chiuso la connessione...\n");
            connok = 0;
            
        }
        else if (bytesRead > 0)
        {
            printf("\nRicevuti %d byte...\n", bytesRead);
            tmpUtenteContagiato = (UtenteContagiato*)buf;

            //stampa a video info ricevute dal CT
            printf("Ricevuto nuovo utente %s: codice tessera %s...\n", StatoContagio[tmpUtenteContagiato->contagiato], tmpUtenteContagiato->codice);
            //ulteriore funziona che implementa nei fatti il servizio relativo al CV
            ScriviFile("UtentiContagiati.bin", (UtenteContagiato*)tmpUtenteContagiato, sizeof(UtenteContagiato));
        }
    }
}


void GestioneDescrittoreClientS()
{
    char                buf[CODE_SIZE];
    UtenteContagiato*   tmpUtenteContagiato;
    int                 tmpContagiato;
    int                 statoGreenPass;

    int connok = 1;
    while (connok == 1)
    {
        // legge dal CS
        int bytesRead = FullRead(connfd_cs, &buf, sizeof(buf));
        if (bytesRead == 0)
        {
            //Il client ha chiuso la connessione
            printf("\nIl client S ha chiuso la connessione...\n");
            connok = 0;
            
        }
        else if (bytesRead > 0)
        {
            printf("\nRicevuti %d byte... codice tessera %s...\n", bytesRead, buf);

            //Verifica in locale sul file UtentiContagiati.bin
            tmpContagiato = VerificaContagio(buf);
            
            if (tmpContagiato == CONTAGIATO)
            {   
                // sicuro non ha green pass
                statoGreenPass = NON_VALIDO;
            }
            else
            {
                // dobbiamo contattare il server V per vedere se 
                if (VerificaValiditaGreenPassFromServerV(buf) == 1)
                {
                    statoGreenPass = VALIDO;
                }
                else
                {
                    statoGreenPass = NON_VALIDO;
                }                
            }

            FullWrite(connfd_cs, &statoGreenPass, sizeof(statoGreenPass));
        }
    }
}


//CONTAGIATO se l'utente è contagiato, GUARITO se l'utente è guarito, NONE se il codiceTessera non è presente nel file
int VerificaContagio(char *codiceTessera)
{
    int i = 0;
    UtenteContagiato uc;
    
    do
    {
        if (LeggiUtenteContagiato(&uc, i) == -1)
        {
            //questo NONE significa che il codiceTessera non è presente nel file (abbiamo processato l'intero file)
            return NONE;
        }

        if (strcmp(codiceTessera, uc.codice) != 0)
        {
            //Se il codice tessera dell'utente che abbiamo appena letto non coinCSde con quello passato in parametro d'ingresso
            //vado avanti col CSclo
        }
        else
        {
            return uc.contagiato;
        }

        i++;
    } while (1 == 1);
}

int GetMaxFDP1()
{
    int max = listenfd_ct;
    if (listenfd_cs > max)
    {
          max = listenfd_cs;
    }

    return (max + 1);
}

void ctrlC_Handler (int signo) {
    if (signo == SIGINT) {
        printf("\nRicevuto Ctrl + C\n");
        close(listenfd_ct);
        close(listenfd_cs);
        close(connfd_ct);
        close(connfd_cs);
        close(servVfd);
        printf("FD socket chiusi con successo\n");
        exit(-1);
    }
    
}



