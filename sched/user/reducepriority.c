#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	sys_reduce_priority(47);
	if (sys_get_priority() == 47)
		cprintf("Se redujo la prioridad a 47 correctamente\n");
}