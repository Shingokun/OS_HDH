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
 #include <stdlib.h>
 #include "queue.h"
 #include "mm.h" 
 #include <string.h>
 
 int __sys_killall(struct pcb_t *caller, struct sc_regs* regs)
 {
    addr_t region_id = regs->a1;
    addr_t mem_region = caller->regs[region_id];
    if (!mem_region) {
        return -1;  
    }
 
    char target_name[100] = {0};  
    int idx = 0;
     
    while (idx < 99) {
        char c;
        if (MEMPHY_read(caller->mram, mem_region + idx, &c) != 0) {
            return -1;  
        }
        if (c == '\0') break;  
        target_name[idx++] = c;
    }
    target_name[idx] = '\0';  

    int killed = 0;
     
    if (caller->running_list) {
        struct pcb_t* process;
        int i = 0;
        
        while (i < caller->running_list->size) {
            process = caller->running_list->proc[i];
            
            if (process && strcmp(process->path, target_name) == 0) {
               
                for (int j = i; j < caller->running_list->size - 1; j++) {
                    caller->running_list->proc[j] = caller->running_list->proc[j + 1];
                }
                caller->running_list->size--;
          
                if (process->code) free(process->code);
                if (process->page_table) free(process->page_table);
#ifdef MM_PAGING
                if (process->mm) free(process->mm);
#endif
                free(process);
                
                killed++;
            } else {
                i++;
            }
        }
    }
    return killed;
}