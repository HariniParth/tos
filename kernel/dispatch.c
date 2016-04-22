
#include <kernel.h>

#include "disptable.c"


PROCESS active_proc;


/*
 * Ready queues for all eight priorities.
 */
PCB *ready_queue [MAX_READY_QUEUES];

unsigned ready_procs;


/*
 * add_ready_queue
 *----------------------------------------------------------------------------
 * The process pointed to by p is put the ready queue.
 * The appropiate ready queue is determined by p->priority.
 */

void add_ready_queue (PROCESS proc)
{
	int prio;

	assert(proc->magic == MAGIC_PCB);
	prio = proc->priority;
	if(ready_queue[prio] == NULL){
		ready_queue[prio] = proc;
		proc->next = proc;
		proc->prev = proc;
		ready_procs |= 1 << prio;
	} else {
		proc->next = ready_queue[prio];
		proc->prev = ready_queue[prio]->prev;
		ready_queue[prio]->prev->next = proc;
		ready_queue[prio]->prev = proc;
	}
	proc->state = STATE_READY;
}



/*
 * remove_ready_queue
 *----------------------------------------------------------------------------
 * The process pointed to by p is dequeued from the ready
 * queue.
 */

void remove_ready_queue (PROCESS proc)
{
	int prio;

	assert(proc->magic == MAGIC_PCB);
	prio = proc->priority;
	if(proc->next == proc){
		ready_queue[prio] = NULL;
		ready_procs &= ~(1 << prio);
	} else {
		ready_queue[prio] = proc->next;
		proc->prev->next = proc->next;
		proc->next->prev = proc->prev;
	}
}



/*
 * dispatcher
 *----------------------------------------------------------------------------
 * Determines a new process to be dispatched. The process
 * with the highest priority is taken. Within one priority
 * level round robin is used.
 */

PROCESS dispatcher()
{
	PROCESS new_proc;
	unsigned i;

	i = table[ready_procs];
	assert(i!=-1);
	if(i == active_proc->priority)
		new_proc = active_proc->next;
	else
		new_proc = ready_queue[i];

	return new_proc;
}



/*
 * resign
 *----------------------------------------------------------------------------
 * The current process gives up the CPU voluntarily. The
 * next running process is determined via dispatcher().
 * The stack of the calling process is setup such that it
 * looks like an interrupt.
 */
void resign()
{
	asm ("pushl %eax;pushl %ecx;pushl %edx");
	asm ("pushl %ebx;pushl %ebp;pushl %esi;pushl %edi");
	asm ("movl %%esp, %0" : "=r" (active_proc->esp) :);
	active_proc = dispatcher();
	asm ("movl %0, %%esp" :: "r" (active_proc->esp));
	asm ("popl %edi;popl %esi;popl %ebp;popl %ebx");
	asm ("popl %edx; popl %ecx;popl %eax");
}



/*
 * init_dispatcher
 *----------------------------------------------------------------------------
 * Initializes the necessary data structures.
 */

void init_dispatcher()
{
	int i;
	for(i=0;i<MAX_READY_QUEUES;i++){
		ready_queue[i] = NULL;
	}
	ready_procs = 0;

	add_ready_queue(active_proc);
}
