#include <stdio.h> 
#include <unistd.h>
#include <signal.h> 
int stringToInt( char* string )
{
	if( string[1] < 48 || string[1] > 57 )
	{
		return string[0] - 48;
	}

	return 10 * ( string[0] - 48 ) + ( string[1] - 48 );
}
int main( int argc, char *argv[ ] )
{
	
   int i,j,rajada;
   for(i=1; i < argc; i++){
	rajada=stringToInt(argv[i]);
	for(j=0;j<rajada;j++){
		printf("pid:%d  tempo:%d de %d\n",getpid(),j+1,rajada);
		sleep(1);
       	}
	if(i<argc-1){ 
		kill(getppid(),SIGUSR1);
		raise(SIGSTOP);
	}
   }
    kill(getppid(),SIGUSR2);
    return 0;
}
