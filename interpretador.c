#include <stdio.h>
#include <stdlib.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define SHM 8550

#define SEM 7300

union semun 
{ 
	int              val; 
	unsigned short  *array; 
	struct semid_ds *buf; 
};


void obtemArgs( char* line, char *items, int n );

int setSemValue( int semId );

void delSemValue( int semId );

int semaforoP( int semId );

int semaforoV( int semId );


int main (void)
{ 
	
	char  line[40];

	char *items;
	int n =  100;

	int semId;

	int *p, shm_rr;
	
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
	shm_rr   = shmget( SHM, 100 * sizeof( int ),  S_IRUSR | S_IWUSR );
	if( shm_rr == -1 )
	{
		printf("interpretador.c: Nao foi possivel alocar memoria compartilhada.\n");
		exit( 1 );
	}
	//seta ela para o ponteiro
	p     = ( int * ) shmat( shm_rr, 0, 0 );
	if( ( long ) p == -1 )
	{
		printf("interpretador.c: Nao foi possivel alocar memoria compartilhada.\n");
		exit( 1 );
	}
	//grava na memoria compartilhada como um grande vetor de char
	while( 1 )
	{
		scanf(" %[^\n]s",line);
		semaforoP( semId );
		{
			obtemArgs( line, items, n );
			printf("%s\n",line);
			strcat( ( char * ) p, items );
			strcat( ( char * ) p, "\n" );
			strcat( ( char * ) p, "\0" );
		}
		semaforoV( semId );
		sleep(1);
		
	}

	free( items );

	
	return 0; 
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
