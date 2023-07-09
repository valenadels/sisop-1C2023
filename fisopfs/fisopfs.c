#define FUSE_USE_VERSION 30
#define DIR -1
#define FILE_T 1
#define MAX_CONTENT 1024
#define MAX_DIRECTORY_SIZE 1024
#define MAX_INODES 80
#define MAX_PATH 200
#define FREE 0
#define OCCUPIED 1
#define ROOT_PATH "/"

#include <fuse.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>


struct inode {
	int type;                  // -1 = directorio, 1 = archivo
	mode_t mode;               // permissions
	size_t size;               // size of the file
	uid_t uid;                 // user id
	gid_t gid;                 // group id
	time_t last_access;        // last access time
	time_t last_modification;  // last modification time
	time_t creation_time;      // creation time
	char path[MAX_PATH];
	char content[MAX_CONTENT];      // content of the file
	char directory_path[MAX_PATH];  // path of the directory that contais
	                                // the file. if it's a directory, it's
	                                // the path of the parent directory if
	                                // it's the root, it's an empty string
};

struct super_block {
	struct inode inodes[MAX_INODES];
	int bitmap_inodes[MAX_INODES];  // 0 = libre, 1 = ocupado
};

struct super_block super = {};

// Donde se va a guardar el fs
char fs_file[MAX_PATH] = "fs.fisopfs";

// Remueve el slash del path pasado y devuelve unicamente el nombre del archivo
// o directorio
char *
remove_slash(const char *path)
{
	size_t len = strlen(path);
	char *path_without_root_slash = malloc(len);
	if (!path_without_root_slash)
		return NULL;

	memcpy(path_without_root_slash, path + 1, len - 1);
	path_without_root_slash[len - 1] = '\0';

	const char *last_slash = strrchr(path, '/');
	if (last_slash == NULL)
		return path_without_root_slash;

	size_t absolute_len = strlen(last_slash + 1);
	char *absolute_path = malloc(absolute_len + 1);
	if (!absolute_path) {
		free(path_without_root_slash);
		return NULL;
	}

	memcpy(absolute_path, last_slash + 1, absolute_len);
	absolute_path[absolute_len] = '\0';

	free(path_without_root_slash);

	return absolute_path;
}

// Devuelve el index del inodo que tiene el path pasado
// -1 si no existe
int
get_inode_index(const char *path)
{
	if (strcmp(path, ROOT_PATH) == 0)
		return 0;
	char *path_without_root_slash = remove_slash(path);
	if (!path_without_root_slash)
		return -1;
	for (int i = 0; i < MAX_INODES; i++) {
		if (strcmp(path_without_root_slash, super.inodes[i].path) == 0) {
			return i;
		}
	}
	free(path_without_root_slash);
	return -1;
}

int
fisopfs_utimens(const char *path, const struct timespec tv[2])
{
	int index_inodo = get_inode_index(path);
	if (index_inodo == -1) {
		fprintf(stderr, "[debug] Error utimens: %s\n", strerror(errno));
		errno = ENOENT;
		return -ENOENT;
	}
	struct inode *inode = &super.inodes[index_inodo];

	inode->last_access = tv[0].tv_sec;
	inode->last_modification = tv[1].tv_sec;
	return 0;
}


// Devuelve el index del proximo inodo libre
// -ENOSPC si no hay mas espacio
// -EEXIST si ya existe un inodo con ese path
int
next_free_inode_index(const char *path)
{
	bool exists = false;
	int next_free_inode = -ENOSPC;
	for (int i = 0; i < MAX_INODES && !exists; i++) {
		if (super.bitmap_inodes[i] == FREE &&
		    next_free_inode <
		            0) {  // me quedo con el index del primero libre
			next_free_inode = i;
		}
		if (strcmp(super.inodes[i].path, path) == 0)
			exists = true;
	}
	if (exists) {
		fprintf(stderr,
		        "[debug] Error next_free_inode_index: %s\n",
		        strerror(errno));
		errno = EEXIST;
		return -EEXIST;
	} else {
		return next_free_inode;
	}
}

// Modifica el path del inodo pasado, cambiando el '/' por '\0'
void
get_parent_path(char *parent_path)
{
	char *last_slash = strrchr(parent_path, '/');

	if (last_slash != NULL)
		*last_slash = '\0';
	else
		parent_path[0] = '\0';
}

// Crea un nuevo inodo con el path, mode y type pasado. Lo guarda en el
// superblock y devuelve el index del inodo creado.
// En caso de algun error devuelve:
// -ENAMETOOLONG si el path es demasiado largo
// -ENOSPC si no hay mas espacio
// -EEXIST si ya existe un inodo con ese path
int
new_inode(const char *path, mode_t mode, int type)
{
	if (strlen(path) - 1 > MAX_CONTENT) {
		fprintf(stderr, "[debug] Error new_inode: %s\n", strerror(errno));
		errno = ENAMETOOLONG;
		return -ENAMETOOLONG;
	}
	char *absolute_path = remove_slash(path);
	if (!absolute_path)
		return -1;
	int i = next_free_inode_index(absolute_path);
	if (i < 0)
		return i;

	struct inode new_inode;
	new_inode.type = type;
	new_inode.mode = mode;
	new_inode.size = 0;  // arrancan vacios
	new_inode.uid = getuid();
	new_inode.gid = getgid();
	new_inode.last_access = time(NULL);
	new_inode.last_modification = time(NULL);
	strcpy(new_inode.path, absolute_path);

	if (type == FILE_T) {
		char parent_path[MAX_PATH];
		memcpy(parent_path, path + 1, strlen(path) - 1);
		parent_path[strlen(path) - 1] = '\0';

		get_parent_path(parent_path);

		if (strlen(parent_path) == 0)
			strcpy(parent_path, ROOT_PATH);

		strcpy(new_inode.directory_path, parent_path);

	} else
		strcpy(new_inode.directory_path, ROOT_PATH);

	memset(new_inode.content, 0, sizeof(new_inode.content));
	super.inodes[i] = new_inode;
	super.bitmap_inodes[i] = OCCUPIED;
	free(absolute_path);

	return 0;
}


static int
fisopfs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_touch - path: %s\n", path);

	return new_inode(path, mode, FILE_T);
}


static int
fisopfs_getattr(const char *path, struct stat *st)
{
	printf("[debug] fisopfs_getattr - path: %s\n", path);

	int index_inodo = get_inode_index(path);
	if (index_inodo == -1) {
		fprintf(stderr, "[debug] Getattr: %s\n", strerror(errno));
		errno = ENOENT;
		return -ENOENT;
	}
	struct inode inode = super.inodes[index_inodo];
	st->st_dev = 0;
	st->st_ino = index_inodo;
	st->st_uid = inode.uid;
	st->st_mode = inode.mode;
	st->st_atime = inode.last_access;
	st->st_mtime = inode.last_modification;
	st->st_ctime = inode.creation_time;
	st->st_size = inode.size;
	st->st_gid = inode.gid;
	st->st_nlink = 2;
	st->st_mode = __S_IFDIR | 0755;
	if (inode.type == FILE_T) {
		st->st_mode = __S_IFREG | 0644;
		st->st_nlink = 1;
	}

	return 0;
}


static int
fisopfs_read(const char *path,
             char *buffer,
             size_t size,
             off_t offset,
             struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_read - path: %s, offset: %lu, size: %lu\n",
	       path,
	       offset,
	       size);

	if (offset < 0 || size < 0) {
		fprintf(stderr, "[debug] Error read: %s\n", strerror(errno));
		errno = EINVAL;
		return -EINVAL;
	}


	int index = get_inode_index(path);
	if (index == -1) {
		fprintf(stderr, "[debug] Error read: %s\n", strerror(errno));
		errno = ENOENT;
		return -ENOENT;
	}


	struct inode *inode = &super.inodes[index];
	if (inode->type == DIR) {
		fprintf(stderr, "[debug] Error read: %s\n", strerror(errno));
		errno = EISDIR;
		return -EISDIR;
	}


	char *content = inode->content;
	size_t size_file = inode->size;
	if (offset > size_file) {
		fprintf(stderr, "[debug] Error read: %s\n", strerror(errno));
		errno = EINVAL;
		return -EINVAL;
	}

	if (size_file - offset < size)
		size = size_file - offset;

	strncpy(buffer, content + offset, size);
	inode->last_access = time(NULL);

	return size;
}

static int
fisopfs_unlink(const char *path)
{
	printf("[debug] fisopfs_unlink(%s)\n", path);
	int index = get_inode_index(path);
	if (index == -1) {
		fprintf(stderr, "[debug] Error unlink: %s\n", strerror(errno));
		errno = ENOENT;
		return -ENOENT;
	}


	struct inode *inode = &super.inodes[index];

	if (inode->type == DIR) {
		fprintf(stderr, "[debug] Error unlink: %s\n", strerror(errno));
		errno = EISDIR;
		return -EISDIR;
	}


	super.bitmap_inodes[index] = FREE;
	memset(inode, 0, sizeof(struct inode));

	return 0;
}

int
fisopfs_write(const char *path,
              const char *data,
              size_t size_data,
              off_t offset,
              struct fuse_file_info *fuse_info)
{
	printf("[debug] fisops_write (%s) \n", path);
	size_t sum = offset + size_data;
	if (sum > MAX_CONTENT) {
		fprintf(stderr, "[debug] Error write: %s\n", strerror(errno));
		errno = EFBIG;
		return -EFBIG;
	}
	int inode_index = get_inode_index(path);
	if (inode_index < 0) {  // si no existe el archivo, lo creo
		int result = fisopfs_create(path, 33204, fuse_info);
		if (result < 0)
			return result;
		inode_index = get_inode_index(path);
	}
	if (inode_index < 0) {
		fprintf(stderr, "[debug] Error write: %s\n", strerror(errno));
		errno = ENOENT;
		return -ENOENT;
	}
	struct inode *inode = &super.inodes[inode_index];
	if (inode->size < offset) {
		fprintf(stderr, "[debug] Error write: %s\n", strerror(errno));
		errno = EINVAL;
		return -EINVAL;
	}

	if (inode->type != FILE_T) {
		fprintf(stderr, "[debug] Error write: %s\n", strerror(errno));
		errno = EACCES;
		return -EACCES;
	}

	strncpy(inode->content + offset, data, size_data);
	inode->last_access = time(NULL);
	inode->last_modification = time(NULL);
	inode->size = strlen(inode->content);
	inode->content[inode->size] = '\0';

	return (int) size_data;
}

static int
fisopfs_readdir(const char *path,
                void *buffer,
                fuse_fill_dir_t filler,
                off_t offset,
                struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_readdir(%s)\n", path);

	filler(buffer, ".", NULL, 0);
	filler(buffer, "..", NULL, 0);
	int pos = get_inode_index(path);
	if (pos == -1) {
		fprintf(stderr, "[debug] Error readdir: %s\n", strerror(errno));
		errno = ENOENT;
		return -ENOENT;
	}

	struct inode dir_inode = super.inodes[pos];

	if (dir_inode.type != DIR) {
		fprintf(stderr, "[debug] Error readdir: %s\n", strerror(errno));
		errno = ENOTDIR;
		return -ENOTDIR;
	}
	dir_inode.last_access = time(NULL);

	for (int i = 1; i < MAX_INODES; i++) {
		if (super.bitmap_inodes[i] == OCCUPIED) {
			if (strcmp(super.inodes[i].directory_path,
			           dir_inode.path) == 0) {
				filler(buffer, super.inodes[i].path, NULL, 0);
			}
		}
	}

	return 0;
}

static int
fisopfs_mkdir(const char *path, mode_t mode)
{
	printf("[debug] fisopfs_mkdir - path: %s\n", path);

	return new_inode(path, mode, DIR);
}

struct inode **
files_in_dir(const char *path_dir, int *nfiles)
{
	int tope = 0;
	struct inode **files = malloc(MAX_INODES * sizeof(struct inode *));
	char *path_without_root_slash = remove_slash(path_dir);
	if (!path_without_root_slash)
		return NULL;

	for (int i = 0; i < MAX_INODES; i++) {
		if (strcmp(super.inodes[i].directory_path,
		           path_without_root_slash) == 0) {
			files[tope++] = &super.inodes[i];
		}
	}
	free(path_without_root_slash);

	*nfiles = tope;
	return files;
}

static int
fisopfs_rmdir(const char *path)
{
	printf("[debug] fisopfs_rmdir - path: %s\n", path);

	int i = get_inode_index(path);
	if (i < 0)
		return i;

	struct inode *inode = &super.inodes[i];
	int nfiles = 0;
	struct inode **files = files_in_dir(path, &nfiles);
	if (!files) {
		fprintf(stderr,
		        "[debug] Error rmdir when allocating memory for path: "
		        "%s\n",
		        strerror(errno));
		errno = ENOMEM;
		return -ENOMEM;
	}
	free(files);
	if (inode->type != DIR) {
		fprintf(stderr, "[debug] Error rmdir: %s\n", strerror(errno));
		errno = ENOTDIR;
		return -ENOTDIR;
	}

	if (nfiles > 0) {
		fprintf(stderr, "[debug] Error rmdir: %s\n", strerror(errno));
		errno = ENOTEMPTY;
		return -ENOTEMPTY;
	}
	super.bitmap_inodes[i] = FREE;
	memset(inode, 0, sizeof(struct inode));
	return 0;
}


static int
fisopfs_mknod(const char *path, mode_t mode, dev_t rdev)
{
	printf("[debug] fisopfs_mknod - path: %s\n", path);

	return 0;
}

// Inicializa el sistema de archivos cuando no existe el archivo fs.fisopfs en
// el cual se guardan los datos del sistema de archivos. Crea dicho archivo e
// inicializa el superbloque y el directorio raiz.
int
initialize_filesystem()
{
	memset(super.inodes, 0, sizeof(super.inodes));
	memset(super.bitmap_inodes, 0, sizeof(super.bitmap_inodes));

	struct inode *root_dir = &super.inodes[0];
	root_dir->type = DIR;
	root_dir->mode = __S_IFDIR | 0755;
	root_dir->size = MAX_DIRECTORY_SIZE;
	root_dir->uid = 1717;
	root_dir->gid = getgid();
	root_dir->last_access = time(NULL);
	root_dir->last_modification = time(NULL);
	root_dir->creation_time = time(NULL);
	strcpy(root_dir->path, ROOT_PATH);
	memset(root_dir->content, 0, sizeof(root_dir->content));
	strcpy(root_dir->directory_path, "");
	super.bitmap_inodes[0] = OCCUPIED;
	return 0;
}

void *
fisopfs_init(struct fuse_conn_info *conn)
{
	printf("[debug] fisop_init\n");

	FILE *file = fopen(fs_file, "r");
	if (!file) {
		initialize_filesystem();
	} else {
		int i = fread(&super, sizeof(super), 1, file);
		if (i != 1) {
			fprintf(stderr,
			        "[debug] Error init: %s\n",
			        strerror(errno));
			return NULL;
		}
		fclose(file);
	}
	return 0;
}

void
fisopfs_destroy(void *private_data)
{
	printf("[debug] fisop_destroy\n");
	FILE *file = fopen(fs_file, "w");
	if (!file) {
		fprintf(stderr,
		        "[debug] Error saving filesystem: %s\n",
		        strerror(errno));
	}
	fwrite(&super, sizeof(super), 1, file);
	fflush(file);
	fclose(file);
}

static int
fisopfs_truncate(const char *path, off_t size)
{
	printf("[debug] fisopfs_truncate - path: %s\n", path);
	if (size > MAX_CONTENT) {
		fprintf(stderr, "[debug] Error truncate: %s\n", strerror(errno));
		errno = EINVAL;
		return -EINVAL;
	}

	int i = get_inode_index(path);
	if (i < 0) {
		fprintf(stderr, "[debug] Error truncate: %s\n", strerror(errno));
		errno = ENOENT;
		return -ENOENT;
	}

	struct inode *inode = &super.inodes[i];
	inode->size = size;
	inode->last_modification = time(NULL);
	return 0;
}

static void
fisopfs_flush(const char *path, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_flush(%s)\n", path);
	return fisopfs_destroy(NULL);
}


static struct fuse_operations operations = {
	.getattr = fisopfs_getattr,
	.readdir = fisopfs_readdir,
	.read = fisopfs_read,
	.mkdir = fisopfs_mkdir,
	.rmdir = fisopfs_rmdir,
	.unlink = fisopfs_unlink,
	.mknod = fisopfs_mknod,
	.init = fisopfs_init,
	.write = fisopfs_write,
	.destroy = fisopfs_destroy,
	.create = fisopfs_create,
	.utimens = fisopfs_utimens,
	.truncate = fisopfs_truncate,
};

int
main(int argc, char *argv[])
{
	// El ultimo argumento es el path del archivo del fs, si es que se pasa
	if (strcmp(argv[1], "-f") == 0) {
		if (argc == 4) {
			strcpy(fs_file, argv[3]);
			argv[3] = NULL;
			argc--;
		}
	} else {
		if (argc == 3) {
			strcpy(fs_file, argv[2]);
			argv[2] = NULL;
			argc--;
		}
	}

	return fuse_main(argc, argv, &operations, NULL);
}
