#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef int pid_t;

/*
los procesos que se generan son 2^n 
siendo n = la cant de veces que hago fork
Entonces hay 2¹² procesos.

Puede fallar si hay muchos procesos corriendo a la vez o no hay suficientes recursos -> SE ME TRABO TODO
*/
int main(int argc, char* argv[]) {
  printf("Mi PID es: %d\n", getpid());

  for (int i = 0; i < 12; i++) {
    pid_t r = fork();

    if (r < 0) {
      perror("Error en fork");
      exit(-1);
    }

    printf("[%d] Hola!\n", getpid());
  }

  printf("Terminando\n");
  exit(0);
}
