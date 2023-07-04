#include "builtin.h"
#include <string.h>
#include "history.h"
#define TRUE 1
#define FALSE 0
#define HISTFILE "HISTFILE"
#define HOME "HOME"
#define FISOP_HISTORY "/.fisop_history"

void liberar_memoria(char **comandos, int comandos_totales);

void mostrar_historial(char **comandos, char *argumento, int comandos_totales);


// returns true if the 'exit' call
// should be performed
//
// (It must not be called from here)
int
exit_shell(char *cmd)
{
	if (strcmp(cmd, "exit") == 0)
		return TRUE;
	return FALSE;
}

// returns true if "chdir" was performed
//  this means that if 'cmd' contains:
// 	1. $ cd directory (change to 'directory')
// 	2. $ cd (change to $HOME)
//  it has to be executed and then return true
//
//  Remember to update the 'prompt' with the
//  	new directory.
//
// Examples:
//  1. cmd = ['c','d', ' ', '/', 'b', 'i', 'n', '\0']
//  2. cmd = ['c','d', '\0']
int
cd(char *cmd)
{
	if (strstr(cmd, "cd")) {
		strtok(cmd, " ");
		char *argumento = strtok(NULL, " ");

		if (argumento == NULL) {
			char *home = getenv(HOME);

			if (chdir(home) < 0) {
				perror("No se pudo cambiar a HOME");
				return TRUE;
			}
			snprintf(prompt, sizeof prompt, "(%s)", home);
		} else {
			if (chdir(argumento) < 0) {
				perror("No se pudo cambiar a la dirección "
				       "solicitada\n");

				return TRUE;
			}
			char buffer[BUFLEN];
			snprintf(prompt,
			         sizeof prompt,
			         "(%s)",
			         getcwd(buffer, sizeof(buffer)));
		}
		return TRUE;
	}

	return FALSE;
}

// returns true if 'pwd' was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
pwd(char *cmd)
{
	if (strcmp("pwd", cmd) == 0) {
		char buffer[BUFLEN];
		char *pwd = getcwd(buffer, sizeof(buffer));
		if (!pwd) {
			perror("getcwd() error");
			return TRUE;
		}
		printf("%s\n", pwd);
		return TRUE;
	}
	return FALSE;
}

void
liberar_memoria(char **comandos, int comandos_totales)
{
	for (int i = 0; i < comandos_totales; i++)
		free(comandos[i]);

	free(comandos);
}

void
mostrar_historial(char **comandos, char *argumento, int comandos_totales)
{
	int n = comandos_totales;
	if (argumento) {
		n = atoi(argumento);
		if (n > comandos_totales) {
			n = comandos_totales;
		}
	}

	for (int i = comandos_totales - n; i < comandos_totales; i++) {
		printf("%s", comandos[i]);
	}
}

// returns true if `history` was invoked
// in the command line
//
// (It has to be executed here and then
// 	return true)
int
history(char *cmd)
{
	if (strstr(cmd, "history")) {
		strtok(cmd, " ");
		char *argumento = strtok(NULL, " ");
		char *path_historial = getenv(HISTFILE);
		if (!path_historial) {
			path_historial = obtener_path();
			if (!path_historial) {
				perror("No se pudo abrir el archivo de "
				       "historial");
				return TRUE;
			};
		}

		int comandos_totales = 0;
		char **comandos =
		        obtener_historial(path_historial, &comandos_totales);
		if (!comandos) {
			perror("No se pudo obtener el historial\n");
			return TRUE;
		}

		mostrar_historial(comandos, argumento, comandos_totales);
		liberar_memoria(comandos, comandos_totales);
		free(path_historial);
	} else {
		return FALSE;
	}

	return TRUE;
}
