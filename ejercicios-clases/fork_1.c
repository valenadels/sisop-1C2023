#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/*SALIDA: lA DIREC DE A ES LA MISMA
[padre] mi pid es: 36058
[padre] a=6
DIR DE A: 0x7ffc1b478ec0
Terminando
[hijo] mi pid es: 36059
[hijo] a=5
DIR DE A: 0x7ffc1b478ec0
Terminando


cuando un proceso llama a fork(), se crea un proceso hijo con una copia independiente del espacio de memoria 
del proceso padre. Esto significa que ambos procesos tienen copias separadas de todas las variables,
 incluyendo la variable "a". Por lo tanto, en general, la dirección de "a" en el proceso padre y 
 el proceso hijo debería ser diferente.

 La dirección que se muestra con el operador "&" en C representa la dirección 
 de memoria virtual de la variable en el espacio de memoria del proceso en el 
 que se está ejecutando el programa.
*/
typedef int pid_t;

int main(int argc, char* argv[]) {
  int a = 4;
  pid_t i = fork();

  a = 5;

  if (i < 0) {
    printf("Error en fork! %d\n", i);
    exit(-1);
  }

  if (i == 0) {
    printf("[hijo] mi pid es: %d\n", getpid());
    printf("[hijo] a=%d\n", a);
    printf("DIR DE A: %p\n", &a);
  } else {
    a = 6;
    printf("[padre] mi pid es: %d\n", getpid());
    printf("[padre] a=%d\n", a);
    printf("DIR DE A: %p\n", &a);
  }

  printf("Terminando\n");
  exit(0);
}
