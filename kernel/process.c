
#include <kernel.h>


PCB pcb[MAX_PROCS];
PCB *next_free_pcb;

PORT create_process (void (*ptr_to_new_proc) (PROCESS, PARAM),
		     int prio,
		     PARAM param,
		     char *name)
{
	MEM_ADDR esp;
	PROCESS new_proc;
	PORT new_port;

	if(prio >= MAX_READY_QUEUES)
		panic("create(): Bad Priority");
	if(next_free_pcb == NULL)
		panic("create(): PCB Full");

	new_proc = next_free_pcb;
	next_free_pcb = new_proc->next;

	new_proc->used = TRUE;
	new_proc->magic = MAGIC_PCB;
	new_proc->state = STATE_READY;
	new_proc->priority = prio;
	new_proc->first_port = NULL;
	new_proc->name = name;

	new_port = create_new_port(new_proc);
	esp = 640 * 1024 - (new_proc - pcb) * 16 * 1024;

#define PUSH(x) esp -= 4; \
				poke_l(esp, (LONG) x);

	PUSH(param);
	PUSH(new_proc);
	PUSH(0);
	PUSH(ptr_to_new_proc);
	PUSH(0);
	PUSH(0);
	PUSH(0);
	PUSH(0);
	PUSH(0);
	PUSH(0);
	PUSH(0);

#undef PUSH

	new_proc->esp = esp;
	add_ready_queue(new_proc);
	return new_port;
}


PROCESS fork()
{
    // Dummy return to make gcc happy
    return (PROCESS) NULL;
}


void print_process_heading(WINDOW* wnd)
{
	wprintf(wnd,"State       Active Priority Name\n");
	wprintf(wnd,"------------------------------------\n");
}

void print_process_details(WINDOW* wnd, PROCESS p)
{
	static const char *state[] = {
		"READY          ",
		"SEND_BLOCKED   ",
		"MESSAGE_BLOCKED",
		"RECEIVE_BLOCKED",
		"REPLY_BLOCKED  "
	};
	if(!p->used){
		wprintf(wnd,"PCB slot unused");
		return;
	}
	wprintf(wnd, state[p->state]);

	if(p == active_proc){
		wprintf(wnd,"  *  ");
	} else {
		wprintf(wnd,"     ");
	}
	wprintf(wnd, "%2d  ", p->priority);
	wprintf(wnd, "%s\n", p->name);
}


void print_process(WINDOW* wnd, PROCESS p)
{
	print_process_heading(wnd);
	print_process_details(wnd,p);
}

void print_all_processes(WINDOW* wnd)
{
	int i;
	PCB *p = pcb;
	print_process_heading(wnd);
	for(i=0; i<MAX_PROCS; i++, p++){
		if(!p->used)
			continue;
		print_process_details(wnd,p);
	}
}



void init_process()
{
	int i;

	for(i=1; i<MAX_PROCS; i++){
		pcb [i].magic = 0;
		pcb [i].used = FALSE;
	}

	for(i=1; i<MAX_PROCS - 1; i++){
		pcb [i].next = &pcb [i+1];
	}
	pcb[MAX_PROCS - 1].next = NULL;
	next_free_pcb = &pcb[1];

	active_proc = pcb;
	pcb[0].state = STATE_READY;
	pcb[0].magic = MAGIC_PCB;
	pcb[0].used = TRUE;
	pcb[0].priority = 1;
	pcb[0].first_port = NULL;
	pcb[0].name = "Boot process";
}
