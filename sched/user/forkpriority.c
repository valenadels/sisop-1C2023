#include <inc/lib.h>
void
umain(int argc, char **argv)
{
	int pfork = fork();
	sys_reduce_priority(40);

	if (pfork == 0) {
		if (sys_get_priority() == 40)
			cprintf("El proceso hijo de un proceso de prioridad 40 "
			        "tiene también prioridad 40\n");
	} else {
		if (sys_get_priority() == 40)
			cprintf("El proceso padre mantiene una prioridad de 40 "
			        "luego del fork\n");
	}
}