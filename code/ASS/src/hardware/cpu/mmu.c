#include "headers/cpu.h"
#include "headers/common.h"
#include "headers/memory.h"
#include "headers/address.h"
#include <assert.h>
#include <stdio.h>


// ---------------------------------------//
//           TLB cache struct             //
// ---------------------------------------//

#define NUM_TLB_CACHE_LINE_PER_SET (8)

typedef struct{
  int valid;
  uint64_t tag;
  uint64_t ppn;
}tlb_cacheline_t; 

typedef struct{
  tlb_cacheline_t lines[NUM_TLB_CACHE_LINE_PER_SET]; 
}tlb_cacheset_t;


typedef struct{
  tlb_cacheset_t sets[1<<TLB_CACHE_INDEX_LENGTH];
}tlb_cache_t;

static tlb_cache_t mmu_tlb;  //init to .bss  -> INVALID


static uint16_t page_walk(uint64_t vaddr_value);
static void page_fault_handler(pte4_t *pte, address_t vaddr);
int swap_in(uint64_t daddr, uint64_t ppn);
int swap_out(uint64_t daddr, uint64_t ppn);
static int read_tlb(uint64_t vaddr_value, uint64_t *paddr_value_ptr, int *free_tlb_line_index);
static int write_tlb(uint64_t vaddr_value, uint64_t paddr, int free_tlb_line_index);


uint64_t va2pa(uint64_t vaddr, core_t *cr){
  // use page table as va2pa
  uint64_t paddr = 0;

  #ifdef USE_TLB_HARDWARE
  int free_tlb_line_index = -1;
  int tlb_hit = read_tlb(vaddr, &paddr, &free_tlb_line_index);
  //TODO add flag to read tlb failed
  if(tlb_hit == 1){
    //TLB read hit
    return paddr;
  }
  // TLB read miss
  #endif
  
  //assume that page_walk is consuming much time
  paddr =  page_walk(vaddr);

  #ifdef USE_TLB_HARDWARE
  //refresh tlb
  //TODO : check if this paddr from is a legal address
  if(paddr != 0){
    //TLB write
    if(write_tlb(vaddr, paddr,free_tlb_line_index) == 1){
      return paddr;
    }
    
    
  }
  #endif
  return paddr;
}


static int read_tlb(uint64_t vaddr_value, uint64_t *paddr_value_ptr, int *free_tlb_line_index){
  address_t addr;
  addr.address_value = vaddr_value;

  tlb_cacheset_t * set = &mmu_tlb.sets[addr.tlbi];
  *free_tlb_line_index = -1;
  
  for(int i =0;i<NUM_TLB_CACHE_LINE_PER_SET;++i){
    
    tlb_cacheline_t *line = &set->lines[i];

    //find tlb read hit
    if(line->valid == 0){
      *free_tlb_line_index = i;
    }
    if(line->tag == addr.tlbt && line->valid != 0){
      *paddr_value_ptr = line->ppn;
      return 1;
    }

  }

  //tib miss
  paddr_value_ptr = NULL;
  
  return 0;
}


static int write_tlb(uint64_t vaddr_value, uint64_t paddr_value, int free_tlb_line_index){
  address_t vaddr = {
    .address_value = vaddr_value
  };

  address_t paddr = {
    .address_value = paddr_value
  };

  tlb_cacheset_t * set = &mmu_tlb.sets[vaddr.tlbi];
  
  int tlb_victim = (free_tlb_line_index >= 0 && free_tlb_line_index < NUM_TLB_CACHE_LINE_PER_SET)?
                    free_tlb_line_index : random() % NUM_TLB_CACHE_LINE_PER_SET ;
  //free : use free
  //no free TLB cacheline, select one RANDOM victim

  tlb_cacheline_t *line = &set->lines[tlb_victim];
  line->valid = 1;
  line->ppn = paddr.ppn;
  line->tag = vaddr.tlbt;
  return 1;
}

//intput  - virtual address
//output  - physical address
//cr3->pgd->pud->pmd
static uint16_t page_walk(uint64_t vaddr_value){
  address_t vaddr = {
    .vaddr_value = vaddr_value
  };
  int pagetable_size = PAGE_TABLE_ENTRY_NUMBER * sizeof(pte123_t);  //4kB
  //CR3 register's value  is malloced on the heap of the simulator
  // pgd global up  median 
  pte123_t *pgd =  (pte123_t *)cpu_controls.cr3; //pgd
  assert(pgd != NULL);
  
  if(pgd[vaddr.vpn1].present == 1){
    pte123_t *pud = (pte123_t*)(pgd[vaddr.vpn1].paddr);

    if(pud[vaddr.vpn2].present == 1){
      pte123_t *pmd = (pte123_t*)(pud[vaddr.vpn2].paddr);

      if(pmd[vaddr.vpn3].present == 1){
        //find pt ppn
        pte4_t *pt = (pte4_t*)(pmd[vaddr.vpn3].paddr);
        
        if(pt[vaddr.vpn4].present == 1){
          //find page table entry
          address_t paddr = {
            .ppn = pt[vaddr.vpn4].ppn,
            .vpo = vaddr.vpo
          };

          return paddr.paddr_value;
        }else{
          //page table entry not exists
          #ifdef DEBUG_PAGE_WALk
          printf("page walk level 4:pt[%lx].present == 0\n\t malloc new page table for it",vaddr.vpn4);
          #endif
          //search paddr from main memory and disk
          //TODO: raise exception 14(page fault) here
          //switch privilege from user mode(ring 3)to kernel mode (ring 0)

          page_fault_handler(&(pt[vaddr.vpn4]), vaddr);
          /*
          pte4_t *pte = malloc(pagetable_size);
          memset(pte, 0, pagetable_size);
          //set pagetable entry
          pt[vaddr.vpn4].present = 1;
          pt[vaddr.vpn4].pte_value = (uint64_t)pte;
          //TODO:
          //page fault
          //map the physical page and virtual page
          exit(0);
          */
        }
        

      }else{
        //lv 3 not exists   page fault
        #ifdef DEBUG_PAGE_WALk
        printf("page walk level 3:pmd[%lx].present == 0\n\t malloc new page table for it",vaddr.vpn3);
        #endif
        
        pte4_t *pt = malloc(pagetable_size);
        memset(pt, 0, pagetable_size);
        
        //set pagetable entry
        pmd[vaddr.vpn3].present = 1;
        pmd[vaddr.vpn3].paddr = (uint64_t)pt;
        //TODO:
        //page fault
        //map the physical page and virtual page

        exit(0);
      }

    }else{
      #ifdef DEBUG_PAGE_WALk
      printf("page walk level 2:pud[%lx].present == 0\n\t malloc new page table for it",vaddr.vpn2);
      #endif
      pte123_t *pmd = malloc(pagetable_size);
      memset(pmd, 0, pagetable_size);
      
      //set pagetable entry
      pud[vaddr.vpn2].present = 1;
      pud[vaddr.vpn2].paddr = (uint64_t)pmd;
      //TODO:
      //page fault
      //map the physical page and virtual page
      exit(0);
    }

  }else{
    //pud - level 2 not exists
    #ifdef DEBUG_PAGE_WALk
    printf("page walk level 1:pgd[%lx].present == 0\n\t malloc new page table for it",vaddr.vpn1);
    #endif
    
    pte123_t *pud = malloc(pagetable_size);
    memset(pud, 0, pagetable_size);
    
    //set pagetable entry
    pgd[vaddr.vpn1].present = 1;
    pgd[vaddr.vpn1].paddr = (uint64_t)pud;
    //TODO:
    //page fault
    //map the physical page and virtual page
    exit(0);

  }

}


//input vaddr:
//input pte:
static void page_fault_handler(pte4_t *pte, address_t vaddr){
  //search one victim physical page to swap to disk
  //this is the selected ppn for vaddr
  assert(pte->present == 0);
  int ppn = -1;
  pte4_t *victim = NULL;
  uint64_t daddr = 0xffffffffffffffff;
  
  //1 try to request one free physical page from dram
  //kernel's responsibility

  //reverse mapping   vpn->ppn
  for(int i = 0;i<MAX_NUM_PHYSICAL_PAGE;i++){
    if(page_map[i].pte4->present == 0){
      printf("PageFault: use free ppn %d\n", i);


      ppn = i;
      page_map[ppn].allocated = 1;
      page_map[ppn].dirty = 0;
      page_map[ppn].time = 0; // most recently used

      page_map[ppn].pte4 = pte;

      pte->present = 1;
      pte->ppn = ppn;
      pte->dirty = 0;

      return;

    }
  }
  

  //2.  no free physical page : select one CLEAN page(LRU) and overwrite
  //in this case, there is no DRAM-DISK translation
  int lru_ppn = -1;
  int lru_time = -1;
  for(int i = 0;i<MAX_NUM_PHYSICAL_PAGE;i++){
    if(page_map[i].dirty == 0 && lru_time<page_map[i].time){
      lru_time = page_map[i].time;
      lru_ppn = i;
    }
  }

  if(lru_ppn != -1 && lru_ppn<MAX_NUM_PHYSICAL_PAGE){
      //find
      /*

      present 1 : page_map
              0 : pte

      */
      ppn = lru_ppn;

      victim = page_map[ppn].pte4;
      victim->pte_value = 0; //reset
      victim->present = 0;
      victim->daddr = page_map[ppn].daddr;
      
      //load page from disk to physical memory first
      daddr = pte->daddr;
      swap_in(daddr, ppn);

      pte->pte_value = 0;
      pte->present = 1;
      pte->ppn = ppn;
      pte->dirty = 0;

      page_map[ppn].allocated = 1;
      page_map[ppn].dirty = 0;
      page_map[ppn].time = 0;
      page_map[ppn].pte4 = pte;
      page_map[ppn].daddr = daddr;

      return;
  }


  //3. no clean nor free
  //select one LRU victim  wirteback(swap out) the DIRTY victim to disk
  
  lru_ppn = -1;
  lru_time = -1;
  for(int i = 0;i<MAX_NUM_PHYSICAL_PAGE;i++){
    if(page_map[i].dirty == 0 && lru_time<page_map[i].time){
      lru_time = page_map[i].time;
      lru_ppn = i;
    }  
  }

  assert(lru_ppn >=0 && lru_ppn < MAX_NUM_PHYSICAL_PAGE);

  //find
  /*

  present 1 : page_map
          0 : disk

  */
  ppn = lru_ppn;
  victim = page_map[ppn].pte4;
  //write back
  swap_out(page_map[ppn].daddr, ppn);
  
  victim->pte_value = 0; //reset
  victim->present = 0;
  victim->daddr = page_map[ppn].daddr;
  
  //load page from disk to physical memory first
  daddr = pte->daddr;
  swap_in(daddr, ppn);

  pte->pte_value = 0;
  pte->present = 1;
  pte->ppn = ppn;
  pte->dirty = 0;

  page_map[ppn].allocated = 1;
  page_map[ppn].dirty = 0;
  page_map[ppn].time = 0;
  page_map[ppn].pte4 = pte;
  page_map[ppn].daddr = daddr;

  return;
}



