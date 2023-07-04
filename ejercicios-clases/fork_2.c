#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

typedef int pid_t;

int main(int argc, char* argv[]) {
  char* msg = "fisop\n";

  // Abro un archivo y si no existe lo creo
  /*Los parámetros en esta línea de código corresponden a:
  "hola.txt": El nombre del archivo que se desea abrir o crear.
  O_CREAT: Un flag que indica que se debe crear el archivo si no existe.
  O_RDWR: Un flag que indica que se debe abrir el archivo para lectura y escritura.
  0644: Un valor octal que representa los permisos de acceso para el archivo que se está creando.
  En este caso, 0644 indica que el archivo tendrá permisos de lectura y escritura para el usuario propietario,
   y permisos de sólo lectura para el grupo y otros usuarios.
  */
  int fd = open("hola.txt", O_CREAT | O_RDWR, 0644);
  pid_t i = fork();

  if (i < 0) {
    printf("Error en fork! %d\n", i);
    exit(-1);
  }

  if (i == 0) {
    printf("[hijo] mi pid es: %d\n", getpid());
    write(fd, msg, 6);
    close(fd);
  } else {
    printf("[padre] mi pid es: %d\n", getpid());
    close(fd);
  }

  printf("Terminando\n");
  exit(0);
}
