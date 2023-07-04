#include <inc/assert.h>
#include <inc/x86.h>
#include <kern/spinlock.h>
#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/monitor.h>
#include <kern/random.h>

#define MIN_TICKETS 1

int amount_sched_calls = 0;
int total_executions = 0;
unsigned int pid[100000];
unsigned int pid_index = 0;
unsigned int env_runs[100000];


void sched_halt(void);
void round_robin(void);
void lottery(void);
// Choose a user environment to run and run it.
void
sched_yield(void)
{
	// actualizar estadisticas
	amount_sched_calls++;

#ifdef LOTTERY
	lottery();
#endif
#ifdef ROUND_ROBIN
	round_robin();
#endif
}

void
round_robin()
{
	struct Env *idle;

	// Implement simple round-robin scheduling.
	//
	// Search through 'envs' for an ENV_RUNNABLE environment in
	// circular fashion starting just after the env this CPU was
	// last running.  Switch to the first such environment found.
	//
	// If no envs are runnable, but the environment previously
	// running on this CPU is still ENV_RUNNING, it's okay to
	// choose that environment.
	//
	// Never choose an environment that's currently running on
	// another CPU (env_status == ENV_RUNNING). If there are
	// no runnable environments, simply drop through to the code
	// below to halt the cpu.

	// Your code here
	// Wihtout scheduler, keep runing the last environment while it exists


	idle = curenv;
	int start_index = idle ? ENVX(idle->env_id) + 1 : 0;

	for (int i = 0; i < NENV; i++) {
		int idx = (start_index + i) % NENV;
		struct Env *env = &envs[idx];
		if (env->env_status == ENV_RUNNABLE) {
			total_executions++;
			pid[pid_index++] = env->env_id;
			env_run(env);
		}
	}

	if (idle && idle->env_status == ENV_RUNNING) {
		total_executions++;
		pid[pid_index++] = idle->env_id;
		env_run(idle);
	}

	// sched_halt never returns
	sched_halt();
}

/*
 * Lottery scheduling algorithm
 *
 * 1. Calculate the total number of tickets
 * 2. Generate a random ticket number
 * 3. Find the process that has the winning ticket
 * 4. Run the process
 */
void
lottery(void)
{
	int total_tickets = 0;
	struct Env *env = curenv;

	struct Env *envs_to_run[NENV];
	int envs_to_run_total = 0;

	for (int i = 0; i < NENV; i++) {
		if (envs[i].env_status == ENV_RUNNABLE) {
			total_tickets += envs[i].tickets;
		}
	}

	if (total_tickets == 0) {
		if (env && env->env_status == ENV_RUNNING) {
			total_executions++;
			pid[pid_index++] = env->env_id;
			if (env->tickets > MIN_TICKETS)
				env->tickets--;
			env_run(env);
		} else {
			sched_halt();
		}
	}

	// Generate a random ticket number
	int winning_ticket = next_random() % total_tickets;
	int ticket_sum = 0;

	// find processes that are possible candidates to run
	for (int i = 0; i < NENV; i++) {
		ticket_sum += envs[i].tickets;
		if (envs[i].env_status == ENV_RUNNABLE &&
		    ticket_sum > winning_ticket) {
			int j = 0;
			if (envs_to_run_total > 0) {
				while (j < envs_to_run_total) {
					if (envs_to_run[j]->tickets !=
					    envs[i].tickets) {
						break;
					}
					j++;
				}
			}

			if (j == envs_to_run_total) {
				envs_to_run[envs_to_run_total] = &envs[i];
				envs_to_run_total++;
			}
		}
	}

	if (envs_to_run_total > 0) {
		// run one process based on a random index
		int final_winner = next_random() % envs_to_run_total;
		struct Env *final_env = envs_to_run[final_winner];
		if (final_env->tickets > MIN_TICKETS)
			final_env->tickets--;
		total_executions++;
		pid[pid_index++] = final_env->env_id;
		env_run(final_env);
	}
}

// Halt this CPU when there is nothing to do. Wait until the
// timer interrupt wakes it up. This function never returns.
//
void
sched_halt(void)
{
	int i;

	// For debugging and testing purposes, if there are no runnable
	// environments in the system, then drop into the kernel monitor.
	for (i = 0; i < NENV; i++) {
		if ((envs[i].env_status == ENV_RUNNABLE ||
		     envs[i].env_status == ENV_RUNNING ||
		     envs[i].env_status == ENV_DYING))
			break;
	}
	if (i == NENV) {
		cprintf("No runnable environments in the system!\n");

		cprintf("History of executed processes (PID)\n");
		for (int i = 0; i < pid_index; i++) {
			env_runs[ENVX(pid[i])]++;
			cprintf("Process of id: %d Ran: %d times.\n ",
			        pid[i],
			        env_runs[ENVX(pid[i])]);
		}
		cprintf("Total executions: %d\n", total_executions);
		cprintf("Scheduler was called a total of: %d times.\n",
		        amount_sched_calls);

		while (1)
			monitor(NULL);
	}

	// Mark that no environment is running on this CPU
	curenv = NULL;
	lcr3(PADDR(kern_pgdir));

	// Mark that this CPU is in the HALT state, so that when
	// timer interupts come in, we know we should re-acquire the
	// big kernel lock
	xchg(&thiscpu->cpu_status, CPU_HALTED);

	// Release the big kernel lock as if we were "leaving" the kernel
	unlock_kernel();

	// Once the scheduler has finishied it's work, print statistics on
	// performance. Your code here

	// Reset stack pointer, enable interrupts and then halt.
	asm volatile("movl $0, %%ebp\n"
	             "movl %0, %%esp\n"
	             "pushl $0\n"
	             "pushl $0\n"
	             "sti\n"
	             "1:\n"
	             "hlt\n"
	             "jmp 1b\n"
	             :
	             : "a"(thiscpu->cpu_ts.ts_esp0));
}
