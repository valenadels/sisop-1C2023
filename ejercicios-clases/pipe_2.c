#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

/*       The <errno.h> header file defines the integer variable errno,
       which is set by system calls and some library functions in the
       event of an error to indicate what went wrong.

   errno
       The value in errno is significant only when the return value of
       the call indicated an error (i.e., -1 from most system calls; -1
       or NULL from most library functions); a function that succeeds is
       allowed to change errno.  The value of errno is never set to zero
       by any system call or library function.


      En algun momento falla porque el pipe se llena xq nadie lee, aunque lo que pasa es:
      Cuando un proceso intenta escribir en un pipe y no hay suficiente espacio disponible en el buffer
      del extremo de lectura, la llamada al sistema write() se bloquea hasta que se libera suficiente 
      espacio en el buffer o hasta que ocurre un error. 

      Si el proceso bloqueado en la llamada a write() recibe una señal durante el bloqueo, la llamada a write()
      se interrumpe y devuelve un código de error EINTR. EINTR es un código de error (errno) que indica que una
       llamada al sistema fue interrumpida por una señal. Cuando un proceso está bloqueado en una llamada al
        sistema que puede ser interrumpida por una señal, como por ejemplo read(), write() o wait(),
         la señal puede interrumpir la llamada al sistema y devolver el código de error EINTR.
          La señal que interrumpió la llamada a write() puede ser cualquier señal que esté 
          configurada para ser entregada al proceso, como SIGINT o SIGTERM.


pipe(7)
La página del manual pipe(7) describe los detalles técnicos sobre la función pipe() y su comportamiento. Específicamente, esta página del manual describe el uso de los pipes y sus características en los sistemas operativos basados en UNIX.

En general, el manual pipe(7) describe los siguientes aspectos de los pipes:

Cómo se crean y se utilizan los pipes en los sistemas operativos basados en UNIX.

Las restricciones en la cantidad de datos que pueden ser escritos y leídos en un pipe.

Las limitaciones de la sincronización en los pipes, incluyendo cómo el bloqueo y la 
no-bloqueo funcionan en la escritura y lectura de datos en un pipe.

Las características de los pipes sin nombre (unidireccionales) y los pipes con nombre (bidireccionales).

Cómo se pueden utilizar los pipes en combinación con otras funciones del 
sistema operativo, como fork(), dup(), fcntl(), select(), etc.

Cómo se pueden utilizar los pipes en la comunicación entre procesos.

En resumen, la página del manual pipe(7) proporciona información detallada sobre el uso 
y la implementación de los pipes en los sistemas operativos basados en UNIX.

*/

int main(int argc, char* argv[]) {
  int fds[2];

  pipe(fds);
  //chequeo pipe 
  
  printf("Lectura: %d, Escritura: %d\n", fds[0], fds[1]);

  int msg = 42;
  int escritos = 0;
  while (1) {
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

  close(fds[0]);
  close(fds[1]);
}

