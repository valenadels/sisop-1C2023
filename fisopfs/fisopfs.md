# fisop-fs

## Decisiones de diseño

A continuación se detallan las decisiones de diseño tomadas para la implementación del sistema de archivos.

### Estructuras en memoria

En terminos generales, utilizamos un sistema de archivos basado en inodos, donde cada archivo/directorio es representado por
uno de ellos. Además, contamos con un super bloque que contiene información general del sistema de archivos:

- Inodos
- Bitmap de inodos (para saber cuales están libres y cuales no)

```c
struct super_block {
	struct inode inodes[MAX_INODES];
	int bitmap_inodes[MAX_INODES];  // 0 = libre, 1 = ocupado
};
```
https://github.com/fiubatps/sisop_2023a_g04/blob/entrega_filesystem/fisopfs/diagrama.png


#### Inodo

El inodo es una estructura que contiene lo que vendría a ser la metadata de un archivo/directorio. En nuestro caso, cada inodo
contiene la siguiente información relevante:

- Tipo de archivo (archivo regular o directorio)
- Tamaño
- Contenido
- Path y path del padre que lo contiene
- Tiempo de modificación / acceso

Decidimos poner que el tamaño de un directorio es 0 por simplicidad.

En cuanto a tiempo de creación no fue implementado, ya que en clase se nos dijo que no era necesario.

La cantidad maxima de inodos que puede tener el sistema de archivos es de `MAX_INODES 80` y el máximo tamaño de un archivo es de `MAX_CONTENT 1024`.

#### Bitmap de inodos

El bitmap de inodos es una estructura que nos permite saber cuales inodos están libres y cuales no. En nuestro caso, cada bit representa un inodo, donde un 0 significa que el inodo está libre y un 1 que está ocupado. La cantidad de bits que tiene el bitmap es igual a la cantidad de inodos que puede tener el sistema de archivos.

### Busqueda de archivos dado un path 

Para obtener el archivo deseado dado un path, se implemento la función `get_inode_index(char* path)` la cual busca en el super bloque el inodo que corresponde al path pasado por parámetro. Para esto, se recorren todos los inodos que se encuentran en memoria, comparando contra el path pasado por parámetro. En caso de encontrar el inodo, se devuelve su índice en el super bloque. En caso contrario, se devuelve `-1`.

En esta función se llama a la función `remove_slash(char* path)` la cual se encarga de remover el slash final del path, en caso de que exista. Esto es necesario para poder comparar el path pasado por parámetro con el path de los inodos.

### Persistencia del sistema de archivos en disco

Una vez iniciado el sistema de archivos, este se mantiene en memoria hasta que se ejecute el comando `.destroy`. En este momento, se persiste el sistema de archivos en disco, guardando el bitmap de inodos y los inodos en un archivo llamado `fs.fisopfs`. En este archivo, se escribe el super bloque, el cual contiene los inodos del sistema y el bitmap de inodos libres.

Al momento de iniciar el sistema de archivos (con `fisop init`), se lee el archivo `fs.fisopfs` y se carga el super bloque en memoria. De esta forma, quedan cargados los inodos y el bitmap de inodos libres en memoria, listos para ser utilizados.


## Pruebas

### Listar archivos en un directorio creado, lectura de archivo y remover directorio con contenido

https://github.com/fiubatps/sisop_2023a_g04/blob/entrega_filesystem/fisopfs/pruebas_tp/rmdir_con_contenido.png

### Creación de directorio, listar directorio vacío, remover directorio sin contenido

https://github.com/fiubatps/sisop_2023a_g04/blob/entrega_filesystem/fisopfs/pruebas_tp/rmdir_sin_contenido.png

### Creación de archivo, escritura y append del archivo, lectura

https://github.com/fiubatps/sisop_2023a_g04/blob/entrega_filesystem/fisopfs/pruebas_tp/creacion_archivo_y_append.png

### Stats de un archivo

https://github.com/fiubatps/sisop_2023a_g04/blob/entrega_filesystem/fisopfs/pruebas_tp/stat.png

### Remover un archivo

https://github.com/fiubatps/sisop_2023a_g04/blob/entrega_filesystem/fisopfs/pruebas_tp/rm.png

### Creación de directorio y creación de archivo

https://github.com/fiubatps/sisop_2023a_g04/blob/entrega_filesystem/fisopfs/pruebas_tp/creacion_dir_y_archivo.png 

### Creación de archivo en un directorio

https://github.com/fiubatps/sisop_2023a_g04/blob/entrega_filesystem/fisopfs/pruebas_tp/creacion_archivo_en_dir.png

### Directorios especiales . y ..

https://github.com/fiubatps/sisop_2023a_g04/blob/entrega_filesystem/fisopfs/pruebas_tp/directorios_especiales.png

### More y less para archivos

https://github.com/fiubatps/sisop_2023a_g04/blob/entrega_filesystem/fisopfs/pruebas_tp/more.png

https://github.com/fiubatps/sisop_2023a_g04/blob/entrega_filesystem/fisopfs/pruebas_tp/less.png

### Sobreescritura de archivo

https://github.com/fiubatps/sisop_2023a_g04/blob/entrega_filesystem/fisopfs/pruebas_tp/sobreescribo.png

### Persistencia

https://github.com/fiubatps/sisop_2023a_g04/blob/entrega_filesystem/fisopfs/pruebas_tp/persistencia.png





