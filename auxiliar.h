#ifndef AUX_H
#define AUX_H

/**
* Funcao Sleep em milisegundos
*/
void ms_sleep(long milis);

#define MSG_MAX_SIZE 100

#define SHARED_MEMORY_SIZE (MSG_MAX_SIZE * sizeof(char))

//#define DEBUG

#endif
