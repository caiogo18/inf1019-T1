
#include "Fila.h"
Fila* fila_cria (void){
	Fila* f = (Fila*) malloc(sizeof(Fila));
	f->n = 0; /*inicializa uma fila vazia*/
	return f;
}
void fila_insere (Fila* f, int v){
	No *novo,*aux;
	novo=(No *)malloc(sizeof(No));
	novo->elem=v;
	if(f->ini==0){
		novo->prox=novo;
		novo->ant=novo;
		f->ini=novo;
	}
	else{	
		novo->prox=f->ini;
		novo->ant=f->ini->ant;
		aux=f->ini->ant;
		aux->prox=novo;
		aux=f->ini;
		aux->ant=novo;
	}
	f->n++;
	return;
}

int fila_vazia (Fila* f){
	return (f->n == 0);
}

int fila_retira (Fila* f){
	int v;
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
int fila_obtem (Fila* f){
	int v;
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
	
	
