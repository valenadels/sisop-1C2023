#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	if (sys_reduce_priority(100) == -1) {
		cprintf("No se pudo aumentar la prioridad\n");
		if (sys_get_priority() == 49)
			cprintf("La prioridad no cambió\n");
	}

	if (sys_reduce_priority(-5) == -1)
		cprintf("No se pudo reducir la prioridad a -5 ya que no existe "
		        "prioridad negativa\n");
}