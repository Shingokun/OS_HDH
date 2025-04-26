// #ifdef MM_PAGING
/*
 * PAGING based Memory Management
 * Virtual memory module mm/mm-vm.c
 */

#include "string.h"
#include "mm.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

/*get_vma_by_num - get vm area by numID
 *@mm: memory region
 *@vmaid: ID vm area to alloc memory region
 *
 */
struct vm_area_struct *get_vma_by_num(struct mm_struct *mm, int vmaid)
{
  struct vm_area_struct *pvma = mm->mmap;

  if (mm->mmap == NULL)
    return NULL;

  int vmait = pvma->vm_id;

  while (vmait < vmaid)
  {
    if (pvma == NULL)
      return NULL;

    pvma = pvma->vm_next;
    vmait = pvma->vm_id;
  }

  return pvma;
}

int __mm_swap_page(struct pcb_t *caller, int vicfpn , int swpfpn)
{
    __swap_cp_page(caller->mram, vicfpn, caller->active_mswp, swpfpn);
    return 0;
}
struct vm_rg_struct *get_vm_area_node_at_brk(struct pcb_t *caller, int vmaid, int size, int alignedsz)
{
    struct vm_rg_struct *newrg;
    struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

    if (!cur_vma) {
        return NULL; /* VMA không tồn tại */
    }

    newrg = malloc(sizeof(struct vm_rg_struct));

    /* Cập nhật ranh giới của vùng mới */
    newrg->rg_start = cur_vma->sbrk; /* Bắt đầu từ sbrk hiện tại */
    newrg->rg_end = newrg->rg_start + alignedsz; /* Kết thúc sau kích thước được căn chỉnh */
    newrg->rg_next = NULL;

    return newrg;
  }

// struct vm_rg_struct *get_vm_area_node_at_brk(struct pcb_t *caller, int vmaid, int size, int alignedsz)
// {
//   struct vm_rg_struct * newrg;
 
//   //struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

//   //newrg = malloc(sizeof(struct vm_rg_struct));

  

//   //return newrg;
// }


 int validate_overlap_vm_area(struct pcb_t *caller, int vmaid, int vmastart, int vmaend)
 {
     struct vm_area_struct *vma = caller->mm->mmap;
 
     while (vma != NULL) {
         if (vma->vm_id != vmaid) { /* Bỏ qua VMA đang kiểm tra */
             /* Kiểm tra chồng lấn: [vmastart, vmaend] có giao với [vma->vm_start, vma->vm_end]? */
             if (!(vmaend <= vma->vm_start || vmastart >= vma->vm_end)) {
                 return -1; /* Có chồng lấn */
             }
         }
         vma = vma->vm_next;
     }
 
     return 0; /* Không chồng lấn */
 }
 int inc_vma_limit(struct pcb_t *caller, int vmaid, int inc_sz)
 {
     struct vm_rg_struct *newrg = malloc(sizeof(struct vm_rg_struct));
     int inc_amt = PAGING_PAGE_ALIGNSZ(inc_sz);
     int incnumpage = inc_amt / PAGING_PAGESZ;
     struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
 
     if (!cur_vma) {
         free(newrg);
         return -1; /* VMA không tồn tại */
     }
 
     int old_end = cur_vma->vm_end;
     struct vm_rg_struct *area = get_vm_area_node_at_brk(caller, vmaid, inc_sz, inc_amt);
 
     if (!area) {
         free(newrg);
         return -1; /* Không tạo được vùng mới */
     }
 
     /* Kiểm tra chồng lấn */
     if (validate_overlap_vm_area(caller, vmaid, area->rg_start, area->rg_end) < 0) {
         free(newrg);
         free(area);
         return -1; /* Chồng lấn */
     }
 
     /* Cập nhật giới hạn VMA */
     cur_vma->vm_end = area->rg_end;
     cur_vma->sbrk = area->rg_end; /* Cập nhật sbrk */
     newrg->rg_start = area->rg_start;
     newrg->rg_end = area->rg_end;
 
     /* Ánh xạ vùng mới vào RAM */
     if (vm_map_ram(caller, area->rg_start, area->rg_end, old_end, incnumpage, newrg) < 0) {
         free(newrg);
         free(area);
         return -1; /* Ánh xạ thất bại */
     }
 
     /* Thêm vùng mới vào danh sách tự do */
     enlist_vm_rg_node(&cur_vma->vm_freerg_list, newrg);
     free(area);
 
     return 0;
 }
/*inc_vma_limit - increase vm area limits to reserve space for new variable
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@inc_sz: increment size
 *vvvv
 */
// int inc_vma_limit(struct pcb_t *caller, int vmaid, int inc_sz)
// {
//   struct vm_rg_struct * newrg = malloc(sizeof(struct vm_rg_struct));
//   int inc_amt = PAGING_PAGE_ALIGNSZ(inc_sz);
//   int incnumpage =  inc_amt / PAGING_PAGESZ;
//   struct vm_rg_struct *area = get_vm_area_node_at_brk(caller, vmaid, inc_sz, inc_amt);
//   struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

//   int old_end = cur_vma->vm_end;

//   /*Validate overlap of obtained region */
//   if (validate_overlap_vm_area(caller, vmaid, area->rg_start, area->rg_end) < 0)
//     return -1; /*Overlap and failed allocation */

//   /* TODO: Obtain the new vm area based on vmaid */
//   //cur_vma->vm_end... 
//   // inc_limit_ret...

//   if (vm_map_ram(caller, area->rg_start, area->rg_end, 
//                     old_end, incnumpage , newrg) < 0)
//     return -1; /* Map the memory to MEMRAM */

//   return 0;
// }

// #endif
