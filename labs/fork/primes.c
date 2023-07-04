#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/wait.h>
#include <stdbool.h>

#define ERROR_STATUS -1
#define _EOF 0

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

int
_fork()
{
	pid_t i = fork();
	verificar_error(i, "Error al hacer fork");
	return i;
}

void
_write(int fd, void *buffer, size_t tamanio)
{
	if (buffer)
		verificar_error(write(fd, buffer, tamanio), "Error al escribir");
}

void
primer_secuencia(int i, int n, int fd)
{
	if (i > n)
		return;

	_write(fd, &i, sizeof(int));
	primer_secuencia(i + 1, n, fd);
}

bool
es_multiplo(int actual, int primo)
{
	return actual % primo == 0;
}

void
siguiente_filtro(int pipe_izq[2])
{
	int primo;
	int r = read(pipe_izq[0], &primo, sizeof(int));
	verificar_error(r, "Error al leer primo");

	if (r == _EOF) {
		close(pipe_izq[0]);
		return;
	}

	printf("Primo %d\n", primo);

	int pipe_derecho[2];
	_pipe(pipe_derecho);

	int i = _fork();

	if (i == 0) {
		close(pipe_derecho[1]);
		close(pipe_izq[0]);
		siguiente_filtro(pipe_derecho);
		close(pipe_izq[1]);
	} else {
		int j;
		int r = read(pipe_izq[0], &j, sizeof(int));
		verificar_error(r, "Error al leer");

		while (r > _EOF) {
			if (!es_multiplo(j, primo))
				_write(pipe_derecho[1], &j, sizeof(int));
			r = read(pipe_izq[0], &j, sizeof(int));
			verificar_error(r, "Error al leer");
		}

		close(pipe_izq[0]);
		close(pipe_derecho[1]);
		close(pipe_derecho[0]);
		verificar_error(wait(NULL), "Error al esperar");
	}
	return;
}

int
main(int argc, char *argv[])
{
	if (argc < 2)
		exit(-1);

	int n = atoi(argv[1]);

	int fds[2];
	_pipe(fds);

	pid_t i = _fork();

	if (i == 0) {
		close(fds[1]);
		siguiente_filtro(fds);
	} else {
		primer_secuencia(2, n, fds[1]);
		close(fds[1]);
		close(fds[0]);
		verificar_error(wait(NULL), "Error al esperar");
	}

	exit(0);
}