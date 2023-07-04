#include "runcmd.h"

#define HISTFILE "HISTFILE"
#define HOME "HOME"
#define FISOP_HISTORY "/.fisop_history"

int status = 0;
struct cmd *parsed_pipe;


void guardar_comando_en_historial(char *cmd);

void
guardar_comando_en_historial(char *cmd)
{
	char *path_historial = getenv(HISTFILE);

	if (path_historial == NULL) {
		char *home = getenv(HOME);
		if (home == NULL) {
			perror("Error al obtener el directorio de inicio");
			return;
		}
		size_t tam_home = strlen(home);
		size_t tam_path = tam_home + strlen(FISOP_HISTORY) + 1;
		path_historial = malloc(tam_path);
		if (path_historial == NULL) {
			perror("Error al reservar memoria para el archivo de "
			       "historial");
			return;
		}
		strcpy(path_historial, home);
		strncat(path_historial, FISOP_HISTORY, tam_path + 1);
		path_historial[tam_path - 1] = '\0';
	}

	FILE *historial = fopen(path_historial, "a");
	if (historial == NULL) {
		perror("Error al abrir el archivo de historial");
		free(path_historial);
		return;
	}

	fprintf(historial, "%s\n", cmd);
	fclose(historial);
	free(path_historial);
}

// runs the command in 'cmd'
int
run_cmd(char *cmd)
{
	pid_t p;
	struct cmd *parsed;

	// if the "enter" key is pressed
	// just print the prompt again
	if (cmd[0] == END_STRING)
		return 0;

#ifndef SHELL_NO_INTERACTIVE
	guardar_comando_en_historial(cmd);
#endif

	// "history" built-in call
	if (history(cmd))
		return 0;

	// "cd" built-in call
	if (cd(cmd))
		return 0;

	// "exit" built-in call
	if (exit_shell(cmd))
		return EXIT_SHELL;

	// "pwd" built-in call
	if (pwd(cmd))
		return 0;

	// parses the command line
	parsed = parse_line(cmd);

	// forks and run the command
	if ((p = fork()) == 0) {
		// keep a reference
		// to the parsed pipe cmd
		// so it can be freed later
		if (parsed->type == PIPE)
			parsed_pipe = parsed;

		exec_cmd(parsed);
	}

	// stores the pid of the process
	parsed->pid = p;

	// background process special treatment
	// Hint:
	// - check if the process is
	//		going to be run in the 'back'
	// - print info about it with
	// 	'print_back_info()'
	//
	if (parsed->type == BACK) {
		print_back_info(parsed);
		waitpid(p, &status, WNOHANG);
	} else {
		// waits for the process to finish
		waitpid(p, &status, 0);
		print_status_info(parsed);
	}

	free_command(parsed);

	return 0;
}
