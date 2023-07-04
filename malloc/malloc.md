# Malloc

Lugar para respuestas en prosa y documentación del TP.

## Implementación, decisiones de diseño y estructuras de datos

En el presente Trabajo Práctico se implementaron las funciones `malloc(size)`, `free(ptr)` y `realloc(ptr, size)` y `calloc(nmemb, size)`. Dichas funciones están 
relacionadas con el almacenamiento dinámico de memoria, y son utilizadas para la asignación y liberación de memoria en tiempo de ejecución.

Contamos con la lógica de 'bloques' y 'regiones' para la división de la memoria. Las regiones son las unidades mínimas de memoria que se pueden asignar, y los bloques son conjuntos de regiones que se encuentran contiguos en memoria. Existen tres tipos posibles de
tamaño de bloque:
       - <u> **BLOCK_MIN_SIZE**: 16384 
       - <u> **BLOCK_MEDIUM_SIZE**: 1048576
       - <u> **BLOCK_LARGE_SIZE**: 33554432

Para la administración de bloques contamos con tres arreglos `struct region *small_blocks[MAX_BLOCKS]`, `struct region *medium_blocks[MAX_BLOCKS]` y `struct region *large_blocks[MAX_BLOCKS]` que contienen bloques de tamaño chico, mediano y grande respectivamente. Dentro
de cada 'bloque' se encuentran las regiones que lo componen. La cantidad máxima de bloque de cada tipo es `MAX_BLOCKS`, que es 1000. Se decidió poner este número por defecto ya que a lo sumo se podrá reservar aproximadamente 33gb de memoria, la cual nos parece una cantidad considerablemente grande.

Al tener estos 3 arreglos, nos aseguramos que en cada uno de ellos, los bloques sean del mismo tamaño y las regiones podrán
tomar como máximo el tamaño de su bloque. Por lo tanto, al momento de realizar un malloc o realloc, si el tamaño pedido es mayor, 
por ejemplo, a `BLOCK_MIN_SIZE` debo buscar la región libre en el arreglo de bloques medianos o grandes. En consecuencia, se ahorra tiempo
de busqueda ya que no se va a estar intentando encontar regiones en un arreglo que no corresponde.
Para realizar la busqueda de regiones libres se implementaron los algoritmos de first-fit y best-fit. El algoritmo de first-fit busca la primera región libre que encuentra en el arreglo de bloques, mientras que el algoritmo de best-fit busca la región libre que tenga el tamaño más cercano al tamaño pedido.

En lugar de crear un `struct bloque` decidimos manejarnos siempre con el mismo `struct region` porque nos permite tener un código más limpio y
simple y además vimos que no era necesario. Lo que vendría a ser la 'logica del manejo de bloques' se puede llevar a cabo con un `struct region` sin problemas, ya que se podría
pensar a un bloque como una región de cierto tamaño que contiene sub regiones en su interior.

Las variables `int amount_small_blocks`, `int amount_medium_blocks` y `int amount_large_blocks` contienen la cantidad de bloques de cada tipo que se encuentran en los arreglos. Estas variables se utilizan para poder realizar los tests.


## Funciones principales

### malloc(size)
La función `malloc(size)` se encarga de reservar un bloque de memoria de tamaño `size` y devolver un puntero a la dirección de memoria
 
Primero, se comprueba si el tamaño solicitado es mayor que el tamaño máximo para un bloque de memoria (BLOCK_LARGE_SIZE). Si lo es, se establece la variable errno con el valor ENOMEM y se devuelve NULL. En caso de que el `size` sea menor que el tamaño minimo, se tomará como tamaño el BLOCK_MIN_SIZE y se reservará una región de ese tamaño.

Luego, se busca una región de memoria libre en el arreglo de bloques correspondiente al tamaño pedido. Si no se encuentra una región libre, significa que se debe alocar un nuevo bloque. En ese caso, se crea un nuevo bloque llamando a la función `grow_heap(size)` y se agrega al arreglo de bloques correspondiente. Si falla la creación del bloque, se establece la variable errno con el valor ENOMEM y se devuelve NULL.
Una vez creado el nuevo bloque, se busca una región libre en el mismo y se devuelve un puntero a la dirección de memoria de la región.

Cabe aclarar que al buscar una región de un cierto tamaño `size` se toma en consideración también el tamaño del header, es decir, del struct region.

### find_free_region(size)
Encuentra la próxima región libre del tamaño `size` utilizando los algoritmos de busqueda `first-fit` o `best-fit` dependiendo si se definió la constante FIRST_FIT o BEST_FIT en tiempo de compilación. Esta función devuelve un puntero a la región de memoria libre que mejor se ajusta a los requisitos, según el algoritmo elegido.

En ambos se utiliza 'splitting' como método para la separación de regiones (utilizando la función `split_region(struct region *current_region, size_t size)`). Si la región libre encontrada es más grande que el tamaño solicitado, se divide la región en dos regiones, una de tamaño `size` y otra de tamaño `region->size - size - sizeof(struct region)`. La región de tamaño `size` es la que se devuelve. Al soportar splitting, nos aseguramos que se desperdicie la menor cantidad de memoria.

#### first-fit
Inicialmente obtiene el arreglo de bloques en el cual se debe buscar la región solicitada y se itera sobre el mismo. Para cada bloque, se itera sobre las regiones que lo componen. Si la región está libre y su tamaño es mayor o igual al tamaño solicitado, se devuelve un puntero a la región. Si no se encuentra ninguna región libre, se devuelve NULL.

#### best-fit
Inicialmente obtiene el arreglo de bloques en el cual busca la región de memoria libre que mejor se ajuste al tamaño solicitado, es decir, la región con el tamaño más cercano a size. Si encuentra una región libre que pueda contener al menos size bytes, la función la compara con la mejor región encontrada hasta el momento y la reemplaza si es más pequeña. Si no se encuentra ninguna región libre, se devuelve NULL.

### free(ptr)
La función `free(ptr)` se encarga de liberar la región de memoria a la que apunta el puntero `ptr`.

La función primero verifica si el puntero es nulo, en ese caso no hace nada y simplemente retorna. En caso contrario
      1. Convierte el puntero a la dirección de región correspondiente
      2. Obtiene el arreglo de bloques en el que se encuentra la región
      3. Obtiene la dirección de inicio del bloque.

A continuación, se realizan los siguientes chequeos: 
- Se verifica que la dirección de inicio de la región se encuentre dentro del rango de direcciones de bloques de memoria asignados por malloc() o realloc().
- Se verifica si la región ha sido liberada previamente. Si se ha liberado previamente, la función simplemente retorna sin hacer nada.
En ambos casos si se detecta un error, se devuelve NULL.

En caso que la región cumpla con los chequeos, se establece el flag free de la región en true y se actualiza el contador de cantidad de liberaciones realizadas.

A continuación, se realiza el coalescing de regiones adyacentes liberadas (si las hay) a través de la función `coalesce_regions()`. Esto se hace para evitar fragmentación externa de la memoria y poder reutilizar las regiones liberadas de manera más eficiente.

En caso de que todas las regiones en el bloque hayan sido liberadas, se libera el bloque de memoria a través de la función `munmap()`. Si la llamada a munmap() falla, se establece el código de error ENOMEM y se sale del programa con EXIT_FAILURE (1). 

### realloc(ptr, size)
Se encarga de cambiar el tamaño de la región de memoria apuntada por `ptr` a `size` bytes.

Permite cambiar el tamaño de un bloque de memoria previamente asignado con malloc, calloc o realloc. 
- Si el puntero ptr es nulo, la función realloc equivale a `malloc(size)`. 
- Si size es cero, la función realloc libera la memoria apuntada por ptr y devuelve NULL. 
- Si el bloque de memoria apuntado por ptr fue asignado por otra función que no sea de la bibliotecla de malloc(3), o si size es mayor que BLOCK_LARGE_SIZE, la función devuelve NULL.

Retorna
- NULL: si ptr es NULL, size es mayor que BLOCK_LARGE_SIZE, o si la memoria no puede ser asignada.
- new_ptr: un puntero a la nueva region de memoria o el mismo ptr si no fue necesario utilizar malloc porque el bloque o region tenia espacio suficiente.

Una vez obtenido un puntero a la estructura region que contiene la información del puntero de la memoria actual se llama a `size_of_block` para obtener el tamaño actual del bloque de memoria y su número de bloque correspondiente.
Ya con el arreglo de bloques donde se encuentra la región, verifica las regiones contiguas en memoria (si la región a mi derecha o la región a mi izquierda se encuentran libres). De ser así y la suma de los tamaños es mayor o igual al nuevo tamaño actual requerido, se puede expandir la región. La función fusiona las dos regiones de memoria y devuelve ptr o el prev (copiando el contenido de `curr` en prev y seteando la memoria de `curr` en 0) en caso de que la region previa tenga lugar.

Si ni la región a mi izquierda o a mi derecha cumple con lo pedido, la función llama a `malloc(size)` para asignar nueva memoria del tamaño especificado y copia el contenido de la region de memoria actual a la nueva. Luego, la función llama a free(ptr) para liberar el bloque de memoria anterior y devuelve el nuevo puntero.

### calloc(nmemb, size)
Se encarga de reservar un bloque de memoria de tamaño `nmemb * size` utilizando la función `malloc()` que reserva el tamaño solicitado. Inicializa toda esa memoria en 0 utilizando `memset`.

### grow_heap(size)
Se encarga de crear un nuevo bloque de memoria de tamaño `size` utilizando la función `mmap()`. Si la llamada a mmap() falla, se establece el código de error ENOMEM y se sale del programa.

## Tests
Se realizaron pruebas unitarias para cada una de las funciones implementadas. 
amount_of_mallocs; amount_of_frees; requested_memory; amount_small_blocks; amount_medium_blocks; amount_large_blocks; son variables globales que se utilizan para llevar un registro de las peticiones a memoria / cantidad de bloques de memoria asignados por tamaño. De esta forma se puede llevar a cabo un testeo más exhaustivo de las funciones implementadas.
