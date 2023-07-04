#include <inc/lib.h>
#include <inc/env.h>

void
umain(int argc, char **argv)
{
	envid_t pid = sys_getenvid();
	sys_modify_tickets(pid, -10);
	if (sys_get_process_ticket(pid) == 39)
		cprintf("Se redujo la prioridad del proceso correctamente \n");
}
