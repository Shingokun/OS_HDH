/*
 * Copyright (C) 2025 pdnguyen of HCMC University of Technology VNU-HCM
 */

/* Sierra release
 * Source Code License Grant: The authors hereby grant to Licensee
 * personal permission to use and modify the Licensed Source Code
 * for the sole purpose of studying while attending the course CO2018.
 */

 #include "common.h"
 #include "syscall.h"
 #include "stdio.h"
 #include "libmem.h"
 #include <stdlib.h>  
 #include <string.h>  
 #include "queue.h"    
 
 int __sys_killall(struct pcb_t *caller, struct sc_regs* regs) {
    char proc_name[100];
    uint32_t data;
    uint32_t memrg = regs->a1;
     
    int i = 0;
    data = 0;
    while(data != -1) {
        libread(caller, memrg, i, &data);
        proc_name[i] = data;
        if(data == -1) proc_name[i] = '\0';
        i++;
    }
    printf("The procname retrieved from memregionid %d is \"%s\"\n", memrg, proc_name);
 
    #ifdef MLQ_SCHED 
    for(i = 0; i < caller->running_list->size; i++) {
        struct pcb_t *proc = caller->running_list->proc[i];
        if(proc && strcmp(proc->path, proc_name) == 0) { 
            caller->running_list->proc[i] = NULL; 
            free(proc);
        }
    }

    for(int prio = 0; prio < MAX_PRIO; prio++) {
        struct queue_t *q = &caller->mlq_ready_queue[prio];
        for(i = 0; i < q->size; i++) {
            struct pcb_t *proc = q->proc[i];
            if(proc && strcmp(proc->path, proc_name) == 0) { 
                q->proc[i] = NULL; 
                free(proc);
            }
        } 
        int new_size = 0;
        for(i = 0; i < q->size; i++) {
            if(q->proc[i] != NULL) {
                q->proc[new_size++] = q->proc[i];
            }
        }
        q->size = new_size;
    }
    #endif

    return 0;
}