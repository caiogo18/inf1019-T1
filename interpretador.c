#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include "aux.h"

#define SHM 8550

#define SEM 7300

union semun
{
	int              val;
	unsigned short  *array;
	struct semid_ds *buf;
};

char* pointer_sharedmem();

void obtemArgs( char* line, char *items, int n );

int setSemValue( int semId );

void delSemValue( int semId );

int semaforoP( int semId );

int semaforoV( int semId );

void error_try_again();

int main (void)
{

	char  line[MSG_MAX_SIZE];
	char *items;
	int n =  100;
	int bytes_lidos;

	int semId;

	char *p;
	printf("Inicializando o Intepretador no pid: %d\n", getpid());
	p = NULL;
	items = ( char * ) malloc( n * sizeof( char ) );
	if( items == NULL )
	{
		printf("interpretador.c: Nao foi possivel alocar memoria.\n");
		exit( 1 );
	}


//cria o semaforo
	semId = semget( SEM, 1, 0666 | IPC_CREAT );
	if( semId == -1 )
	{
		printf("interpretador.c: Nao foi possivel criar o semaforo.\n");
		exit( 1 );
	}

	if( setSemValue( semId ) == -1 )
	{
		printf("interpretador.c: Nao foi possivel operar o semaforo.\n");
		exit( 1 );
	}
	//cria a memoria compartilhada alocando a memoria

	//grava na memoria compartilhada como um grande vetor de char
	while( 1 )
	{
		bytes_lidos = scanf(" %[^\n]s",line);
		p = pointer_sharedmem();
		if(bytes_lidos > 0){
			semaforoP( semId );
			{
				obtemArgs( line, items, n );
				strcat( ( char * ) p, items );
				strcat( ( char * ) p, "\n" );
				strcat( ( char * ) p, "\0" );
			}
			semaforoV( semId );
			ms_sleep(1);
		}
	}

	free( items );
	return 0;
}

char* pointer_sharedmem(){
	char* p;
	int shm_rr   = shmget( SHM, SHARED_MEMORY_SIZE, S_IRUSR | S_IWUSR );
	if( shm_rr == -1 )
	{
		while(shm_rr == -1){
			printf("interpretador.c: Nao foi possivel alocar memoria compartilhada. Garanta que o escalonador esteja em execucao.\n");
			error_try_again();
			shm_rr   = shmget( SHM, SHARED_MEMORY_SIZE, S_IRUSR | S_IWUSR );
		}
	}

	p     = ( char * ) shmat( shm_rr, 0, 0 );
	if( p == NULL )
	{
		while(p == NULL){
			printf("interpretador.c: Nao foi possivel acessar memoria compartilhada. Garanta que o escalonador esteja em execucao.\n");
			error_try_again();
			p = ( char * ) shmat( shm_rr, 0, 0 );
		}
	}
	return p;
}

void error_try_again(){
	char input[100];
	char resposta;
	printf("interpretador.c: Tentar novamente? [y/n]: ");
	scanf("%s",input);
	if(input[1] == '\0'){
		resposta = input[0];
		if(resposta == 'y' || resposta == 'Y'){
			return;
		}
	}
	printf("interpretador.c: Encerrando execucao\n");
	exit(1);
}

void obtemArgs( char* line, char *items, int n )
{
	int i;
	int c = 0;

	for( ; c < n && line[c] != ' '; c++ );
	c++;

	for( i = 0; i < n && c < n && line[c] != '\n'; c++, i++ )
	{
		items[i] = line[c];
	}
}



int setSemValue( int semId )
{
	union semun semUnion;

	semUnion.val = 1;

	return semctl( semId, 0, SETVAL, semUnion );
}



void delSemValue( int semId )
{
	union semun semUnion;

	semctl( semId, 0, IPC_RMID, semUnion );
}



int semaforoP( int semId )
{
	struct sembuf semB;

	semB.sem_num =  0;
	semB.sem_op  = -1;
	semB.sem_flg = SEM_UNDO;

	semop( semId, &semB, 1 );

	return 0;
}



int semaforoV(int semId)
{
	struct sembuf semB;

	semB.sem_num = 0;
	semB.sem_op  = 1;
	semB.sem_flg = SEM_UNDO;

	semop( semId, &semB, 1 );

	return 0;
}
