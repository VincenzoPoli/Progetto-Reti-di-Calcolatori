#include "includes.h"

#define CODE_SIZE               20

#define NONE                    -1
#define GUARITO                 0
#define CONTAGIATO              1

#define NON_VALIDO              0
#define VALIDO                  1

#define MAX_CLIENT_NUM          1024
#define BUF_SIZE                4096

#define VALIDITA_GREENPASS      180

#define SERVER_V_IP             "10.0.2.15"
#define SERVER_V_PORT_CV        5031
#define SERVER_V_PORT_SG        5032

#define CENTRO_VACCINALE_IP     SERVER_V_IP
#define CENTRO_VACCINALE_PORT   5033

#define SERVER_G_IP             SERVER_V_IP
#define SERVER_G_PORT_CS        5026
#define SERVER_G_PORT_CT        5027

#include "datastructures.c"

void address_initializer (struct sockaddr_in* addr, int port);
void address_connect_initializer (struct sockaddr_in* addr, char* ip, int port);
char StatoContagio[2][11] = {"GUARITO", "CONTAGIATO"};
char StatoGreenPass[2][11] = {"NON VALIDO", "VALIDO"};

struct GreenPasss
{
    char code[CODE_SIZE + 1];     //codice della tessera sanitaria
    int valid;                    //validità espressa in giorni
};


typedef struct
{
    char code[CODE_SIZE + 1];   //codice della tessera sanitaria
    int valid;                  //validità espressa in secondi 
                                //  (1 giorno = 86400 secondi, va moltiplicato * 180, tutto questo va fatto nell'header nel define o con una macro)
    time_t t;                   //data e ora corrente (al momento della registrazione della vaccinazione)
} GreenPass;


typedef struct
{
    char codice[CODE_SIZE + 1];     //codice della tessera sanitaria
    int contagiato;                 //1 se contagiato, 0 se guarito
} UtenteContagiato;

/*
typedef struct
{
    char code[CODE_SIZE + 1];//codice della tessera sanitaria
    
    int contagiato;
    
    
} GreenPass;
*/

/*
void printDate(struct tm timeStamp)
{
    printf("Ora: %d-%02d-%02d %02d:%02d:%02d\n", timeStamp.tm_year + 1900, timeStamp.tm_mon + 1, timeStamp.tm_mday, timeStamp.tm_hour, timeStamp.tm_min, timeStamp.tm_sec);
}

void printDate(struct tm timeStamp)
{
    printf("Ora: %d-%02d-%02d %02d:%02d:%02d\n", timeStamp.tm_year + 1900, timeStamp.tm_mon + 1, timeStamp.tm_mday, timeStamp.tm_hour, timeStamp.tm_min, timeStamp.tm_sec);
}

GreenPass gp; //dichiaro un green pass
gp.t = time(NULL); //inizializzazione della variabile tempo del green pass
gp.timeStamp = *localtime(&gp.t); //ottengo la data attuale e la salvo nel campo apposito del green pass

time_t t = time(NULL); //inizializzazione della variabile tempo 
struct tm oggi = *localtime(&t); //ottengo la data attuale e la salvo in now

time_t dataRilascio = mktime(&gp.timeStamp); //così si converte il timeStamp in numero di secondi
time_t dataOggi = mktime(&oggi);
time_t differenza = dataOggi-dataRilasio;

if (differenza >= gp.valid) //eccetera eccetera

*/
