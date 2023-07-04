#include "exec.h"

// sets "key" with the key part of "arg"
// and null-terminates it
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  key = "KEY"
//
static void
get_environ_key(char *arg, char *key)
{
	int i;
	for (i = 0; arg[i] != '='; i++)
		key[i] = arg[i];

	key[i] = END_STRING;
}

// sets "value" with the value part of "arg"
// and null-terminates it
// "idx" should be the index in "arg" where "=" char
// resides
//
// Example:
//  - KEY=value
//  arg = ['K', 'E', 'Y', '=', 'v', 'a', 'l', 'u', 'e', '\0']
//  value = "value"
//
static void
get_environ_value(char *arg, char *value, int idx)
{
	size_t i, j;
	for (i = (idx + 1), j = 0; i < strlen(arg); i++, j++)
		value[j] = arg[i];

	value[j] = END_STRING;
}

// sets the environment variables received
// in the command line
//
// Hints:
// - use 'block_contains()' to
// 	get the index where the '=' is
// - 'get_environ_*()' can be useful here
static void
set_environ_vars(char **eargv, int eargc)
{
	for (int i = 0; i < eargc; i++) {
		char key[BUFLEN];
		char value[BUFLEN];
		int index = block_contains(eargv[i], '=');

		if (index >= 0) {
			get_environ_key(eargv[i], key);
			get_environ_value(eargv[i], value, index);
			setenv(key, value, 1);
		}
	}
}

// opens the file in which the stdin/stdout/stderr
// flow will be redirected, and returns
// the file descriptor
//
// Find out what permissions it needs.
// Does it have to be closed after the execve(2) call?
//
// Hints:
// - if O_CREAT is used, add S_IWUSR and S_IRUSR
// 	to make it a readable normal file
static int
open_redir_fd(char *file, int flags)
{
	int fd;
	if (flags & O_CREAT) {
		fd = open(file, flags, S_IWUSR | S_IRUSR);
	} else {
		fd = open(file, flags);
	}

	if (fd < 0) {
		perror("Error al abrir archivo a donde redireccionar\n");
		exit(-1);
	}

	return fd;
}

// executes a command - does not return
//
// Hint:
// - check how the 'cmd' structs are defined
// 	in types.h
// - casting could be a good option
void
exec_cmd(struct cmd *cmd)
{
	// To be used in the different cases
	struct execcmd *e;
	struct backcmd *b;
	struct execcmd *r;
	struct pipecmd *p;

	switch (cmd->type) {
	case EXEC: {
		// spawns a command
		e = (struct execcmd *) cmd;
		set_environ_vars(e->eargv, e->eargc);
		if (execvp(e->argv[0], e->argv) < 0) {
			perror("Error: Execvp falló\n");
			free_command(cmd);
			exit(-1);
		}
		break;
	}

	case BACK: {
		// runs a command in background
		b = (struct backcmd *) cmd;
		exec_cmd(b->c);
		break;
	}

	case REDIR: {
		// changes the input/output/stderr flow
		//
		// To check if a redirection has to be performed
		// verify if file name's length (in the execcmd struct)
		// is greater than zero
		//
		r = (struct execcmd *) cmd;
		if (strlen(r->in_file) > 0) {
			int fd = open_redir_fd(r->in_file, O_CLOEXEC | O_RDONLY);
			dup2(fd, STDIN_FILENO);
		}
		if (strlen(r->out_file) > 0) {
			int fd = open_redir_fd(r->out_file,
			                       O_CLOEXEC | O_CREAT | O_WRONLY |
			                               O_TRUNC);
			dup2(fd, STDOUT_FILENO);
		}
		if (strlen(r->err_file) > 0) {
			if (strcmp(r->err_file, "&1") == 0) {
				dup2(STDOUT_FILENO, STDERR_FILENO);
			} else {
				int fd = open_redir_fd(r->err_file,
				                       O_CLOEXEC | O_CREAT |
				                               O_WRONLY | O_TRUNC);
				dup2(fd, STDERR_FILENO);
			}
		}

		r->type = EXEC;
		exec_cmd((struct cmd *) r);

		break;
	}

	case PIPE: {
		// pipes two commands
		p = (struct pipecmd *) cmd;
		int fd[2];

		if (pipe(fd) < 0) {
			perror("Error en pipe\n");
			exit(-1);
		};

		int izq = fork();

		if (izq < 0) {
			perror("Error en fork izq");
			close(fd[READ]);
			close(fd[WRITE]);
			exit(-1);
		} else if (izq == 0) {
			free_command(p->rightcmd);
			close(fd[READ]);
			dup2(fd[WRITE], STDOUT_FILENO);
			close(fd[WRITE]);
			exec_cmd(p->leftcmd);
			exit(-1);
		}

		int der = fork();

		if (der < 0) {
			perror("Error en fork der");
			close(fd[READ]);
			close(fd[WRITE]);
			exit(-1);
		} else if (der == 0) {
			free_command(p->leftcmd);
			close(fd[WRITE]);
			dup2(fd[READ], STDIN_FILENO);
			close(fd[READ]);
			exec_cmd(p->rightcmd);
			exit(-1);
		}

		close(fd[READ]);
		close(fd[WRITE]);
		waitpid(izq, NULL, 0);
		waitpid(der, NULL, 0);
		free_command(cmd);
		exit(0);
	}
	}
}
