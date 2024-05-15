#define DEBUG_FILE              0
#define DEBUG_READ              0
#define DEBUG_START_END         0

#include "includes.h"
#include "wrapper.c"

//Funzioni Wrapper
int Socket(int, int, int);
void Connect(int, const struct sockaddr*, socklen_t);
void Bind(int, const struct sockaddr*, socklen_t);
int Accept(int, struct sockaddr*, socklen_t*);
void Listen(int, int);
int Select(int maxfdp1, fd_set *readset, fd_set *writeset, fd_set *exceptset, struct timeval *timeout);
ssize_t FullRead(ssize_t, void*, size_t);
ssize_t FullWrite(int fd, const void *buf, size_t count);
ssize_t FullReadFromStdIn(char *buf, size_t count);
void ScriviFile (char* nomeFile, void* datiDaSalvare, ssize_t sizeOfDatiDaSalvare);
int LeggiFile (char* nomeFile, void* elemento, ssize_t offset, ssize_t sizeOfDatiDaLeggere, int position);




