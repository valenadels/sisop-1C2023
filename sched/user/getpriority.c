#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	cprintf("Proceso para ver la prioridad\n");
	if (sys_get_priority() == 49)
		cprintf("La prioridad del proceso que esta corriendo es de "
		        "49\n");
}