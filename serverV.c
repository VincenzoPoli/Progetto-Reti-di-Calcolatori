#include "wrapper.h"
#include "datastructures.h"
#include "signal.h"

int VerificaValiditaGreenPass(char *codiceTessera);
int GreenPassValido(GreenPass* gp, char *codiceTessera);
int LeggiGreenPass(GreenPass *gp, int indice);
void SalvaGreenPass(GreenPass*);
void GestioneDescrittoreServerG();
void GestioneDescrittoreCentroVaccinale();
int GetMaxFDP1();

void ctrlC_Handler (int);

//globali per la signal
int listenfd_cv,    listenfd_sg;
int connfd_cv,      connfd_sg;


int main(int argc, char** argv)
{
    struct sockaddr_in  servaddr_cv;
    struct sockaddr_in  servaddr_sg;
    
    int                 pid;
    int                 maxfd;
    fd_set              readset;

    // Sarebbe più opportuno gestire tutto con i messaggi su socket e non con i segnali

     if (signal(SIGINT, ctrlC_Handler) == SIG_ERR)
        printf("Non posso SIGINT\n");

    /* si inizializza il descrittore di socket in ascolto per eventuali connessioni
    inizializza la propria struttura indirizzo su una specifica porta che verrà utilizzata da CV
    associa il descrittore di ascolto alla struttura inizializzata al passo precedente    
    si mette in ascolto su quel descrittore */

    listenfd_cv = Socket(AF_INET, SOCK_STREAM, 0);
    address_initializer(&servaddr_cv, SERVER_V_PORT_CV);
    Bind(listenfd_cv, (struct sockaddr *) &servaddr_cv, sizeof(servaddr_cv));
    Listen(listenfd_cv, MAX_CLIENT_NUM);
    printf("1 - Server in ascolto su porta %d\n", SERVER_V_PORT_CV);

    // si ripetono le operazioni precedenti, ma per il server G

    listenfd_sg = Socket(AF_INET, SOCK_STREAM, 0);
    address_initializer(&servaddr_sg, SERVER_V_PORT_SG);
    Bind(listenfd_sg, (struct sockaddr *) &servaddr_sg, sizeof(servaddr_sg));
    Listen(listenfd_sg, MAX_CLIENT_NUM);
    printf("2 - Server in ascolto su porta %d\n", SERVER_V_PORT_SG);

    maxfd = GetMaxFDP1();

    for(int i = 1;;i++)
    {
        /* inizializza il readset a tutti 0
            imposta il readset aggiungendo il descrittore per il CV
            imposta il readset aggiungendo il descrittore per il serverG
            funzione max per ricever il FD più grande */
        FD_ZERO(&readset);
        FD_SET(listenfd_cv, &readset);
        FD_SET(listenfd_sg, &readset);
        
        if (Select(maxfd, &readset, NULL, NULL, NULL) > 0)
        {
            if (FD_ISSET(listenfd_cv, &readset))
            {
                printf("E' arrivata una richiesta dal CV\n");

                /* se ci si è sbloccati per una richiesta dal CV
                si accetta la connessione e si apre canale di comunicazione col CV */
                connfd_cv = Accept(listenfd_cv, (struct sockaddr *) NULL, NULL);
                
                /* il figlio gestirà il servizio per il CV
                il figlio non deve rimanere in ascolto, deve invece scrivere su file i dati ricevuti dal CV 
                il figlio chiude il canale di comunicazione col CV una volta esaurita la sua funzione */
                if ((pid = fork()) == 0)
                {
                    // sono il processo figlio

                    close(listenfd_cv);

                    // funzione che gestisce il servizio precedentemente citato
                    GestioneDescrittoreCentroVaccinale();

                    close(connfd_cv);
                    exit(0);
                }
                else
                {
                    // sono il processo padre, io non devo fare nulla, se non rimanere in ascolto
                    close(connfd_cv);
                }
            }
            
            /* se ci si è sbloccati per una richiesta dal SG
            si accetta la connessione e si apre canale di comunicazione col serverG
            il figlio non deve rimanere in ascolto, deve invece occuparsi del servzio relativo al serverG
            il figlio deve chiude canale di comunicazione col SG una volta esaurito il proprio compito */
            if (FD_ISSET(listenfd_sg, &readset))
            {
                printf("E' arrivata una richiesta dal SG\n");
                connfd_sg = Accept(listenfd_sg, (struct sockaddr *) NULL, NULL);

                if ((pid = fork()) == 0)
                {
                    // sono il processo figlio

                    close(listenfd_sg);

                    //funzione che gestisce il servizio precedentemente citato
                    GestioneDescrittoreServerG();

                    close(connfd_sg);
                    exit(0);
                }
                else
                {
                    // sono il processo padre, io non devo fare nulla, se non rimanere in ascolto
                    close(connfd_sg);
                }
            }
        }

        
    }
    //chiude canale di comunicazione col CV
    close(listenfd_cv);
    //chiude canale di comunicazione col serverG
    close(listenfd_sg);

    return 0;
}

void GestioneDescrittoreServerG()
{
    char        buf[BUF_SIZE];
    fd_set      writeset;
    int         maxfd;

    //legge dal SG
    int bytesRead = FullRead(connfd_sg, &buf, sizeof(buf));
    if (bytesRead == 0)
    {
        printf("\nIl server G ha chiuso la connessione...\n");
    }
    else if (bytesRead > 0)
    {
        printf("\nRicevuti %d byte...\n", bytesRead);

        //ora in "buf" abbiamo il codice della tessera sanitaria su cui effettuare il check della validità del green pass
        int gpValid = VerificaValiditaGreenPass(buf);
        printf("Green pass valido: %d\n", gpValid);
        
        /* inizializza il writeset a tutti 0
        imposta il writeset aggiungendo il descrittore per il SG
        setta il FD più grande */
        FD_ZERO(&writeset);
        FD_SET(connfd_sg, &writeset);
        maxfd = connfd_sg + 1;

        if (Select(maxfd, NULL, &writeset, NULL, NULL) > 0)
        {
            //SG pronto a ricevere dati
            if (FD_ISSET(connfd_sg, &writeset))
            {
                FullWrite(connfd_sg, &gpValid, sizeof(gpValid));
            }
        }
    }

}

void GestioneDescrittoreCentroVaccinale()
{
    char        buf[BUF_SIZE];
    GreenPass*  tmpGreenPass;
    int         pid;
 
    int connok = 1;
    while (connok == 1)
    {
        //legge dal CV
        int bytesRead = FullRead(connfd_cv, &buf, sizeof(GreenPass));
        if (bytesRead == 0)
        {
            //Il client ha chiuso la connessione
            printf("\nIl centro vaccinale ha chiuso la connessione...\n");
            connok = 0;
            
        }
        else if (bytesRead > 0)
        {
            printf("\nRicevuti %d byte...\n", bytesRead);
            tmpGreenPass = (GreenPass*)buf;

            //stampa a video info ricevute dal CV
            printf("\nRicevuto nuovo Green pass %s, valido per %d giorni...\n", tmpGreenPass->code, tmpGreenPass->valid);
            //ulteriore funziona che implementa nei fatti il servizio relativo al CV
            ScriviFile("GreenPass.bin", (GreenPass*)tmpGreenPass, sizeof(GreenPass));
        }
    }
    


}

//1 se green pass è valido, 0 se no
int VerificaValiditaGreenPass(char *codiceTessera)
{
    int i = 0;
    GreenPass gp;
    int gpValid;
    
    do
    {
        printf("Lettura numero %d\n", i);
        if (LeggiGreenPass(&gp, i) == -1)
        {
            return 0;
        }
        gpValid = GreenPassValido(&gp, codiceTessera);
        switch (gpValid)
        {
            case 0:
                return 1;
                break;
            case 1:
                //vado avanti col ciclo
                break;
            case 2:
                return 0;
                break;
        }
        i++;
    } while (1 == 1);
}

//restituisce 0 se valido, 1 se dobbiamo continuare la ricerca, 2 se green pass è scaduto
int GreenPassValido(GreenPass* gp, char *codiceTessera)
{
    if (strcmp(codiceTessera, gp->code) != 0)
    {
        //Se non è l'utente che stiamo cercando, dico che non è valido, quindi la ricerca deve continuare
        printf("return 1\n");
        return 1;
    }

    time_t now = time(NULL);

    //86400 = secondi in un giorno
    //t+valid*86400 è la data di scadenza espressa in secondi
    if ((gp->t + (gp->valid * 86400)) < now)
    {
        //Se la data della vaccinazione + la durata del green pass SUPERA la data e ora attuale, allora il green pass è ancora valido
        printf("return 2\n");
        return 2;
    }

    printf("return 0\n");
    return 0;
}

//restitisce 0 se letto con successo, -1 se errore
int LeggiGreenPass(GreenPass *gp, int indice)
{
    gp->code[0] = 0;
    gp->valid = 0;

    if (LeggiFile("GreenPass.bin", (GreenPass*)gp, sizeof(GreenPass) * indice, sizeof(GreenPass), SEEK_SET) == -1)
    {
        printf("No dati da leggere\n");
        return -1;
    }

    printf("Letto Green pass.. Codice tessera: %s\n", gp->code);

    return 0;
}



int GetMaxFDP1()
{
    if (listenfd_cv > listenfd_sg)
    {
        return listenfd_cv + 1;
    }
    else
    {
        return listenfd_sg + 1;
    }
}

void ctrlC_Handler (int signo) {
    if (signo == SIGINT) {
        printf("\nRicevuto Ctrl + C\n");

        close(listenfd_cv);
        close(connfd_cv);

        close(listenfd_sg);
        close(connfd_sg);

        printf("FD socket chiusi con successo\n");
        exit(-1);
    }
    
}




