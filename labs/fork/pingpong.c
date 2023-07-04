#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/wait.h>
#include <stdbool.h>

#define ERROR_STATUS -1

void
verificar_error(int i, char *mensaje)
{
	if (i < 0) {
		fprintf(stderr, "%s\n", mensaje);
		fprintf(stderr, "Errno: %d\n\n", errno);
		exit(ERROR_STATUS);
	}
}

int
_pipe(int *fds)
{
	int p = pipe(fds);
	verificar_error(p, "Error al crear pipe");
	return p;
}

void
proceso_hijo(int *hijo_a_padre, int *padre_a_hijo)
{
	int escritura = hijo_a_padre[1];
	int lectura = padre_a_hijo[0];

	close(padre_a_hijo[1]);  // no voy a escribir con ese pipe
	close(hijo_a_padre[0]);  // no voy a leer con ese pipe

	int recibido;
	ssize_t r = read(lectura, &recibido, sizeof(recibido));
	verificar_error((int) r, "Error al leer el mensaje del padre");

	printf("Donde fork me devuelve 0:\n"
	       "  - getpid me devuelve: %d\n"
	       "  - getppid me devuelve: %d\n"
	       "  - recibo valor %d vía fd=%d\n",
	       getpid(),
	       getppid(),
	       recibido,
	       lectura);

	ssize_t w = write(escritura, &recibido, sizeof(recibido));
	verificar_error((int) w, "Error al escribir del hijo al padre");

	printf("  - reenvío valor en fd=%d y termino\n\n", escritura);

	close(escritura);
	close(lectura);
}

void
proceso_padre(int *padre_a_hijo, int *hijo_a_padre, pid_t f)
{
	int escritura = padre_a_hijo[1];
	int lectura = hijo_a_padre[0];
	close(padre_a_hijo[0]);  // no voy a leer con ese pipe
	close(hijo_a_padre[1]);  // no voy a escribir con ese pipe

	int mensaje = random();
	ssize_t i = write(escritura, &mensaje, sizeof(mensaje));
	verificar_error((int) i, "Error al escribir del padre al hijo");
	printf("Donde fork me devuelve %d:\n"
	       "  - getpid me devuelve: %d\n"
	       "  - getppid me devuelve: %d\n"
	       "  - random me devuelve: %d\n"
	       "  - envío valor %d a través de fd=%d\n\n",
	       f,
	       getpid(),
	       getppid(),
	       mensaje,
	       mensaje,
	       escritura);

	int recibido;
	ssize_t r = read(lectura, &recibido, sizeof(recibido));
	verificar_error((int) r, "Error al leer el mensaje del hijo");

	printf("Hola, de nuevo PID %d:\n"
	       "  - recibí valor %d vía fd=%d\n",
	       getpid(),
	       recibido,
	       lectura);

	close(escritura);
	close(lectura);
	wait(NULL);
}

int
main(void)
{
	srandom((unsigned) time(NULL));

	int padre_a_hijo[2];
	int hijo_a_padre[2];
	_pipe(padre_a_hijo);
	_pipe(hijo_a_padre);

	printf("Hola, soy PID %d:\n"
	       "  - primer pipe me devuelve: [%d, %d]\n"
	       "  - segundo pipe me devuelve: [%d, %d]\n\n",
	       getpid(),
	       padre_a_hijo[0],
	       padre_a_hijo[1],
	       hijo_a_padre[0],
	       hijo_a_padre[1]);

	pid_t f = fork();
	verificar_error(f, "Error en el fork");

	if (f == 0)
		proceso_hijo(hijo_a_padre, padre_a_hijo);
	else
		proceso_padre(padre_a_hijo, hijo_a_padre, f);

	exit(0);
}