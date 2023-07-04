#include "defs.h"
#include "readline.h"
#include "history.h"
#include "set_input_mode.h"
#include <ctype.h>
#include <sys/ioctl.h>

#define NEW_LINE_CHAR '\n'
#define DEL_CHAR 127
#define ESCAPE_CHAR '\033'
#define EOF_CHAR '\004'
#define BACKSPACE_CHAR '\b'
#define HISTFILE "HISTFILE"
#define HOME "HOME"
#define FISOP_HISTORY "/.fisop_history"

static char buffer[BUFLEN];
static int history_index = 0;

char *read_noncanonical_mode(void);
char *read_canonical_mode(void);
void handle_new_line(int tope, char c);
void handle_backspace(int *tope, int *len_actual);
void handle_escape_input(char c,
                         int *tope,
                         int *len_actual,
                         char **comandos,
                         int comandos_totales_historial);
void escribir_buffer(int *tope, int *len_actual, char c);
void liberar_comandos(char **comandos, int comandos_totales_historial);

void
escribir_buffer(int *tope, int *len_actual, char c)
{
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	int cols = w.ws_col;  // obtengo la cantidad de caracteres máximos que puede tener una linea en mi terminal

	int i = write(STDOUT_FILENO, &c, 1);
	buffer[(*tope)++] = c;
	if (i < 0) {
		perror("Falló la escritura");
		return;
	}
	(*len_actual)++;
	if ((*len_actual) ==
	    cols - 1) {  // si la cantidad de caracteres escritos llego al "tope"
		         // de caracteres permitidos, tengo que saltar a la proxima linea
		int j = write(STDOUT_FILENO, "\n", 1);
		if (j < 0) {
			perror("Falló la escritura");
			return;
		}
		(*len_actual) = 0;  // en mi nueva linea hay 0 caracteres escritos
	}
}

void
handle_new_line(int tope, char c)
{
	buffer[(tope)++] = END_STRING;
	int i = write(STDOUT_FILENO, &c, 1);
	if (i < 0)
		perror("Falló la escritura");
}

void
handle_backspace(int *tope, int *len_actual)
{
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	int cols = w.ws_col;  // obtengo la cantidad de caracteres máximos que puede tener una linea en mi terminal

	if (*len_actual == 0 && *tope) {
		char linea[10];
		sprintf(linea, "\033[%dC", cols);
		int linea_anterior =
		        write(STDOUT_FILENO,
		              "\033[1A",
		              sizeof("\033[1A"));  // subo a la linea anterior
		int final_linea =
		        write(STDOUT_FILENO,
		              linea,
		              strlen(linea));  // me muevo al final de la linea
		if (linea_anterior < 0 || final_linea < 0) {
			perror("Falló la escritura");
		}
		*len_actual =
		        cols -
		        1;  // la longitud actual (caracteres escritos en mi linea)
		            // es igual a la cantidad de caracteres que entran
		            // en mi terminal (cols) menos 1 (porque borre el ultimo)
	}
	if (*tope) {
		buffer[*tope - 1] = '\0';
		(*tope)--;
		(*len_actual)--;
		int i = write(STDOUT_FILENO, "\b \b", 3);
		if (i < 0)
			perror("Falló la escritura\n");
	}
}

void
handle_escape_input(char c,
                    int *tope,
                    int *len_actual,
                    char **comandos,
                    int comandos_totales_historial)
{
	if (read(STDIN_FILENO, &c, 1) < 0) {
		perror("Falló la lectura en read_line\n");
		return;
	}

	if (c == '[') {
		if (read(STDIN_FILENO, &c, 1) < 0) {
			perror("Falló la lectura\n");
			return;
		}

		if (c == 'A') {
			if (history_index > 0) {
				int tope_aux = *tope;
				for (int i = 0; i < tope_aux; i++) {
					handle_backspace(tope, len_actual);
				}
				char *comando = comandos[--history_index];

				if (strlen(comando) == 0)
					return;

				char *comando_aux = strdup(comando);
				comando_aux[strlen(comando) - 1] = '\0';

				int tope_nuevo = strlen(comando_aux);
				for (int i = 0; i < tope_nuevo; i++) {
					escribir_buffer(tope,
					                len_actual,
					                comando_aux[i]);
				}
				free(comando_aux);
			}
		} else if (c == 'B') {
			if (history_index == comandos_totales_historial) {
				int tope_aux = *tope;
				for (int i = 0; i < tope_aux; i++) {
					handle_backspace(tope, len_actual);
				}

			} else if (history_index < comandos_totales_historial) {
				int tope_aux = *tope;
				for (int i = 0; i < tope_aux; i++) {
					handle_backspace(tope, len_actual);
				}

				if (history_index <
				    comandos_totales_historial - 1) {
					char *comando = comandos[++history_index];

					if (strlen(comando) == 0)
						return;

					char *comando_aux = strdup(comando);
					comando_aux[strlen(comando) - 1] = '\0';

					int tope_nuevo = strlen(comando_aux);
					for (int i = 0; i < tope_nuevo; i++) {
						escribir_buffer(tope,
						                len_actual,
						                comando_aux[i]);
					}
					free(comando_aux);
				} else {
					history_index++;
				}
			}
		}
	}
}

void
liberar_comandos(char **comandos, int comandos_totales_historial)
{
	for (int i = 0; i < comandos_totales_historial; i++)
		free(comandos[i]);
	free(comandos);
}


char *
read_noncanonical_mode()
{
	memset(buffer, 0, BUFLEN);
	int c = 0;
	int tope = 0;
	char *path_historial = getenv(HISTFILE);
	if (!path_historial) {
		path_historial = obtener_path();
		if (!path_historial) {
			perror("No se pudo abrir el archivo de "
			       "historial\n");
			return NULL;
		};
	}
	int comandos_totales_historial = 0;
	char **comandos =
	        obtener_historial(path_historial, &comandos_totales_historial);
	free(path_historial);

	history_index = comandos_totales_historial;

	int len_actual =
	        2;  // inicialmente la cantidad de caracteres escritos en la linea es 2 por el "$ "

	set_input_mode();

	while (true) {
		int i = read(STDIN_FILENO, &c, 1);
		if (i < 0) {
			perror("Falló la lectura en read_line\n");
			return NULL;
		}

		if (c == NEW_LINE_CHAR) {
			handle_new_line(tope, c);
			break;
		}
		if (c == EOF_CHAR) {
			liberar_comandos(comandos, comandos_totales_historial);
			return NULL;
		}
		if (c == DEL_CHAR) {
			handle_backspace(&tope, &len_actual);
			continue;
		}
		if (c == ESCAPE_CHAR) {
			handle_escape_input(c,
			                    &tope,
			                    &len_actual,
			                    comandos,
			                    comandos_totales_historial);
			continue;
		}

		if (isprint(c)) {
			escribir_buffer(&tope, &len_actual, c);
		}
	}

	reset_input_mode();
	liberar_comandos(comandos, comandos_totales_historial);

	return buffer;
}

char *
read_canonical_mode()
{
	memset(buffer, 0, BUFLEN);
	int c = 0;
	c = getchar();
	int i = 0;
	while (c != END_LINE && c != EOF) {
		buffer[i++] = c;
		c = getchar();
	}

	// if the user press ctrl+D
	// just exit normally
	if (c == EOF)
		return NULL;

	buffer[i] = END_STRING;
	return buffer;
}

// reads a line from the standard input
// and prints the prompt
char *
read_line(const char *prompt)
{
#ifndef SHELL_NO_INTERACTIVE
	fprintf(stdout, "%s %s %s\n", COLOR_RED, prompt, COLOR_RESET);
	fprintf(stdout, "%s", "$ ");
	fflush(stdout);
#endif

	if (isatty(STDIN_FILENO))
		return read_noncanonical_mode();
	else
		return read_canonical_mode();
}