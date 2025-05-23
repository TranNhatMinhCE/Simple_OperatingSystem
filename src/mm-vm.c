//#ifdef MM_PAGING
/*
 * PAGING based Memory Management
 * Virtual memory module mm/mm-vm.c
 */

#include "string.h"
#include "mm.h"
#include <stdlib.h>
#include <stdio.h>


int expand_vma_sbrk(struct vm_area_struct *vma, unsigned long val) {

  if(vma->vm_id == 1)
    vma->sbrk -= val;
  else
    vma->sbrk += val;
  
  return 1;
}
/*enlist_vm_freerg_list - add new rg to freerg_list
 *@mm: memory region
 *@new_rgnode: new region
 *
 */
int enlist_vm_freerg_list(struct mm_struct *mm, int vmaid, struct vm_rg_struct* new_rgnode)
{
  struct vm_area_struct *cur_vma = get_vma_by_num(mm, vmaid);

  if (new_rgnode->rg_start > new_rgnode->rg_end)
    return -1;

  /* Enlist the new region */
  new_rgnode->rg_next = cur_vma->vm_freerg_list;
  cur_vma->vm_freerg_list = new_rgnode;

  return 0;
}

/*get_vma_by_num - get vm area by numID
 *@mm: memory region
 *@vmaid: ID vm area to alloc memory region
 *
 */
struct vm_area_struct *get_vma_by_num(struct mm_struct *mm, int vmaid)
{
  struct vm_area_struct *pvma= mm->mmap;

  if(mm->mmap == NULL)
    return NULL;

  int vmait = 0;
  
  while (vmait < vmaid)
  {
    if(pvma == NULL)
	  return NULL;

    vmait++;
    pvma = pvma->vm_next;
  }

  return pvma;
}

/*get_symrg_byid - get mem region by region ID
 *@mm: memory region
 *@rgid: region ID act as symbol index of variable
 *
 */
struct vm_rg_struct *get_symrg_byid(struct mm_struct *mm, int rgid)
{
  if(rgid < 0 || rgid > PAGING_MAX_SYMTBL_SZ)
    return NULL;

  return &mm->symrgtbl[rgid];
}

/*__alloc - allocate a region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size 
 *@alloc_addr: address of allocated memory region
 *
 */
int __alloc(struct pcb_t *caller, int vmaid, int rgid, int size, int *alloc_addr)
{
  /*Allocate at the toproof */
  struct vm_rg_struct rgnode;

  /* TODO: commit the vmaid */
  // rgnode.vmaid

  if (get_free_vmrg_area(caller, vmaid, size, &rgnode) == 0)
  {
    caller->mm->symrgtbl[rgid].rg_start = rgnode.rg_start;
    caller->mm->symrgtbl[rgid].rg_end = rgnode.rg_end;
    caller->mm->symrgtbl[rgid].vmaid = rgnode.vmaid;

    *alloc_addr = rgnode.rg_start;

    return 0;
  }

  /* TODO: get_free_vmrg_area FAILED handle the region management (Fig.6)*/

  /* TODO retrive current vma if needed, current comment out due to compiler redundant warning*/
  /*Attempt to increate limit to get space */
  //struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
 
  int inc_limit_ret;

  /* TODO INCREASE THE LIMIT */
  inc_vma_limit(caller, vmaid, size, &inc_limit_ret);

  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
  // unsigned long old_sbrk;

  if(inc_limit_ret > size) {
    struct vm_rg_struct* new_rgnode = init_vm_rg(cur_vma->sbrk, (inc_limit_ret - size), cur_vma->vm_id);
    expand_vma_sbrk(cur_vma, (inc_limit_ret - size));
    enlist_vm_freerg_list(caller->mm, vmaid, new_rgnode);
  }
  /* TODO: commit the limit increment */
  struct vm_rg_struct* new_var = init_vm_rg(cur_vma->sbrk, size, cur_vma->vm_id);;
  expand_vma_sbrk(cur_vma, size);
  caller->mm->symrgtbl[rgid] = *new_var;

  free(new_var);

  /* TODO: commit the allocation address */
  *alloc_addr = caller->mm->symrgtbl[rgid].rg_start;

  return 0;
}

/*__free - remove a region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size 
 *
 */
int __free(struct pcb_t *caller, int rgid)
{
  struct vm_rg_struct* rgnode = malloc(sizeof(struct vm_rg_struct));

  if(rgid < 0 || rgid >= PAGING_MAX_SYMTBL_SZ)
    return -1;

  /* TODO: Manage the collect freed region to freerg_list */

  rgnode->rg_start = caller->mm->symrgtbl[rgid].rg_start;
  rgnode->rg_end = caller->mm->symrgtbl[rgid].rg_end;
  rgnode->vmaid = caller->mm->symrgtbl[rgid].vmaid;

  caller->mm->symrgtbl[rgid].rg_end = caller->mm->symrgtbl[rgid].rg_start = 0;
  caller->mm->symrgtbl[rgid].vmaid = 0;

  /*enlist the obsoleted memory region */
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, rgnode->vmaid);
  enlist_vm_rg_node(&cur_vma->vm_freerg_list, rgnode);
    
  printf("mm-vm, 150, _free: \n");
  printf("- region % d start: %ld \n", rgid, rgnode->rg_start);
  printf("- region % d end: %ld \n", rgid, rgnode->rg_end);

  return 0;
}

/*pgalloc - PAGING-based allocate a region memory
 *@proc:  Process executing the instruction
 *@size: allocated size 
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */
int pgalloc(struct pcb_t *proc, uint32_t size, uint32_t reg_index)
{
  int addr;

  printf("Lenh pgalloc: rdid = %d, size = %d", reg_index, size);

  /* By default using vmaid = 0 */
  return __alloc(proc, 0, reg_index, size, &addr);
}

/*pgmalloc - PAGING-based allocate a region memory
 *@proc:  Process executing the instruction
 *@size: allocated size 
 *@reg_index: memory region ID (used to identify vaiable in symbole table)
 */
int pgmalloc(struct pcb_t *proc, uint32_t size, uint32_t reg_index)
{
  int addr;

  /* By default using vmaid = 1 */
  return __alloc(proc, 1, reg_index, size, &addr);
}

/*pgfree - PAGING-based free a region memory
 *@proc: Process executing the instruction
 *@size: allocated size 
 *@reg_index: memory region ID (used to identify variable in symbole table)
 */

int pgfree_data(struct pcb_t *proc, uint32_t reg_index)
{
  printf("mm-vm, 207, pgfree_datadata: \n");
  return __free(proc, reg_index);
}

/*pg_getpage - get the page in ram
 *@mm: memory region
 *@pagenum: PGN
 *@framenum: return FPN
 *@caller: caller
 *
 */
int pg_getpage(struct mm_struct *mm, int pgn, int *fpn, struct pcb_t *caller)
{
  uint32_t pte = mm->pgd[pgn];

  if(!PAGING_PTE_PAGE_PRESENT(pte)) return -1;
 
  if (PAGING_PTE_PAGE_SWAPPED(pte))
  { /* Page is not online, make it actively living */ 
    int tgtfpn = PAGING_PTE_SWP(pte);//the target frame storing our variable
    
    int vic_fpn;
    if(MEMPHY_get_freefp(caller->mram, &vic_fpn) != 0) {
      /* TODO: Play with your paging theory here */
      /* Find victim page */
      int vicpgn;
      if(find_victim_page(caller->mm, &vicpgn) == -1) return -1;
      
      uint32_t vic_pte = mm->pgd[vicpgn];
      vic_fpn = PAGING_PTE_FPN(vic_pte);

      /* Get free frame in MEMSWP */
      int swpfpn;
      if(MEMPHY_get_freefp(caller->active_mswp, &swpfpn) == -1) return -1;

      // pte_set_swap(&mm->pgd[vicpgn],
      /* Do swap frame from MEMRAM to MEMSWP and vice versa*/
      /* Copy victim frame to swap */
      
      __swap_cp_page(caller->mram, vic_fpn, caller->active_mswp, swpfpn);
      
      /* Update page table */
      pte_set_swap(&mm->pgd[vicpgn], 0, swpfpn);
    }
    
    /* Copy target frame from swap to mem */
    __swap_cp_page(caller->active_mswp, tgtfpn, caller->mram, vic_fpn);
    
    MEMPHY_put_freefp(caller->active_mswp, tgtfpn);

    /* Update page table */
    pte_set_fpn(&mm->pgd[pgn], vic_fpn);


    enlist_pgn_node(&caller->mm->fifo_pgn, pgn);
  }

  *fpn = PAGING_PTE_FPN(mm->pgd[pgn]);

  return 0;
}

/*pg_getval - read value at given offset
 *@mm: memory region
 *@addr: virtual address to acess 
 *@value: value
 *
 */
int pg_getval(struct mm_struct *mm, int addr, BYTE *data, struct pcb_t *caller)
{
  int pgn = PAGING_PGN(addr);
  int off = PAGING_OFFST(addr);
  int fpn;

  /* Get the page to MEMRAM, swap from MEMSWAP if needed */
  if(pg_getpage(mm, pgn, &fpn, caller) != 0) 
    return -1; /* invalid page access */

  int phyaddr = (fpn << PAGING_ADDR_FPN_LOBIT) + off;

  MEMPHY_read(caller->mram, phyaddr, data);

  printf("mm-vm, 279, pg_getval: \n");
  printf("- addr: %d\n", addr);
  printf("- data: %d \n", data);

  return 0;
}

/*pg_setval - write value to given offset
 *@mm: memory region
 *@addr: virtual address to acess 
 *@value: value
 *
 */
int pg_setval(struct mm_struct *mm, int addr, BYTE value, struct pcb_t *caller)
{
  int pgn = PAGING_PGN(addr);
  int off = PAGING_OFFST(addr);
  int fpn;

  /* Get the page to MEMRAM, swap from MEMSWAP if needed */
  if(pg_getpage(mm, pgn, &fpn, caller) != 0) 
    return -1; /* invalid page access */

  int phyaddr = (fpn << PAGING_ADDR_FPN_LOBIT) + off;

  MEMPHY_write(caller->mram, phyaddr, value);

   return 0;
}

/*__read - read value in region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@offset: offset to acess in memory region 
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size 
 *
 */
int __read(struct pcb_t *caller, int rgid, int offset, BYTE *data)
{
  printf("mm-vm, 331, _read: \n");
  printf("* rgid = ,%d, offset = %d\n", rgid, offset);

  struct vm_rg_struct *currg = get_symrg_byid(caller->mm, rgid);
  int vmaid = currg->vmaid;

  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

  if(currg == NULL || cur_vma == NULL) /* Invalid memory identify */
	  return -1;
  
  if(currg->rg_start == currg->rg_end) {
    printf("ERROR: region %d haven't been allocated\n", rgid);
    return -1;
  }

  pg_getval(caller->mm, currg->rg_start + offset, data, caller);

  return 0;
}


/*pgwrite - PAGING-based read a region memory */
int pgread(
		struct pcb_t * proc, // Process executing the instruction
		uint32_t source, // Index of source register
		uint32_t offset, // Source address = [source] + [offset]
		uint32_t destination) 
{
  BYTE data;
  int val = __read(proc, source, offset, &data);
  
  destination = (uint32_t) data;

  int vmaid =  proc->mm->symrgtbl[source].vmaid;

#ifdef IODUMP
  printf("read region=%d offset=%d value=%d\n", source, offset, data);
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1, vmaid); //print max TBL
#endif
  MEMPHY_dump(proc->mram);
#endif

  return val;
}

/*__write - write a region memory
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@offset: offset to acess in memory region 
 *@rgid: memory region ID (used to identify variable in symbole table)
 *@size: allocated size 
 *
 */
int __write(struct pcb_t *caller, int rgid, int offset, BYTE value)
{
  struct vm_rg_struct *currg = get_symrg_byid(caller->mm, rgid);
  int vmaid = currg->vmaid;

  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);
  
  if(currg == NULL || cur_vma == NULL) /* Invalid memory identify */
	  return -1;

  pg_setval(caller->mm, currg->rg_start + offset, value, caller);

  return 0;
}

/*pgwrite - PAGING-based write a region memory */
int pgwrite(
		struct pcb_t * proc, // Process executing the instruction
		BYTE data, // Data to be wrttien into memory
		uint32_t destination, // Index of destination register
		uint32_t offset)
{
  int vmaid =  proc->mm->symrgtbl[destination].vmaid;

#ifdef IODUMP
  printf("write region=%d offset=%d value=%d\n", destination, offset, data);
#ifdef PAGETBL_DUMP
  print_pgtbl(proc, 0, -1, vmaid); //print max TBL
#endif
  MEMPHY_dump(proc->mram);
#endif

  return __write(proc, destination, offset, data);
}


/*free_pcb_memphy - collect all memphy of pcb
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@incpgnum: number of page
 */
int free_pcb_memph(struct pcb_t *caller)
{
  int pagenum, fpn;
  uint32_t pte;

  for(pagenum = 0; pagenum < PAGING_MAX_PGN; pagenum++)
  {
    pte = caller->mm->pgd[pagenum];

    if (PAGING_PTE_PAGE_PRESENT(pte)) // Chu y
    { 
      if(!PAGING_PTE_PAGE_SWAPPED(pte)) {
        fpn = PAGING_PTE_FPN(pte);
        MEMPHY_put_freefp(caller->mram, fpn);
      } 
      else {
        fpn = PAGING_PTE_SWP(pte);
        MEMPHY_put_freefp(caller->active_mswp, fpn);    
      }
    }
  }

  return 0;
}

/*get_vm_area_node - get vm area for a number of pages
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@incpgnum: number of page
 *@vmastart: vma end
 *@vmaend: vma end
 *
 */
struct vm_rg_struct* get_vm_area_node_at_brk(struct pcb_t *caller, int vmaid, int size, int alignedsz)
{
  struct vm_rg_struct* newrg;
  
  /* TODO retrive current vma to obtain newrg, current comment out due to compiler redundant warning*/
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

  newrg = malloc(sizeof(struct vm_rg_struct));

  //TODO: update the newrg boundary
  // newrg->rg_start = get_vma_sbrk(cur_vma);
  // newrg->rg_end = newrg->rg_start + alignedsz;
  if (vmaid == 1) {
    newrg->rg_start = cur_vma->sbrk - alignedsz;
    newrg->rg_end = cur_vma->sbrk;
  }
  else {
    newrg->rg_start = cur_vma->sbrk;
    newrg->rg_end = cur_vma->sbrk + alignedsz;
  }
  newrg->vmaid = vmaid;
  
  return newrg;
}

/*validate_overlap_vm_area
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@vmastart: vma end
 *@vmaend: vma end
 *
 */
int validate_overlap_vm_area(struct pcb_t *caller, int vmaid, int vmastart, int vmaend)
{
  /* TODO validate the planned memory area is not overlapped */
  
  struct vm_area_struct* other_vma = caller->mm->mmap;

  while (other_vma != NULL) {
    if (other_vma->vm_id != vmaid 
    && ((other_vma->vm_start <= vmaend && vmaend <= other_vma->vm_end)
    || (other_vma->vm_start <= vmastart && vmastart <= other_vma->vm_end))) 
      return -1;

    other_vma = other_vma->vm_next;
  }
  
  return 0;
}

/*inc_vma_limit - increase vm area limits to reserve space for new variable
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@inc_sz: increment size 
 *@inc_limit_ret: increment limit return
 *
 */
int inc_vma_limit(struct pcb_t *caller, int vmaid, int inc_sz, int* inc_limit_ret)
{
  struct vm_rg_struct* newrg = malloc(sizeof(struct vm_rg_struct));
  newrg->vmaid = vmaid; 
  struct vm_area_struct* cur_vma = get_vma_by_num(caller->mm, vmaid);
  
  int old_sbrk = cur_vma->sbrk;
  
  int rm_old_pg_sz, inc_amt;
  
  // if(vmaid == 1)
  //   rm_old_pg_sz = PAGING_OFFST(old_sbrk); //old_sbrk - PAGING_PGN(old_sbrk) * PAGING_PAGESZ;
  // else
  //   rm_old_pg_sz = PAGING_PAGESZ - PAGING_OFFST(old_sbrk);
  if (vmaid == 1)
    rm_old_pg_sz = old_sbrk - cur_vma->vm_end; 
  else
    rm_old_pg_sz = cur_vma->vm_end - old_sbrk;

  if(rm_old_pg_sz > inc_sz) {
    *inc_limit_ret = inc_sz;
    return 0;
  }

  inc_amt = PAGING_PAGE_ALIGNSZ(inc_sz - rm_old_pg_sz);

  if(inc_amt < PAGING_PAGE_ALIGNSZ(inc_sz))
    *inc_limit_ret = inc_sz;
  else
    *inc_limit_ret = inc_sz + rm_old_pg_sz;

  struct vm_rg_struct *area = get_vm_area_node_at_brk(caller, vmaid, inc_sz, inc_amt);
  /*Validate overlap of obtained region */
  if (validate_overlap_vm_area(caller, vmaid, area->rg_start, area->rg_end) < 0)
    return -1; /*Overlap and failed allocation */

  /* TODO: Obtain the new vm area based on vmaid */
  int mapstart = cur_vma->vm_end;
  if(vmaid == 1) {
    cur_vma->vm_end = cur_vma->vm_end - inc_amt;
    mapstart = cur_vma->vm_end;
  }
  else
    cur_vma->vm_end = cur_vma->vm_end + inc_amt;
  
  int incnumpage =  inc_amt / PAGING_PAGESZ;
  if (vm_map_ram(caller, vmaid, area->rg_start, area->rg_end, mapstart, incnumpage , newrg) < 0)
    return -1; /* Map the memory to MEMRAM */

  // printf("mm-vm, 517, inc_vma_limit: \n");
  // printf("- Area % d start: %ld \n", vmaid, cur_vma->vm_start);
  // printf("- Area % d end: %ld \n", vmaid, cur_vma->vm_end);
  
  return 0;

}

/*find_victim_page - find victim page
 *@caller: caller
 *@pgn: return page number
 *
 */
int find_victim_page(struct mm_struct *mm, int *retpgn) 
{
  struct pgn_t *pg = mm->fifo_pgn;

  /* TODO: Implement the theorical mechanism to find the victim page */
  // FIFO
  if(pg == NULL) return -1;

  if(pg->pg_next == NULL) {
    *retpgn = pg->pgn; 
    free(pg);
    mm->fifo_pgn = NULL;
    
    return 0;
  }

  while(pg->pg_next->pg_next != NULL) 
    pg = pg->pg_next;

  *retpgn = pg->pg_next->pgn; 
  free(pg->pg_next);
  pg->pg_next = NULL;

  return 0;
}

/*get_free_vmrg_area - get a free vm region
 *@caller: caller
 *@vmaid: ID vm area to alloc memory region
 *@size: allocated size 
 *
 */
int get_free_vmrg_area(struct pcb_t *caller, int vmaid, int size, struct vm_rg_struct *newrg)
{
  
  struct vm_area_struct *cur_vma = get_vma_by_num(caller->mm, vmaid);

  struct vm_rg_struct *rgit = cur_vma->vm_freerg_list;

  if (rgit == NULL)
    return -1;

  /* Probe unintialized newrg */
  newrg->rg_start = newrg->rg_end = -1;

  /* Traverse on list of free vm region to find a fit space */
  while (rgit != NULL && rgit->vmaid == vmaid)
  {
    if (rgit->rg_start + size <= rgit->rg_end)
    { /* Current region has enough space */
      newrg->rg_start = rgit->rg_start;
      newrg->rg_end = rgit->rg_start + size;

      /* Update left space in chosen region */
      if (rgit->rg_start + size < rgit->rg_end)
      {
        rgit->rg_start = rgit->rg_start + size;
      }
      else
      { /*Use up all space, remove current node */
        /*Clone next rg node */
        struct vm_rg_struct *nextrg = rgit->rg_next;

        /*Cloning */
        if (nextrg != NULL)
        {
          rgit->rg_start = nextrg->rg_start;
          rgit->rg_end = nextrg->rg_end;

          rgit->rg_next = nextrg->rg_next;

          free(nextrg);
        }
        else
        { /*End of free list */
          rgit->rg_start = rgit->rg_end;	//dummy, size 0 region
          rgit->rg_next = NULL;
        }
      }
    }
    else
    {
      rgit = rgit->rg_next;	// Traverse next rg
    }
  }

 if(newrg->rg_start == -1) {// new region not found
    printf("khong co region trong!\n");
    return -1;
 }

  printf("region trong duoc cap phat la:\n");
  printf("- start = %d\n", newrg->rg_start);
  printf("- end = %d\n", newrg->rg_end);

 return 0;
}

//#endif
