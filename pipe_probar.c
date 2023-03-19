#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char* argv[]) {
  int fds[2];

  pipe(fds);

  printf("Escritura: %d, lectura: %d\n", fds[1], fds[0]);

  close(fds[0]); //se cierra el extremo de lectura antes de escribir y el pipe no funciona con un solo lado, tira SIGPIPE
  //Si se intenta escribir en el extremo de escritura después de que el extremo de lectura haya 
  //sido cerrado, la llamada al sistema write() fallará con el error EPIPE y el proceso que intenta 
  //escribir recibirá la señal SIGPIPE.

  int msg = 42;
  int escritos = 0;
  while (1) {
    printf("Voy a intentar escribir\n");
    int r = write(fds[1], &msg, sizeof(msg));
    if (r >= 0) {
      printf("Total escrito: %d\n", escritos);
      escritos += sizeof(msg);
    } else {
      printf("write fallo con %d\n", r);
      printf("errno was: %d\n", errno);
      perror("perror en write");
      break;
    }
  };

  close(fds[1]);
}
