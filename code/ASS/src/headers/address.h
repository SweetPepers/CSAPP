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



/*
+-------+---------+-------+----------+-------------------+
| VPN3  |  VPN2   | VPN1  | VPN0     |                   |
+-------+---------+-------+-+--------+     VPO           |
|  TLBT                     | TLBI   |                   |
+-------------+-----------+----------+-------------------|
              |          PPN         |    PPO            |
              +----------------------+----------+--------+
              |       ct             |      ci  |  co    |    
              +----------------------+----------+--------+
*/

//Little Endia
typedef union{
  uint64_t address_value;

  //low -> high 

  //physical address: 52
  union {
    uint64_t paddr_value : PHYSICAL_ADDRESS_LENGTH;
    struct {
      uint64_t ppo : PHYSICAL_PAGE_OFFSET_LENGTH;
      uint64_t ppn : PHYSICAL_PAGE_NUMBER_LENGTH;
    };
  };

  //virtual address: 48
  struct {
    union {
      uint64_t vaddr_value : VIRTUAL_ADDRESS_LENGTH;
      struct {
        uint64_t vpo : VIRTUAL_PAGE_OFFSET_LENGTH;
        uint64_t vpn3 : VIRTUAL_PAGE_NUMBER_LENGTH;
        uint64_t vpn2 : VIRTUAL_PAGE_NUMBER_LENGTH;
        uint64_t vpn1 : VIRTUAL_PAGE_NUMBER_LENGTH;
        uint64_t vpn0 : VIRTUAL_PAGE_NUMBER_LENGTH;
      };
    };
  };

  

  // sram cache : 52
  struct {  
    uint64_t co : SRAM_CACHE_OFFSET_LENGTH;
    uint64_t ci : SRAM_CACHE_INDEX_LENGTH;
    uint64_t ct : SRAM_CACHE_TAG_LENGTH;
  };
   
}address_t;


#endif