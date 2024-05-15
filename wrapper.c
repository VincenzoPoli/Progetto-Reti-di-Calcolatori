int Socket(int family, int type, int protocol)
{
    if (DEBUG_START_END) printf("Socket START\n");

    int sockfd = socket(family, type, protocol);
    if (sockfd < 0) 
    {
        perror("socket");
        exit (1);
    }

    if (DEBUG_START_END) printf("Socket END\n");
    return sockfd;
}


void Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    if (DEBUG_START_END) printf("Connect START\n");

    int result = connect(sockfd, (struct sockaddr *)addr, addrlen);
    if (result < 0) 
    {
        perror("connect");
        exit(1);
    }

    if (DEBUG_START_END) printf("Connect END\n");
}

void Bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    if (DEBUG_START_END) printf("Bind START\n");

	if (bind(sockfd, addr, addrlen) < 0)
	{
   		perror("bind");
   		exit(1);
	}

    if (DEBUG_START_END) printf("Bind END\n");
}

int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    if (DEBUG_START_END) printf("Accept START\n");

	int connfd;
	if((connfd = accept(sockfd,addr,addrlen)) < 0)
	{
   		perror("accept");
   		exit(1);
   	}

    if (DEBUG_START_END) printf("Accept END\n");
    return connfd;
}

void Listen(int sockfd, int maxClient)
{
    if (DEBUG_START_END) printf("Listen START\n");

    if(listen(sockfd, maxClient) < 0)
	{
        perror("listen");
        exit(1);
  	}
    
    if (DEBUG_START_END) printf("Listen END\n");
}

int Select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset, struct timeval *timeout)
{
    if (DEBUG_START_END) printf("Select START\n");

    int result = select(maxfdp1, readset, writeset, exceptset, timeout);
    if (result < 0)
    {
        perror("select");
        exit(1);
    }

    if (DEBUG_START_END) printf("Select END\n");
    return result;
}


ssize_t FullRead(ssize_t fd, void *buf, size_t count) 
{ 
    if (DEBUG_START_END) printf("FullRead START\n");
    size_t nleft; 
    ssize_t nread; 
    nleft = count; 
    ssize_t readBytes = 0;

    while (nleft > 0) // repeat until no left
    {
        nread = read(fd, buf, nleft);
        readBytes += nread;
        
        if (DEBUG_READ)
        {
            printf("FullRead: bytes letti %ld... max caratteri da leggere %ld\n", nread, count);
        }

        if (nread < 0) 
        { 
            if (errno == EINTR) // if interrupted by system call
            { 
                continue; //repeat the loop
            } 
            else 
            { 
                exit(nread); // otherwise exit
            }
        } 
        else if (nread == 0) // EOF
        {
            break; // break loop here 
        }
        else // se nread è > 0
        {
            char* tmpChar = (char *)(buf + readBytes - 1);
            if (*tmpChar == '\0')
            {
                break;
            }
        }
        
        nleft -= nread; // set left to read 
        buf +=nread; // set pointer 
    } 
    buf = 0; 

    if (DEBUG_START_END) printf("FullRead END\n");
    return readBytes; 
}

ssize_t FullWrite(int fd, const void *buf, size_t count) 
{ 
    if (DEBUG_START_END) printf("FullWrite START\n");
    size_t nleft; 
    ssize_t nwritten; 
    nleft = count; 
    
    while (nleft > 0)  // repeat until no left 
    {
        if ((nwritten = write(fd, buf, nleft)) < 0)
        { 
            if (errno == EINTR) // if interrupted by system call
            { 
                
                continue; // repeat the loop
            } 
            else 
            { 
                exit(nwritten); // otherwise exit with error 
            } 
        } 
        
        nleft -= nwritten; // set left to write 
        buf +=nwritten; // set pointer 
    } 

    if (DEBUG_START_END) printf("FullWrite END\n");    
    return (nleft); 
}

ssize_t FullReadFromStdIn(char *buf, size_t count) 
{ 
    if (DEBUG_START_END) printf("FullReadFromStdIn START\n");
    size_t nleft; 
    ssize_t nread; 
    nleft = count; 
    while (nleft > 0) // repeat until no left
    { 
        if ((nread = read(STDIN_FILENO, buf, nleft)) < 0) 
        { 
            if (errno == EINTR) // if interrupted by system call
            { 
                continue; //repeat the loop
            } 
            else 
            { 
                exit(nread); // otherwise exit
            }
        } 
        else if (nread == 0) // EOF
        {
            break; // break loop here 
        }
        else
        {
            for (int i = 0; i < nread; i++)
            {
                if ((char)*(buf + i) == '\n')
                {
                    if (DEBUG_START_END) printf("FullReadFromStdIn END\n");
                    return i;
                }
            }
        } 
        
        nleft -= nread; // set left to read 
        buf +=nread; // set pointer 
    } 
    buf = 0; 

    if (DEBUG_START_END) printf("FullReadFromStdIn END\n");
    return (nleft); 
}


void ScriviFile (char* nomeFile, void* datiDaSalvare, ssize_t sizeOfDatiDaSalvare)
{
    if (DEBUG_START_END) printf("ScriviFile START\n");
    FILE* pf;
 
    //apre file binario per scrivere informazioni NON di testo(del resto è una struttura)
    pf = fopen(nomeFile, "ab");
    if (pf == NULL) {
        fprintf(stderr, "\nError opening file\n");
        exit(1);
    }
 
    //SEEK_END -> fine del file
    if (fseek(pf, 0, SEEK_END) != 0)
    {   
        if (DEBUG_FILE == 1) printf("Errore durante la memorizzazione del File\n");
        if (DEBUG_START_END) printf("ScriviFile END\n");
        return;
    }

    // scrive la struct sul file
    int flag = 0;
    flag = fwrite(datiDaSalvare, sizeOfDatiDaSalvare, 1, pf);
    
    if (DEBUG_FILE == 1)
    {
        if (flag) 
            printf("File memorizzato con successo\n");
        else
            printf("Errore durante la memorizzazione del File\n");
    }
 
    //chiude il file
    fclose(pf);
    if (DEBUG_START_END) printf("ScriviFile END\n");
}

int LeggiFile (char* nomeFile, void* elemento, ssize_t offset, ssize_t sizeOfDatiDaLeggere, int position)
{
    if (DEBUG_START_END) printf("LeggiFile START\n");
    FILE* pf;
    int result = 0;
    
    //apre file binario per leggere informazioni NON di testo(del resto è una struttura)
    pf = fopen(nomeFile, "rb");
    if (pf == NULL) 
    {
        fprintf(stderr, "\nError opening file\n");
        exit(1);
    }
 
    //POSITION 
    //SEEK_SET -> inizio del file
    //SEEK_END -> fine del file
    if (fseek(pf, offset, position) != 0)
    {   
        if (DEBUG_FILE == 1) printf("Tutti gli elementi del file sono già stati letti\n");
        result = -1;
        goto chiudiFile;
    }

    // legge la struct dal file
    int flag = 0;
    flag = fread(elemento, 1, sizeOfDatiDaLeggere, pf);
    
    if (flag) 
    {
        if (DEBUG_FILE == 1) printf("File letto con successo\n");
    }
    else
    {
        result = -1;
        if (DEBUG_FILE == 1) printf("Errore durante la lettura del File\n");
        goto chiudiFile;
    }

chiudiFile: 
    //chiude il file
    fclose(pf);
    if (DEBUG_START_END) printf("LeggiFile END\n");
    return result;
}

