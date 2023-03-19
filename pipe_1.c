#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef int pid_t;
/*
En cada proceso tengo el fds0 y fds1. No puedo, dentro de un mismo proceso leer y escribir
*/
int main(int argc, char* argv[]) {
  int fds[2];
  int msg = 42;

  int p = pipe(fds);
  if(p == -1)
    exit(-1);

  pid_t i = fork();

  if (i == 0) {
    printf("[hijo] mi pid es: %d\n", getpid());
    // El hijo no va a escribir
    close(fds[1]);

    int recv = 0;
    read(fds[0], &recv, sizeof(recv));
    printf("[hijo] lei: %d\n", recv); //se queda esperando hasta que el proceso padre escriba

    close(fds[0]);
  } else {
    printf("[padre] mi pid es: %d\n", getpid());
    // El padre no va a leer
    close(fds[0]);

    // Esperamos dos segundos, el hijo no debería seguir
    sleep(2);
    write(fds[1], &msg, sizeof(msg));

    close(fds[1]);
  }
}

