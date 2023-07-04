#define _GNU_SOURCE

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <unistd.h>

#define ERROR_STATUS -1

void
verificar_error(void *i, char *mensaje)
{
	if (!i) {
		fprintf(stderr, "%s\n", mensaje);
		fprintf(stderr, "Errno: %d\n\n", errno);
		exit(ERROR_STATUS);
	}
}

bool
no_es_accesible(char *nombre_subdirectorio)
{
	return strcmp(nombre_subdirectorio, "..") == 0 ||
	       strcmp(nombre_subdirectorio, ".") == 0;
}

void
find(DIR *directorio,
     char *substring,
     char *path,
     char *(*strstr_)(const char *, const char *) )
{
	errno = 0;
	struct dirent *entry = readdir(directorio);

	if (!entry) {
		if (errno != 0)
			printf("Error al leer entry del directorio. Errno: "
			       "%d\n",
			       errno);
		return;
	}

	if (entry->d_type == DT_DIR) {
		int dfd = dirfd(directorio);
		if (dfd == ERROR_STATUS) {
			printf("Error al obtener fd del directorio. Errno: "
			       "%d\n",
			       errno);
			exit(ERROR_STATUS);
		}

		char *nombre_subdirectorio = entry->d_name;
		if (no_es_accesible(nombre_subdirectorio)) {
			find(directorio, substring, path, strstr_);
			return;
		}

		char new_path[PATH_MAX];
		snprintf(new_path, PATH_MAX, "%s%s", path, nombre_subdirectorio);

		int sdfd = openat(dfd, nombre_subdirectorio, O_DIRECTORY);
		if (sdfd == ERROR_STATUS) {
			printf("Error al obtener fd del subdirectorio. Errno: "
			       "%d\n",
			       errno);
			exit(ERROR_STATUS);
		}

		DIR *subdirectorio = fdopendir(sdfd);
		verificar_error(subdirectorio, "Error al abrir subdirectorio");

		if (strstr_(nombre_subdirectorio, substring))
			printf("%s\n", new_path);

		strcat(new_path, "/");

		find(subdirectorio, substring, new_path, strstr_);

		close(sdfd);
		closedir(subdirectorio);

	} else if (entry->d_type == DT_REG) {
		char *nombre_archivo = entry->d_name;

		if (strstr_(nombre_archivo, substring)) {
			char new_path[PATH_MAX];
			snprintf(new_path, PATH_MAX, "%s%s", path, nombre_archivo);
			printf("%s\n", new_path);
		}
	}

	find(directorio, substring, path, strstr_);
}

int
main(int argc, char *argv[])
{
	DIR *directorio = opendir(".");
	verificar_error(directorio, "Error al abrir directorio");

	if (argc == 2)
		find(directorio, argv[1], "", strstr);
	else if (argc == 3)
		find(directorio, argv[2], "", strcasestr);
	else {
		closedir(directorio);
		exit(ERROR_STATUS);
	}

	closedir(directorio);
	exit(0);
}