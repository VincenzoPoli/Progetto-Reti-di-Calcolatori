#include "wrapper.h"
#include "datastructures.h"
#include "signal.h"

void GestioneDescrittoreCentroVaccinale();
int GetMaxFDP1();

int main(int argc, char** argv)
{
    int                 sockfd, n;
    char                recvline[1025] ;
    struct sockaddr_in  servaddr;
    char                buf[1024];
    char                tesseraSanitaria[CODE_SIZE + 1];
    int                 bytesRead;
    int                 greenPassValid;
    int                 maxfd;
    fd_set              readset, writeset;
    struct timeval      timeout;

    do {
        /* si di chiara un descrittore di socket
        si inizializza una struttura indirizzo SG per richiedere una connessione
        tentativo di connessione al SG */
        sockfd = Socket(AF_INET, SOCK_STREAM, 0);
        address_connect_initializer (&servaddr, SERVER_G_IP, SERVER_G_PORT_CS);
        Connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    
        printf("1 - Inserire il codice di tessera sanitaria: \n");
        int bytesRead = FullReadFromStdIn(buf, 1025);
        if (bytesRead > 0)
        {
            printf("da buffer di tastera ho letto  %d caaratteri \n", bytesRead);

            // Setto carattere di fine stringa
            buf[bytesRead] = '\0';   
            //strcpy(tesseraSanitaria, buf);
            // Sostituisco il fine stringa col carriage return 
            //tesseraSanitaria[bytesRead] = '\n';

            /*

            // Setto carriage return dopo il fine stringa
            tesseraSanitaria[bytesRead + 1] = '\n';

            */

            /* inizializza il readset a tutti 0
            imposta il writeset aggiungendo il descrittore per il SG
            controlla il FD più grande */
            FD_ZERO(&writeset);
            FD_SET(sockfd, &writeset);
            if (sockfd > STDIN_FILENO)
            {
                maxfd = sockfd + 1;
            }
            else
            {
                maxfd = STDIN_FILENO + 1;
            }

            Select(maxfd, NULL, &writeset, NULL, NULL);      
            if (FD_ISSET(sockfd, &writeset)) 
            {
                //Se è pronto il descrittore di socket in scrittura    
                FullWrite(sockfd, buf, bytesRead + 1);
            }

            /* inizializza il readset a tutti 0
            imposta il readset aggiungendo il descrittore per il SG
            controlla il FD più grande */
            FD_ZERO(&readset);
            FD_SET(sockfd, &readset);
            if (sockfd > STDIN_FILENO)
            {
                maxfd = sockfd + 1;
            }
            else
            {
                maxfd = STDIN_FILENO + 1;
            }

            Select(maxfd, &readset, NULL, NULL, NULL);       
            if (FD_ISSET(sockfd, &readset)) 
            {
                //Se è pronto il descrittore di soket di lettura   
                bytesRead = FullRead(sockfd, &greenPassValid, sizeof(greenPassValid));
                if (bytesRead > 0) 
                {
                    printf("2 - GREEN PASS %s\n", StatoGreenPass[greenPassValid]);
                }
            }
        }

        close(sockfd);
    } while(1);

    return 0;
}
