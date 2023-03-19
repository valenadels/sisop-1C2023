#include<stdio.h>
#include <stdlib.h>
#include <unistd.h>


/*SALIDA:
Mi PID es: 32968
Soy el proceso padre y mi pid es: 32968
Variable i: 32969
Terminando
Soy el proceso hijo y mi pid es: 32969                                                                                         
Terminando
*/
typedef int pid_t;

int main(int argc, char* argv[]) {
  printf("Mi PID es: %d\n", getpid());

  pid_t i = fork();

  if (i < 0) {
    printf("Error en fork! %d\n", i);
    exit(-1);
  }

  if (i == 0) {
    printf("Soy el proceso hijo y mi pid es: %d\n", getpid());
  } else {
    //devuelve el pid del hijo al proceso padre 
    printf("Soy el proceso padre y mi pid es: %d\n", getpid());
    printf("Variable i: %d\n", i);
  }

  printf("Terminando\n");
  exit(0);
}
