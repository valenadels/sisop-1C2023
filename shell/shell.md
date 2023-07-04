# shell

### Búsqueda en $PATH
1- ¿Cuáles son las diferencias entre la syscall execve(2) y la familia de wrappers proporcionados por la librería estándar de C (libc) exec(3)?

La familia de wrappers exce(3) se diferencian entre sí y de execve(2) unicamente en la forma en la que cada una especifica el nombre del programa, la lista de argumentos y la lista de variables de entorno del nuevo progama. Es decir, cada una de estas funciones proporciona una interfaz diferente para la misma funcionalidad. 

Por un lado tenemos a la función
int execve(const char *pathname, char *const argv[], char *const envp[]);
donde
- pathname hace referencia al nuevo programa a ser cargado en la memoria del proceso
- agrv especifica los argumentos de comando de linea que recibe el nuevo programa
- envp especifica el entorno del nuevo progama

Por otro lado tenemos a las famila de funciones exec:
- int execle(const char *pathname, const char *arg, ... /* , (char *) NULL, char *const envp[] */ );
- int execlp(const char *filename, const char *arg, ... /* , (char *) NULL */);
- int execl(const char *pathname, const char *arg, ... /* , (char *) NULL */)
- int execvp(const char *filename, char *const argv[]);
- int execv(const char *pathname, char *const argv[]);
- int execvpe(const char *file, char *const argv[], char *const envp[]);

donde los prefijos en los nombres de estas funciones indica la diferencia entre ellas.
- *l - execl(), execlp(), execle():* Requieren que se especifique la lista de argumentos como una lista de strings dentro de la llamada. Un puntero NULL debe terminar la lista de argumentos, para que estas llamadas puedan ubicar el final de la lista. El primer argumento por convención corresponde a argv[0] en main del nuevo programa.
- *v - execv(), execvp(), execvpe():* Se diferencian de las funciones 'l' por la forma en la que se especifica la lista de argumentos. Esta debe ser un array de punteros a Strings terminados en NULL, con el primer elemento apuntando al nombre del programa asociado con el archivo que esta siendo ejecutado
- *e - execle(), execvpe():*  A diferencia de las demás funciones exec(), que usan el entorno existente del 'caller' (contenido de environ) como el entorno para el nuevo programa, estas dos funciones permiten especificar el entorno del nuevo programa a través del argumento envp. 
- *p - execlp(), execvp(), execvpe():* Permiten que el programa se especifique solo con un nombre de archivo. El nombre de archivo se busca en la lista de directorios especificados en la variable de entorno PATH.  La variable PATH no se utiliza si el nombre de archivo contiene una barra diagonal (/), en cuyo caso se trata como una ruta de acceso relativa o absoluta. Además, ciertos errores son manejados de distinta forma a comparación con las demás funciones. Por ejemplo, si se deniega el permiso para un archivo (el intento de execve(2) falló con el error EACCES), estas funciones seguirán buscando en el resto de la ruta de búsqueda. Si no se encuentra ningún otro archivo, sin embargo, se devolverá con errno establecido en EACCES.

2- ¿Puede la llamada a exec(3) fallar? ¿Cómo se comporta la implementación de la shell en ese caso?

Sí, puede fallar, por ejemplo en estos casos:
- El path al archivo que se le pasa por argumento no existe
- El archivo no contiene los permisos necesarios
- El espacio total requerido por la lista de argumentos y de variables de entorno, excede el máximo permitido
- El archivo referido por pathname ya está siendo escrito por otro proceso
En caso de error devuelve -1 y setea el errno correspondiente.
Nuestra shell en caso de error, realiza un exit(-1) del proceso actual e imprime el error por pantalla. Luego, vuelve a mostrar el prompt esperando el próximo comando. 

![Ejemplo de error en exec](https://github.com/fiubatps/sisop_2023a_g04/blob/entrega_shell/shell/ejemploerror.png)

---

### Procesos en segundo plano
1- Detallar cuál es el mecanismo utilizado para implementar procesos en segundo plano.

Para implementar esto, en runcmd.c verificamos si parsed->type es BACK, lo que indica que el proceso debe ejecutarse en segundo plano. Si es así, el programa llama a la función "print_back_info" para imprimir información sobre el proceso en segundo plano.

Luego, se llama a la función "waitpid" con los argumentos: pid del padre y "status" donde se guardará el estado de salida del proceso hijo. Esto indica que el proceso principal esperará a que el proceso en segundo plano termine antes de continuar. También se le pasa como tercer argumento: WNOHANG que indica que el proceso principal no se bloqueará mientras espera el proceso en segundo plano, lo que significa que puede continuar ejecutando otros procesos en segundo plano o en primer plano. 
En general, el identificador de proceso que recibe waitpid puede ser uno de los siguientes valores, nosotros utilizamos el primero:
- Si el valor del identificador de proceso es mayor que 0, waitpid() espera al proceso hijo con ese PID específico.
- Si el valor del identificador de proceso es -1, waitpid() espera a cualquier proceso hijo.
- Si el valor del identificador de proceso es 0, waitpid() espera a cualquier proceso hijo que tenga el mismo grupo de procesos que el proceso padre.
- Si el valor del identificador de proceso es menor que -1, waitpid() espera a cualquier proceso hijo que tenga el mismo grupo de procesos que el valor absoluto del identificador de proceso.

Para la ejecución del comando en segundo plano llamamos a exec_cmd() con el comando correspondiente.

---

### Flujo estándar

1 -  Investigar el significado de 2>&1, explicar cómo funciona su forma general

"2>&1" es una forma de redirigir el flujo de salida de error estándar hacia el mismo destino que la salida estándar. El símbolo "2" en "2>&1" hace referencia al canal "stderr" y el "&1" indica que la salida de error estándar debe redirigirse al mismo destino que la salida estándar.

2 - Mostrar qué sucede con la salida de cat out.txt en el ejemplo.
 ![Ejemplo de salida cat out.txt](https://github.com/fiubatps/sisop_2023a_g04/blob/entrega_shell/shell/catout1.jpeg)

 Luego repetirlo, invirtiendo el orden de las redirecciones (es decir, 2>&1 >out.txt). ¿Cambió algo?
 ![Ejemplo de salida cat out2.txt](https://github.com/fiubatps/sisop_2023a_g04/blob/entrega_shell/shell/catout2.jpeg)

 Como podemos visualizar, en nuestra implementación de shell no cambió nada. Esto es debido a como se maneja el parsing. En el código proporcionado, la parte que hace que invertir el orden de los redireccionamientos no cambie la salida se encuentra en la función parse_redir_flow, donde se redirecciona el output. La razón por la cual la inversión de los redireccionamientos no afecta la salida es porque tanto la salida estándar (stdout) como la salida de error estándar (stderr) están siendo redirigidas al mismo archivo en el primer comando, donde primero se redirige la salida estándar al archivo "out.txt" y luego se redirige la salida de error estándar a la misma dirección que la salida estándar (es decir, al archivo "out.txt"). Cuando se invierte el orden, la salida estándar se redirige primero a la salida de error estándar, y luego la salida de error estándar y la salida estándar redirigen ambos a "out.txt". Como resultado, el archivo "out.txt" contiene la misma información en ambas situaciones.


 Comportamiento en bash:

 ![Ejemplo de salida](https://github.com/fiubatps/sisop_2023a_g04/blob/entrega_shell/shell/redireccionamiento_bash.png)

 Vemos que aquí es diferente. Lo que ocurre es que al invertir el orden de las redirecciones, debido a 2>&1 se va a imprimir el error por la salida estandar y esto ocurre antes de que se redireccione la salida estandar a out.txt, entonces en out.txt queda solo la salida de ls y no el error.



### Tuberías múltiples
Investigar qué ocurre con el exit code reportado por la shell si se ejecuta un pipe 

1 - ¿Cambia en algo? ¿Qué ocurre si, en un pipe, alguno de los comandos falla? Mostrar evidencia (e.g. salidas de terminal) de este comportamiento usando bash. Comparar con su implementación.

Si se ejecuta un pipe, el exit code reportado por la shell corresponde al exit code del último comando en la cadena y si alguno de los comandos falla, el exit code que devuelve la shell seguirá siendo el correspondiente al último comando.

Aquí devuelve 0 ya que el pipe se ejecutó correctamente
![Ejemplo en bash de caso exitoso](https://github.com/fiubatps/sisop_2023a_g04/blob/entrega_shell/shell/ejemplo_bash_pipe_sin_error.png)

Aquí devuelve el exit code correspondiente al útlimo comando, que en este caso es un comando inexistente
![Ejemplo en bash de caso de error en el último comando](https://github.com/fiubatps/sisop_2023a_g04/blob/entrega_shell/shell/ejemplo_bash_pipe_con_error.png)

Sin embargo, al invertir el orden, es decir, ahora el comando que falla se encuentra en la primer posición, el exit code sigue correspondiendo al del último comando, que en este caso es exitoso.
![Ejemplo en bash de caso de error en el primer comando](https://github.com/fiubatps/sisop_2023a_g04/blob/entrega_shell/shell/ejemplo_bash_pipe_con_error_invirtiendo_orden.png)

En cuanto a nuestra implementación:

Para nuestra shell, como no soporta esta característica, el exit code siempre va a ser 0 (exitoso)

![Ejemplo de nuestra shell de caso exitoso](https://github.com/fiubatps/sisop_2023a_g04/blob/entrega_shell/shell/ejemplo_lab_sin_error.png)

![Ejemplo de nuestra shell de caso de error en el último comando](https://github.com/fiubatps/sisop_2023a_g04/blob/entrega_shell/shell/ejemplo_lab_con_error.png)

![Ejemplo de nuestra shell de caso de error en el primer comando](https://github.com/fiubatps/sisop_2023a_g04/blob/entrega_shell/shell/ejemplo_lab_error_invirtiendo_orden.png)


---

### Variables de entorno temporarias
1- ¿Por qué es necesario hacerlo luego de la llamada a fork(2)?

Se debe realizar luego de la llamada a fork(2) porque las variables de entorno temporales son para utilizarse en un comando específico, es decir, se pasan para que estén disponibles en el proceso donde se ejecutará el binario correspondiente. Por ejemplo:

![Ejemplo de variables](https://github.com/fiubatps/sisop_2023a_g04/blob/entrega_shell/shell/variables.png)

En ese caso, se setean las variables de entorno USER y ENVIRON. Luego, el proceso donde se ejecute /usr/bin/env las tendrá disponibles, por lo que el grep las mostrará en stdout. Después de la ejecución de ese comando podemos ver que por ejemplo, USER sigue como estaba antes.
Si la setearamos antes del fork, eso no pasaría, se hubiera cambiado a "nadie".

2- En algunos de los wrappers de la familia de funciones de exec(3) (las que finalizan con la letra e), se les puede pasar un tercer argumento (o una lista de argumentos dependiendo del caso), con nuevas variables de entorno para la ejecución de ese proceso. Supongamos, entonces, que en vez de utilizar setenv(3) por cada una de las variables, se guardan en un arreglo y se lo coloca en el tercer argumento de una de las funciones de exec(3).
¿El comportamiento resultante es el mismo que en el primer caso? Explicar qué sucede y por qué.
Describir brevemente (sin implementar) una posible implementación para que el comportamiento sea el mismo.

```c
    e - execle(), execvpe()
       The  environment of the caller is specified via the argument envp.  The
       envp argument is an array of pointers to  null-terminated  strings  and
       must be terminated by a null pointer.

       All  other  exec()  functions  (which do not include 'e' in the suffix)
       take the environment for the new process image from the external  vari‐
       able environ in the calling process.
```
Como se puede ver, cuando se pasan nuevas variables de entorno como parte del tercer argumento de una función de la familia exec(3), estas variables de entorno son creadas dentro del espacio de memoria del nuevo proceso que se ejecutará mediante la función exec(3). En contraste, las otras funciones exec() (aquellas que no incluyen una "e" en el sufijo) utilizan la variable externa environ en el proceso actual como la tabla de variables de entorno para el nuevo proceso que se ejecutará. Esto significa que las variables de entorno del proceso actual se copiarán a la tabla de variables de entorno del nuevo proceso.
Por lo tanto la respuesta es no, el comportamiento no es el mismo. Si se pasan las nuevas variables de entorno como un tercer argumento en las funciones exec(3) que terminan en "e", en lugar de utilizar la función setenv(3), solo se establecerán esas variables de entorno para el nuevo proceso que se ejecutará a través de exec(3).
Para que el comportamiento sea similar, podríamos:
- Crear un arreglo de punteros char que contenga todas las nuevas variables de entorno que se desean agregar más las existentes en el proceso actual. Cada puntero en el arreglo apuntaría a una cadena de caracteres en el formato "nombre=valor". Este arreglo debe terminar en NULL.
- Llamar a la función exec(3) para ejecutar el nuevo proceso. 

---

### Pseudo-variables
1- Investigar al menos otras tres variables mágicas estándar, y describir su propósito.
Incluir un ejemplo de su uso en bash (u otra terminal similar).

- HISTFILE: Contiene el nombre del archivo en el que se guarda el historial de comandos. El valor predeterminado en bash es ~/.bash_history.
- HISTFILESIZE: El número máximo de líneas contenidas en el archivo de historial se especifica mediante la variable HISTSIZE. Cuando se asigna un valor a esta variable, el archivo de historial se trunca, si es necesario, para contener no más de ese número de líneas eliminando las entradas más antiguas. El archivo de historial también se trunca a este tamaño después de escribirlo cuando un shell sale. Si el valor es 0, el archivo de historial se trunca a un tamaño de cero. Los valores no numéricos y los valores numéricos menores que cero inhiben la truncación. El valor predeterminado se establece en el valor de HISTSIZE después de leer cualquier archivo de inicio.
- SHLVL: Es una variable que se incrementa en uno cada vez que se inicia una nueva instancia de Bash. Esto está diseñado para ser un recuento de cuán profundamente están anidados tus shells de Bash.
Esta variable puede ser útil para rastrear cuántos shells de Bash diferentes tienes abiertos y cuán profundos están anidados.
Ejemplos en bash:
```c
//HISTFILE
valentinaadelsflugel@AR6S123J3:~$ $HISTFILE
bash: /home/valentinaadelsflugel/.bash_history: Permiso denegado

//SHLVL
valentinaadelsflugel@AR6S123J3:~$ echo $SHLVL
4

//HISTFILESIZE
valentinaadelsflugel@AR6S123J3:~$ echo $HISTFILESIZE
2000
```
---

### Comandos built-in

¿Entre cd y pwd, alguno de los dos se podría implementar sin necesidad de ser built-in? ¿Por qué? ¿Si la respuesta es sí, cuál es el motivo, entonces, de hacerlo como built-in? (para esta última pregunta pensar en los built-in como true y false)

Para contestar estas preguntas primero deberíamos poder responder que es un comando built-in. Un comando built-in es un comando que está contenido en la shell, contrario a los comandos externos que se deben ejecutar en procesos separados. Esto hace que ejecutar un built-in sea mucho más rápido y eficiente que ejecutar uno externo. 

No es posible implementar el comando cd de otra manera que no sea built-in. La funcion de cd es cambiar el directorio actual (cwd = 'Current Working Directory'), este es una propiedad del proceso de la misma shell, si se intentase cambiar de directorio en un proceso hijo al proceso de la shell, este cambio afectaría al proceso hijo, y al volver a la shell uno se daría cuenta rápidamente que el cambio de directorio no se produjo. 

Por otro lado, aunque el comando pwd, cuya funcionalidad es mostrar el directorio actual, es generalmente implementado como un built-in hay algunos ambientes en los que se desarrolla como un comando externo, ubicado en "/bin/pwd" o en otro lugar del filesystem. La razón por la cual pwd puede ser implementado como un comando externo es que puede ser útil en situaciones en las que la shell no está disponible, como cuando se trabaja escribiendo scripts y programas que quieren acceder al directorio actual de trabajo sin la necesidad de tener una shell corriendo. Este comando generalmente es un built-in ya que esto es mucho más eficiente. El directorio actual de trabajo es una propiedad de la shell, por lo que cuando se llama a pwd lo único que hace la shell es printear el directorio. Como la shell ya tiene acceso a su directorio actual, no tiene necesidad de llamar a un ejecutable externo para realizar este trabajo.


### Historial

1- ¿Cuál es la función de los parámetros MIN y TIME del modo no canónico? ¿Qué se logra en el ejemplo dado al establecer a MIN en 1 y a TIME en 0?

Cuando la terminal se encuentra en modo canonico el input del usuario se obtiene línea por línea, es decir se espera a que se reciba un Enter para obtener el comando. Por otro lado cuando la terminal se encuentra en modo no canónico el input se encuentra disponible apenas es tipado. 

Los parámetros MIN y TIME controlan el comportamiento de la terminal cuando esta se encuentra en modo no canónico. MIN representa el número minimo de caracteres que se deben recibir para que se complete una lectura de read(). Si por ejemplo MIN se setea a 0, read() retornará apenas se encuentre un input. TIME representa el tiempo en décimas de segundo que read() se bloqueará hasta que por lo menos MIN caracteres sean recibidos. Si se asignan valores a ambos, read() esperará a que se haya recibido al menos un caracter y terminará cuando se hayan recibido una cantidad MIN de caracteres o haya pasado un tiempo TIME desde que se recibió el último caracter. Si solo se le asigna un valor a MIN, la lectura no va a terminar antes de que se hayan recibido MIN caracteres. Por otro  lado, si solo se le asigna un valor a TIME la lectura va a terminar cuando se haya recibido al menos un caracter o se cumpla el tiempo del temporizador. 

Los valores por default de MIN y TIME son 1 y 0 respectivamente. Lo que se hace en el ejemplo sería equivalente a no explicitar sus valores. Cuando estan seteados a estos valores lo que se quiere lograr es que el read() devuelva lo antes posible que se obtenga por lo menos un caracter, sin esperar ningun tiempo extra. Si hay input disponible read() va a retornar inmediatamente con por lo menos un caracter, y si no hay input disponible read() retornará un 0 indicando que no se leyeron caracteres.

---

## Bibliografía
Busqueda en path
1. [KERR] §27.1, §27.2 / man exec / man 2 execve
2. [KERR] §27.1
Historial
https://manpages.ubuntu.com/manpages/trusty/es/man3/termios.3.html
