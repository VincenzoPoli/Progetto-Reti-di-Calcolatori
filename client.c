#include "wrapper.h"
#include "datastructures.h"

int main(int argc, char** argv)
{
    int                 sockfd, n;
    char                recvline[1025] ;
    struct sockaddr_in  servaddr;
    char                buf[1024];
    char                tesseraSanitaria[CODE_SIZE + 1];
    
    //L'ip del centro vaccinale è definito nel file greenpass.h

    do {

        sockfd = Socket(AF_INET, SOCK_STREAM, 0);
        
        //inizializzazione struttura indirizzo del CV per richiedere una connessione
        address_connect_initializer (&servaddr, CENTRO_VACCINALE_IP, CENTRO_VACCINALE_PORT);

        //tentativo di connessione al CV
        Connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
    
        printf("Inserire il codice di tessera sanitaria: \n");
        int bytesRead = FullReadFromStdIn(buf, 1025);
        if (bytesRead > 0)
        {
            buf[bytesRead] = 0;
            strcpy(tesseraSanitaria, buf);

            FullWrite(sockfd, &tesseraSanitaria, bytesRead);
        }

        close(sockfd);
    } while(1);

    return(0);
}

