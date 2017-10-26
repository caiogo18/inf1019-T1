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

struct node{
	struct node * prox;
	struct node * ant;
	void *elem;
};
typedef struct node No;
struct fila{
	int n;
	No *ini;
};

typedef struct fila Fila;
Fila* fila_cria (void){
	Fila* f = (Fila *) malloc(sizeof(Fila));
	f->n = 0; /*inicializa uma fila vazia*/
	return f;
}
void fila_insere (Fila* f, void * v){
	No *novo,*aux;
	aux=f->ini;
	novo=(No *)malloc(sizeof(No));
	novo->elem=v;
	if(f->ini==0){
		novo->prox=novo;
		novo->ant=novo;
		f->ini=novo;
	}
	else{	
	novo->prox=aux;
	novo->ant=aux->ant;
	aux=aux->ant;
	aux->prox=novo;
	}
	f->n++;
}

int fila_vazia (Fila* f){
	return (f->n == 0);
}

void *fila_retira (Fila* f){
	void * v;
	No *aux;
	if (fila_vazia(f)){
		printf("Fila vazia\n");
		exit(1);
	}
	v = f->ini->elem;
	f->n--;
	if(f->n==0){
		aux=f->ini;
		free(aux);
		f->ini=NULL;
		return v;
	}
	aux = f->ini->prox;
	aux->ant=f->ini->ant;
	aux=f->ini->ant;
	aux->prox=f->ini->prox;
	aux=f->ini;
	f->ini=aux->prox;
	free(aux);
	return v;
}
void *fila_obtem (Fila* f){
	void * v;
	if (fila_vazia(f)){
		printf("Fila vazia\n");
		exit(1);
	}
	v = f->ini->elem;
	return v;
}
void fila_libera (Fila* f){
	free(f);
}
int main(void){
	int numero_de_rajadas=3,j;
	Fila *F;
	PR *temp;
	F=fila_cria();
	temp=( PR * ) malloc( sizeof( PR ) );
	temp->rajadas=(int *)malloc(numero_de_rajadas*sizeof(int));
	for(j=1;j<numero_de_rajadas+1;j++){
		temp->rajadas[j-1]=j;
	}
		temp->rajada_corrente=0;
		temp->tempo_corrente=0;		
		temp->pid=100;
		fila_insere(F,temp);
		
	temp=( PR * ) malloc( sizeof( PR ) );
	temp->rajadas=(int *)malloc(numero_de_rajadas*sizeof(int));
	for(j=1;j<numero_de_rajadas+1;j++){
		temp->rajadas[j-1]=j+5;
	}
		temp->rajada_corrente=0;
		temp->tempo_corrente=0;		
		temp->pid=200;
		fila_insere(F,temp);
		temp=( PR * ) malloc( sizeof( PR ) );
	temp->rajadas=(int *)malloc(numero_de_rajadas*sizeof(int));
	for(j=1;j<numero_de_rajadas+1;j++){
		temp->rajadas[j-1]=j+20;
	}
		temp->rajada_corrente=0;
		temp->tempo_corrente=0;		
		temp->pid=500;
		fila_insere(F,temp);
		temp=fila_retira(F);
		printf("%d",temp->pid);
		for(j=0;j<3;j++) printf(" %d ",temp->rajadas[j]);
		temp=fila_retira(F);
		printf("%d",temp->pid);
		for(j=0;j<3;j++) printf(" %d ",temp->rajadas[j]);
	temp=fila_retira(F);
		printf("%d",temp->pid);
		for(j=0;j<3;j++) printf(" %d ",temp->rajadas[j]);
return 0;

}
