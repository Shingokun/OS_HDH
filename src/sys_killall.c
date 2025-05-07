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
 #include "string.h"
 #include "libmem.h"
 #include <stdlib.h>
 
 struct simple_proc {
     uint32_t pid;          
     char path[100];        
     int pc;                
     struct simple_proc *next; 
 };
  
 static struct simple_proc *proc_list = NULL;
  
 void add_process(uint32_t pid, const char *path) {
     struct simple_proc *new_proc = (struct simple_proc *)malloc(sizeof(struct simple_proc));
     new_proc->pid = pid;
     strncpy(new_proc->path, path, sizeof(new_proc->path) - 1);
     new_proc->path[sizeof(new_proc->path) - 1] = '\0';
     new_proc->pc = 0;
     new_proc->next = proc_list;
     proc_list = new_proc;
 }
  
 void remove_process(struct simple_proc *proc_to_remove) {
     struct simple_proc *curr = proc_list;
     struct simple_proc *prev = NULL;
 
     while (curr != NULL) {
         if (curr == proc_to_remove) {
             if (prev == NULL) {
                 proc_list = curr->next;
             } else {
                 prev->next = curr->next;
             }
             free(curr);
             break;
         }
         prev = curr;
         curr = curr->next;
     }
 }
  
 int __sys_killall(struct pcb_t *caller, struct sc_regs* regs)
 {
     char proc_name[100] = {0};  
     uint32_t data;
  
     uint32_t memrg = regs->a1; 
     if (proc_list == NULL) {
         add_process(caller->pid, "sc2");  
     }
  
     int i = 0;
     int read_result;
     while (i < 99) {
         read_result = libread(caller, memrg, i, &data);
         if (read_result != 0) {  
             printf("Error: libread failed with code %d at offset %d for memregionid %d\n", 
                    read_result, i, memrg);
             return -1;
         }
         if (data == (uint32_t)-1) { 
             proc_name[i] = '\0';
             break;
         }
         if (data < 32 || data > 126) {  
             printf("Error: Invalid character (0x%x) at offset %d for memregionid %d\n", 
                    data, i, memrg);
             return -1;
         }
         proc_name[i] = (char)data;
         i++;
     }
     proc_name[99] = '\0';  
 
     if (i == 0 && proc_name[0] == '\0') {
         printf("Error: Could not retrieve process name from memregionid %d\n", memrg);
         return -1;
     }
 
     printf("The procname retrieved from memregionid %d is \"%s\"\n", memrg, proc_name);
  
     int terminated_count = 0;
     struct simple_proc *curr = proc_list;
     struct simple_proc *next = NULL;
 
     while (curr != NULL) {
         next = curr->next; 
         if (strcmp(curr->path, proc_name) == 0) {
             printf("Terminating process with PID %d and name %s\n",
                    curr->pid, curr->path);
             
             remove_process(curr); 
             terminated_count++;
         }
         curr = next;
     }
 
     if (terminated_count == 0) {
         printf("No processes found with name %s\n", proc_name);
     } else {
         printf("Terminated %d process(es) with name %s\n", terminated_count, proc_name);
     }
 
     return 0; 
 }