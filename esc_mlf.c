#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include "Fila.h"
#include "aux.h"
#define SHM 8550

#define SEM 7300

#define MAX_PROGRAMS 10
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
	int  *rajadas;
	pid_t pid;
	int rajada_corrente;
}PR ;
typedef struct IO_
{
	int num_fila;
	long sec;
}IO ;



int shm;
char* p;

char ***exec_args;

void intHandler( int signal );

int limpaEntrada( char* line, char ***exec_args );

int stringToInt( char* string );

int setSemValue( int semId );
void sig_handler(int signal);
void delSemValue( int semId );
void printa(PR *v[],int i);
int semaforoP( int semId );

int semaforoV( int semId );
Fila *F[3];
Fila *FIO ;
PR *VPR[N];
IO *VIO[N];
PR *temp;
struct timeval aux_tv;
int pos,sit;
int corr_fila=0;
int corr_deltat=0;
int main (void)
{
	int num_new_items=0;
	int i, j,progs;
	int semId;
	int deltat=2;
	char command[30];
	char** command_args ;
	int numero_de_argumentos=0;
	int esperar=0;
	struct timeval corr_tv;
	printf("Inicializando Escalonador no processo: %d\n", getpid());
	//cria o vetor de vetores de strings
	exec_args = ( char *** ) malloc( MAX_PROGRAMS * sizeof( char ** ) );
	FIO= fila_cria() ;
	for(i=0;i<3;i++){
		F[i]= fila_cria() ;
	}
	if( exec_args == NULL )
	{
		printf( "esc.c Erro: Nao foi possivel alocar memoria.\n" );
		exit( 1 );
	}

	for( i = 0; i < MAX_PROGRAMS; i++ )
	{
		exec_args[i] = ( char ** ) malloc( MAX_ARGS * sizeof(  char * ) );
		if( exec_args[i] == NULL )
		{
			printf( "escalonador.c Erro: Nao foi possivel alocar memoria.\n" );
			exit( 1 );
		}
		for( j = 0; j < MAX_ARGS; j++ )
		{
			exec_args[i][j] = ( char * ) malloc( MAX_ARG_LEN * sizeof( char ) );
			if( exec_args[i][j] == NULL )
			{
				printf( "escalonador.c Erro: Nao foi possivel alocar memoria.\n" );
				exit( 1 );
			}
		}
	}
	//cria os handlers
	if( signal( SIGINT, intHandler ) == SIG_ERR )
	{
		printf( "esc.c Erro: Nao foi possivel instalar rotina de atendimento.\n" );
		exit( 1 );
	}
	if( signal( SIGUSR2, sig_handler ) == SIG_ERR )
	{
		printf( "esc.c Erro: Nao foi possivel instalar rotina de atendimento.\n" );
		exit( 1 );
	}
	if( signal( SIGUSR1, sig_handler) == SIG_ERR )
	{
		printf( "esc.c Erro: Nao foi possivel instalar rotina de atendimento.\n" );
		exit( 1 );
	}
	//cria a memoria compartilhada
	shm = shmget( SHM, MSG_MAX_SIZE * sizeof( char ), IPC_CREAT | S_IRUSR | S_IWUSR);
	if( shm == -1 )
	{
		printf("esc.c Erro: Nao foi possivel alocar memoria compartilhada\n");
		exit( 1 );
	}

    	p  = ( char * ) shmat( shm, 0, 0 );
	if( p == NULL )
	{
		printf("esc.c: Nao foi possivel alocar memoria compartilhada\n");
		exit( 1 );
	}
	//cria o semaforo
	semId = semget( SEM, 1, 0666 | IPC_CREAT );
	if( semId == -1 )
	{
		printf("esc.c: Nao foi possivel criar o semaforo.\n");
		exit( 1 );
	}

	if( setSemValue( semId ) == -1 )
	{
		printf("esc.c: Nao foi possivel operar o semaforo.\n");
		exit( 1 );
	}
	memset( p, 0, strlen( ( char * ) p ) + 1 );
	p[0] = '\0';
	//pid_t pid  = fork( );
	//if( pid == 0 ) execl("./interpretador","interpretador",NULL);
	progs=0;
	while(1){//while(esperar<10&&progs<MSG_MAX_SIZE){
		num_new_items = 0;
		//area critica entre interpretador e escalonador
		semaforoP( semId );
		//transforma o grande vetor de char da memoria compartilhada para um vetor de vetores strings
		num_new_items=limpaEntrada( ( char * ) p, exec_args );
		memset( p, 0, strlen( ( char * ) p ) + 1 );
		p[0] = '\0';
		semaforoV( semId );
		i=0;
		if(num_new_items>0){
			numero_de_argumentos=0;
			for(j=0;j<MAX_ARGS &&exec_args[i][j][0]!='\0';j++){
				numero_de_argumentos++;;
			}
			if(numero_de_argumentos<=1){
				printf("argumentos insuficientes\n");
			}
			else{
				pid_t pid  = fork( );

				if( pid < 0 )
				{
					printf( "escalonador.c Erro: Nao foi possivel criar novo processo.\n" );
			  		  exit( 2 );
				}

				else if( pid >0 )
				{
					temp= ( PR * )malloc( sizeof( PR ) );
					temp->rajada_corrente=0;
					temp->rajadas=malloc((numero_de_argumentos-1)*sizeof(int));
					for(j=1;j<numero_de_argumentos;j++){
						temp->rajadas[j-1]=stringToInt(exec_args[i][j]);
					}
					temp->pid=pid;
					VPR[progs]=temp;
					fila_insere(F[0],progs);
					kill( VPR[progs]->pid, SIGSTOP );
					printf("Processo %d adicionado\n", VPR[progs]->pid);
					i++;
					progs++;

				}
				else{
					strcpy( command, "./" );
					strcat( command, exec_args[i][0] );
					command_args=(char **)malloc(sizeof(char *)*(numero_de_argumentos));
					for(j=0;j<numero_de_argumentos;j++){
						command_args[j]=exec_args[i][j];
					}
					execv( command, command_args);
				}
					/*
					 * Pausa o programa recem-criado.
					 */
			}

		}
		sit=0;
		if(!fila_vazia(F[0])){
			pos=fila_retira(F[0]);
			corr_fila=0;
			corr_deltat=deltat;
			#ifdef DEBUG
			printf( "SIGCONT -  %d  -  quantum - %d \n", VPR[pos]->pid,deltat);
			#endif
			kill( VPR[pos]->pid, SIGCONT );
			sleep(deltat);

		}
		else if(!fila_vazia(F[1])){
			pos=fila_retira(F[1]);
			corr_fila=1;
			corr_deltat=2*deltat;
			#ifdef DEBUG
			printf( "SIGCONT -  %d  -  quantum - %d \n", VPR[pos]->pid,2*deltat);
			#endif
			kill( VPR[pos]->pid, SIGCONT );
			sleep(2*deltat);
		}
		else if(!fila_vazia(F[2])){
			pos=fila_retira(F[2]);
			corr_fila=2;
			corr_deltat=4*deltat;
			#ifdef DEBUG
			printf( "SIGCONT -  %d  -  quantum - %d \n", VPR[pos]->pid,4*deltat);
			#endif
			kill( VPR[pos]->pid, SIGCONT );
			sleep(4*deltat);
		}
		else if(!fila_vazia(FIO)) sit=3;
		else{
			if(esperar%1000 == 0){
				printf("Nenhum processo esperando..\n");
			}
			esperar++;
			ms_sleep(1);
			continue;
		}
		gettimeofday (&corr_tv, NULL);
		while(!fila_vazia(FIO)){
			j=fila_obtem(FIO);
			if(corr_tv.tv_sec-VIO[j]->sec>=3){
				j=fila_retira(FIO);
				fila_insere(F[VIO[j]->num_fila],j);
				printf("Processo %d voltou para FILA %d\n",VPR[j]->pid,VIO[j]->num_fila);
				free(VIO[j]);
			}
			else{
				break;
				printf("descansar..\n");
				sleep(1);
			}
		}
		//se recebeu o sinal USR1(W4IO)
		if(sit==1){
			printf("Processo %d entrou na FILA de IO\n",VPR[pos]->pid);
			VPR[pos]->rajada_corrente++;
			IO *temp_io=(IO *)malloc(sizeof(IO));
			corr_fila=corr_fila-1;
			if(corr_fila<0) corr_fila=0;
			temp_io->num_fila=corr_fila;
			gettimeofday (&aux_tv, NULL);
			temp_io->sec=aux_tv.tv_sec;
			VIO[pos]=temp_io;
			fila_insere(FIO,pos);
		}
		//se recebeu o sinal USR2(filho terminou)
		else if(sit==2){
			printf("pid:%d  Morri!\n",VPR[pos]->pid);
			kill( VPR[pos]->pid, SIGKILL );
			free(VPR[pos]);
		}
		//nenhum processo para escalonar
		else if(sit==3);
		//nao recebeu sinal
		else{
			kill( VPR[pos]->pid, SIGSTOP );
			corr_fila++;
			if(corr_fila>2) corr_fila=2;
			fila_insere(F[corr_fila],pos);
		}
	}
	return 0;
}



void intHandler( int signal )
{
	int i, j;


	for( i = 0; i < MAX_PROGRAMS; i++ )
	{
		for( j = 0; j < MAX_ARGS; j++ )
		{
			free( exec_args[i][j] );
		}
		free( exec_args[i] );
	}
	free( exec_args );

	shmdt( p );
	shmctl( shm, IPC_RMID, 0 );
	exit( 0 );
}

void sig_handler(int signal){
        switch(signal) {
                case SIGUSR1 :
                        sit=1;
                        break;
                case SIGUSR2 :
                        sit=2;
                        break;
                default :
                        printf("SIGNAL %d\n", signal);
	}
}
int limpaEntrada( char* buffer, char ***exec_args )
{
	int i, j, k, c = 0;

	for( i = 0; i < MAX_PROGRAMS && buffer[c] != '\0'; i++ )
	{
		for( j = 0; j < MAX_ARGS && buffer[c] != '\n' && buffer[c] != '\0'; j ++ )
		{
			for( k = 0; k < MAX_ARG_LEN - 1 && buffer[c] != ' ' && buffer[c] != '\n' && buffer[c] != '\0'; k++ )
			{
				exec_args[i][j][k]   = buffer[c];
				exec_args[i][j][k+1] = '\0';
				c++;
			}

			if( buffer[c] == ' ' )
			{
				c++;
			}
		}

		if( buffer[c] == '\n' )
		{
			c++;
		}
	}
	return i;
}


int stringToInt( char* string )
{
	if( string[1] < 48 || string[1] > 57 )
	{
		return string[0] - 48;
	}

	return 10 * ( string[0] - 48 ) + ( string[1] - 48 );
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

void printa(PR *v[],int i){
	printf("pid : %d\n rajadas %d %d %d\n rajada atual: %d\n",VPR[i]->pid,VPR[i]->rajadas[0],VPR[0]->rajadas[1],VPR[i]->rajadas[2],VPR[i]->rajada_corrente);
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
