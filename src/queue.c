#include <stdio.h>
#include <stdlib.h>
#include "queue.h"

int empty(struct queue_t * q) {
	return (q->size == 0);
}

void enqueue(struct queue_t * q, struct pcb_t * proc) {
	/* TODO: put a new process to queue [q] */
	if (q->size == MAX_QUEUE_SIZE) return;
	q->proc[q->size] = proc;
	q->size++;
}

struct pcb_t * dequeue(struct queue_t * q) {
	/* TODO: return a pcb whose prioprity is the highest
	 * in the queue [q] and remember to remove it from q
	 * */
	if (empty(q)) return NULL;

	//find highest priority proc
	uint32_t priority_max = q->proc[0]->priority;
	for (int i = 1; i < q->size; i++)
	{
		if (q->proc[i]->priority > priority_max)
		{
			priority_max = q->proc[i]->priority;
		}
	}

	//return highest priority proc
	for (int i = 0; i < q->size; i++)
	{
		if (q->proc[i]->priority == priority_max)
		{
			struct pcb_t *return_proc = q->proc[i];
			q->proc[i] = NULL;
			for (int j = i; j < q->size - 1; j++)
				q->proc[j] = q->proc[j + 1];
			q->proc[q->size - 1] = NULL;
			q->size--;

			return return_proc;
		}
	}

	return NULL;
}


