#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h> 
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#define SHM 8750

#define SEM 7200

#define MAX_PROGRAMS 100
#define MAX_ARGS      20
#define MAX_ARG_LEN  25
#define TRUE  1
#define FALSE 0
#define EVER ; ;
#define N 100
union semun 
{ 
	int              val; 
	unsigned short  *array; 
	struct semid_ds *buf; 
};

typedef struct processo
{	
	int pid;
	int  *rajadas;
	int rajada_corrente;
	int tempo_corrente;
}PR ;
typedef struct IO_
{
	PR *elem;
	int num_fila;
	struct timeval IO_tv;
}IO ;


typedef struct fila Fila;

int shm, *p;

char ***exec_args;

void intHandler( int signal );

int limpaEntrada( char* line, char ***exec_args );

int stringToInt( char* string );

int setSemValue( int semId );
void W4IOHandler(int signal);
void trataalarme(int signal);
void tratafim(int signal);
void delSemValue( int semId );
int semaforoP( int semId );

int semaforoV( int semId );
Fila *F[3];
Fila *FIO ;
PR *temp;
int corr_fila=0;
int corr_deltat=0;