
#include <kernel.h>

PORT_DEF port[MAX_PORTS];
PORT next_free_port;


PORT create_port()
{
	return create_new_port(active_proc);
}


PORT create_new_port (PROCESS owner)
{
	PORT p;
	assert(owner->magic == MAGIC_PCB);
	if(next_free_port == NULL)
		panic("create_new_port(): PORT Full");
	p = next_free_port;
	next_free_port = p->next;
	p->used = TRUE;
	p->magic = MAGIC_PORT;
	p->owner = owner;
	p->blocked_list_head = NULL;
	p->blocked_list_tail = NULL;
	p->open = TRUE;
	if(owner->first_port == NULL)
		p->next = NULL;
	else
		p->next = owner->first_port;
	owner->first_port = p;

	return p;
}


void add_to_send_blocked_list(PORT port, PROCESS proc)
{
	assert(proc->magic == MAGIC_PCB);
	assert(port->magic == MAGIC_PORT);
	if(port->blocked_list_head == NULL){
		port->blocked_list_head = proc;
	} else {
		port->blocked_list_tail->next_blocked = proc;
	}
	port->blocked_list_tail = proc;
	proc->next_blocked = NULL;
}


void open_port (PORT port)
{
	assert(port->magic == MAGIC_PORT);
	port->open = TRUE;
}



void close_port (PORT port)
{
	assert(port->magic == MAGIC_PORT);
	port->open = FALSE;
}


void send (PORT dest_port, void* data)
{
	PROCESS dest;
	assert(dest_port->magic == MAGIC_PORT);
	dest = dest_port->owner;
	assert(dest->magic == MAGIC_PCB);

	if(dest_port->open && dest->state == STATE_RECEIVE_BLOCKED){
		dest->param_proc = active_proc;
		dest->param_data = data;
		active_proc->state = STATE_REPLY_BLOCKED;
		add_ready_queue(dest);
	} else {
		add_to_send_blocked_list(dest_port, active_proc);
		active_proc->state = STATE_SEND_BLOCKED;
		active_proc->param_data = data;
	}
	active_proc->param_data = data;
	remove_ready_queue(active_proc);
	resign();
}


void message (PORT dest_port, void* data)
{
	PROCESS dest;
	assert(dest_port->magic == MAGIC_PORT);
	dest = dest_port->owner;
	assert(dest->magic == MAGIC_PCB);

	if(dest_port->open && dest->state == STATE_RECEIVE_BLOCKED){
		dest->param_proc = active_proc;
		dest->param_data = data;
		add_ready_queue(dest);
	} else {
		add_to_send_blocked_list(dest_port, active_proc);
		remove_ready_queue(active_proc);
		active_proc->state = STATE_MESSAGE_BLOCKED;
		active_proc->param_data = data;
	}
	resign();
}



void* receive (PROCESS* sender)
{
	PROCESS deliver_proc;
	PORT port;
	void *data;

	data = NULL;
	port = active_proc->first_port;
	if(port == NULL)
		panic("receive(): No port created for this process");
	while(port != NULL){
		assert(port->magic == MAGIC_PORT);
		if(port->open && port->blocked_list_head != NULL)
			break;
		port = port->next;
	}

	if(port != NULL){
		deliver_proc = port->blocked_list_head;
		assert(deliver_proc->magic == MAGIC_PCB);
		*sender = deliver_proc;
		data = deliver_proc->param_data;
		port->blocked_list_head = port->blocked_list_head->next_blocked;
		if(port->blocked_list_head == NULL)
			port->blocked_list_tail = NULL;
		if(deliver_proc->state == STATE_MESSAGE_BLOCKED){
			add_ready_queue(deliver_proc);
			return data;
		}
		if(deliver_proc->state == STATE_SEND_BLOCKED){
			deliver_proc->state = STATE_REPLY_BLOCKED;
			return data;
		}
	}
	remove_ready_queue(active_proc);
	active_proc->param_data = data;
	active_proc->state = STATE_RECEIVE_BLOCKED;
	resign();
	*sender = active_proc->param_proc;
	data = active_proc->param_data;
	return data;
}


void reply (PROCESS sender)
{
	if(sender->state != STATE_REPLY_BLOCKED)
		panic("reply(): Sender is not Reply Blocked");
	add_ready_queue(sender);
	resign();
}


void init_ipc()
{
	int i;
	next_free_port = port;
	for(i=0; i<MAX_PORTS - 1; i++){
		port[i].used = FALSE;
		port[i].magic = MAGIC_PORT;
		port[i].next = &port[i+1];
	}
	port[MAX_PORTS - 1].used = FALSE;
	port[MAX_PORTS - 1].magic = MAGIC_PORT;
	port[MAX_PORTS - 1].next = NULL;
}