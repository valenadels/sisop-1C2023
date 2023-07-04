#include <string.h>
#include "defs.h"
#include "history.h"
#define TRUE 1
#define FALSE 0
#define HISTFILE "HISTFILE"
#define HOME "HOME"
#define FISOP_HISTORY "/.fisop_history"

char *obtener_path(void);

char *
obtener_path()
{
	char *home = getenv(HOME);
	if (home == NULL) {
		perror("Error al obtener el directorio de inicio");
		return NULL;
	}

	size_t tam_home = strlen(home);
	size_t tam_path = tam_home + strlen(FISOP_HISTORY) + 1;
	char *path_historial = malloc(tam_path);
	if (path_historial == NULL) {
		perror("Error al reservar memoria para el archivo de "
		       "historial");
		return NULL;
	}

	strcpy(path_historial, home);
	strncat(path_historial, FISOP_HISTORY, tam_path + 1);
	path_historial[tam_path - 1] = '\0';
	return path_historial;
}

char **
obtener_historial(char *path_historial, int *comandos_totales)
{
	FILE *historial = fopen(path_historial, "r");
	if (!historial) {
		perror("No se pudo abrir el archivo de historial\n");
		return NULL;
	}

	char *linea = NULL;
	size_t tam_linea = 0;
	char **comandos = NULL;
	*comandos_totales = 0;

	while ((getline(&linea, &tam_linea, historial)) != -1) {
		comandos = realloc(comandos,
		                   (*comandos_totales + 1) * sizeof(char *));
		if (!comandos) {
			perror("Error al utilizar realloc");
			free(linea);
			for (int i = 0; i < *comandos_totales; i++)
				free(comandos[i]);
			free(comandos);
			fclose(historial);
			return NULL;
		}

		comandos[*comandos_totales] = strdup(linea);
		if (!comandos[*comandos_totales]) {
			perror("Error al utilizar strdup\n");
			free(linea);
			for (int i = 0; i < *comandos_totales; i++)
				free(comandos[i]);
			free(comandos);
			fclose(historial);
			return NULL;
		}
		(*comandos_totales)++;
		tam_linea = 0;
		free(linea);
		linea = NULL;
	}

	free(linea);
	fclose(historial);
	return comandos;
}