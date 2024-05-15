#include "wrapper.h"
#include "datastructures.h"

int main(int argc, char** argv)
{
    int                 sockfd, n;
    char                recvline[1025] ;
    struct sockaddr_in  servaddr;
    char                buf[1024];
    UtenteContagiato    uc;
    int                 bytesRead;
    
    //L'ip del centro vaccinale è definito nel file greenpass.h

    do {

        sockfd = Socket(AF_INET, SOCK_STREAM, 0);
        
        //inizializzazione struttura indirizzo del CV per richiedere una connessione
        address_connect_initializer (&servaddr, SERVER_G_IP, SERVER_G_PORT_CT);

        //tentativo di connessione al ServerG
        Connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    
        printf("Inserire il tuo codice di tessera sanitaria: \n");
        bytesRead = FullReadFromStdIn(buf, 1025);
        if (bytesRead > 0)
        {
            buf[bytesRead] = 0;
            strcpy(uc.codice, buf);

            printf("Inserire [%d] se l'utente è contagiato, [%d] se l'utente è guarito:\n", CONTAGIATO, GUARITO);
            bytesRead = FullReadFromStdIn(buf, 1025);
            if (bytesRead > 0)
            {
                uc.contagiato = atoi(buf);
                if (uc.contagiato == CONTAGIATO)
                {
                    printf("Inserito codice %s: Contagiato: SI\n\n", uc.codice);
                }
                else
                {
                    printf("Inserito codice %s: Contagiato: NO\n\n", uc.codice);
                }
            }
            
            FullWrite(sockfd, &uc, sizeof(uc));
        }

        close(sockfd);
    } while(1);

    return(0);
}

