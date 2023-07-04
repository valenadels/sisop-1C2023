#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/wait.h>
#include <stdbool.h>

#ifndef NARGS
#define NARGS 4
#endif

#define ERROR_STATUS -1
#define SALTO_DE_LINEA '\n'
#define FIN_STRING '\0'
#define MAX_ARGS 6

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
_fork()
{
	pid_t i = fork();
	verificar_error(i, "Error al hacer fork");
	return i;
}

bool
seguir_leyendo(int leidos, bool termino)
{
	return leidos < NARGS && !termino;
}

void
liberar_memoria(char **args)
{
	for (int i = 1; args[i]; i++)
		free(args[i]);
}

void
xargs(char *comando)
{
	int leidos = 0;
	bool termino = false;
	char *args[MAX_ARGS];
	args[0] = comando;

	while (seguir_leyendo(leidos, termino)) {
		char *archivo = NULL;
		size_t len = 0;
		ssize_t cant_leido = 0;
		cant_leido = getline(&archivo, &len, stdin);
		if (cant_leido != ERROR_STATUS) {
			if (archivo[cant_leido - 1] == SALTO_DE_LINEA) {
				archivo[cant_leido - 1] = FIN_STRING;
			}
			leidos++;
			args[leidos] = archivo;

		} else {
			termino = true;
			free(archivo);
			if (leidos == 0)
				return;
		}
	}

	args[leidos + 1] = NULL;
	int i = _fork();
	if (i == 0)
		execvp(comando, args);
	else {
		wait(NULL);
		xargs(comando);
		liberar_memoria(args);
	}
}

int
main(int argc, char *argv[])
{
	if (argc < 2) {
		fprintf(stderr, "Se debe proporcionar el comando");
		return ERROR_STATUS;
	}

	char *comando = argv[1];
	xargs(comando);

	exit(0);
}