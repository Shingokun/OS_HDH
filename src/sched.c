
#include "queue.h"
#include "sched.h"
#include <pthread.h>

#include <stdlib.h>
#include <stdio.h>
static struct queue_t ready_queue;
static struct queue_t run_queue;
static pthread_mutex_t queue_lock;

#ifdef MLQ_SCHED
static struct queue_t mlq_ready_queue[MAX_PRIO];
static int SlotNum[MAX_PRIO] = {0};
static int curr_pos = 0;
#endif

#ifdef MLQ_SCHED
void set_slot(void)
{
	for (int a = 0; a < MAX_PRIO; a++)
	{
		if (SlotNum[a] == 0)
		{
			SlotNum[a] = MAX_PRIO - a;
		}
	}
}
#endif

int queue_empty(void)
{
#ifdef MLQ_SCHED
	unsigned long prio;
	for (prio = 0; prio < MAX_PRIO; prio++)
		if (!empty(&mlq_ready_queue[prio]))
			return -1;
#endif
	return (empty(&ready_queue) && empty(&run_queue));
}

void init_scheduler(void)
{
#ifdef MLQ_SCHED
	int i;

	for (i = 0; i < MAX_PRIO; i++)
	{
		mlq_ready_queue[i].size = 0;
	}
	set_slot();
	curr_pos = 0;
#endif
	ready_queue.size = 0;
	run_queue.size = 0;
	pthread_mutex_init(&queue_lock, NULL);
}

#ifdef MLQ_SCHED
/*
 *  Stateful design for routine calling
 *  based on the priority and our MLQ policy
 *  We implement stateful here using transition technique
 *  State representation   prio = 0 .. MAX_PRIO, curr_slot = 0..(MAX_PRIO - prio)
 */
struct pcb_t *get_mlq_proc(void)
{

	/*TODO: get a process from PRIORITY [ready_queue].
	 * Remember to use lock to protect the queue.
	 */
	pthread_mutex_lock(&queue_lock);
	struct pcb_t *proc = NULL;
	// int i;
	// int flag = 0;
	// for (i = 0; i < MAX_PRIO; i++)
	// {
	// 	if (mlq_ready_queue[i].size != 0)
	// 	{
	// 		flag = 1;
	// 	}
	// 	if (mlq_ready_queue[i].size != 0 && SlotNum[i] > 0)
	// 	{
	// 		proc = dequeue(&mlq_ready_queue[i]);
	// 		SlotNum[i]--;
	// 		pthread_mutex_unlock(&queue_lock);
	// 		return proc;
	// 	}
	// 	if (i == MAX_PRIO - 1 && flag == 1)
	// 	{
	// 		set_slot();
	// 		i = 0;
	// 	}
	// }
	int i;
	int flag = 0;
	for (i = 0; i < MAX_PRIO; i++)
	{
		if (mlq_ready_queue[i].size != 0)
		{
			flag = 1;
		}
	}
	while (curr_pos < MAX_PRIO)
	{
		if (mlq_ready_queue[curr_pos].size != 0 && SlotNum[curr_pos] > 0)
		{
			proc = dequeue(&mlq_ready_queue[curr_pos]);
			SlotNum[curr_pos]--;
			pthread_mutex_unlock(&queue_lock);
			return proc;
		}
		if (curr_pos == MAX_PRIO - 1 && flag == 1)
		{
			set_slot();
			curr_pos = 0;
			continue;
		}
		curr_pos++;
		if (curr_pos == MAX_PRIO)
		{
			curr_pos = 0;
			break;
		}
	}

	pthread_mutex_unlock(&queue_lock);
	// proc = dequeue(&mlq_ready_queue[0]);
	return proc;
}

void put_mlq_proc(struct pcb_t *proc)
{
	pthread_mutex_lock(&queue_lock);
	enqueue(&mlq_ready_queue[proc->prio], proc);
	pthread_mutex_unlock(&queue_lock);
}

void add_mlq_proc(struct pcb_t *proc)
{
	pthread_mutex_lock(&queue_lock);
	enqueue(&mlq_ready_queue[proc->prio], proc);
	pthread_mutex_unlock(&queue_lock);
}

struct pcb_t *get_proc(void)
{
	return get_mlq_proc();
}

void put_proc(struct pcb_t *proc)
{
	return put_mlq_proc(proc);
}

void add_proc(struct pcb_t *proc)
{
	return add_mlq_proc(proc);
}
#else
struct pcb_t *get_proc(void)
{
	struct pcb_t *proc = NULL;
	/*TODO: get a process from [ready_queue].
	 * Remember to use lock to protect the queue.
	 * */
	pthread_mutex_lock(&queue_lock);
	if (ready_queue.size != 0)
		proc = dequeue(&ready_queue);
	pthread_mutex_unlock(&queue_lock);
	return proc;
}

void put_proc(struct pcb_t *proc)
{
	pthread_mutex_lock(&queue_lock);
	enqueue(&run_queue, proc);
	pthread_mutex_unlock(&queue_lock);
}

void add_proc(struct pcb_t *proc)
{
	pthread_mutex_lock(&queue_lock);
	enqueue(&ready_queue, proc);
	pthread_mutex_unlock(&queue_lock);
}
#endif
