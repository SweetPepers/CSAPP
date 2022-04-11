#ifndef ADDRESS_GUARD
#define ADDRESS_GUARD

#include <stdint.h>


#ifndef CACHE_SIMULATION_VERIFICATION

#define SRAM_CACHE_TAG_LENGTH (4) //40 in real
#define SRAM_CACHE_INDEX_LENGTH (6)
#define SRAM_CACHE_OFFSET_LENGTH (6)

#endif

#define PHYSICAL_PAGE_OFFSET_LENGTH (12)
#define PHYSICAL_PAGE_NUMBER_LENGTH (4)   //40 in real
#define PHYSICAL_ADDRESS_LENGTH (16)      //52 in real

#define VIRTUAL_PAGE_OFFSET_LENGTH (12)
#define VIRTUAL_PAGE_NUMBER_LENGTH (9)  // 9+9+9+9 = 36
#define VIRTUAL_ADDRESS_LENGTH (48)

#define TLB_CACHE_OFFSET_LENGTH (12)
#define TLB_CACHE_INDEX_LENGTH (4)
#define TLB_CACHE_TAG_LENGTH (32)

/*
+-------+---------+-------+----------+-------------------+
| VPN1  |  VPN2   | VPN3  | VPN4     |                   |
+-------+---------+-------+-+--------+     VPO(12)       |
|  TLBT(32)                 | TLBI(4)|                   |
+-------------+-----------+----------+-------------------|
              |          PPN         |    PPO            |
              +----------------------+----------+--------+
              |         ct           |      ci  |  co    |    
              +----------------------+----------+--------+
*/

//Little Endia
typedef union{
  uint64_t address_value;

  //low -> high 

  //physical address: 16
  union {
    uint64_t paddr_value : PHYSICAL_ADDRESS_LENGTH;
    struct {
      uint64_t ppo : PHYSICAL_PAGE_OFFSET_LENGTH;
      uint64_t ppn : PHYSICAL_PAGE_NUMBER_LENGTH;
    };
  };

  //virtual address: 48
  union {
    uint64_t vaddr_value : VIRTUAL_ADDRESS_LENGTH;
    struct {
      uint64_t vpo  : VIRTUAL_PAGE_OFFSET_LENGTH;
      uint64_t vpn4 : VIRTUAL_PAGE_NUMBER_LENGTH;
      uint64_t vpn3 : VIRTUAL_PAGE_NUMBER_LENGTH;
      uint64_t vpn2 : VIRTUAL_PAGE_NUMBER_LENGTH;
      uint64_t vpn1 : VIRTUAL_PAGE_NUMBER_LENGTH;
    };
  };

  // sram cache : 16
  struct {  
    uint64_t co : SRAM_CACHE_OFFSET_LENGTH;
    uint64_t ci : SRAM_CACHE_INDEX_LENGTH;
    uint64_t ct : SRAM_CACHE_TAG_LENGTH;
  };


  //TLB cache 48
  struct{
    uint64_t tlbo : TLB_CACHE_OFFSET_LENGTH;  //virtual page offset 
    uint64_t tlbi : TLB_CACHE_INDEX_LENGTH ;  //TLB set index
    uint64_t tlbt : TLB_CACHE_TAG_LENGTH;   //TLB line tag 
  };
   
}address_t;


#endif