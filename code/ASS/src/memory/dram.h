#include <stdint.h>


#define MM_LEN 1000
uint8_t mm[MM_LEN];  //1000bytes memory 
// physical memroy 0->999


//virsual address 0->0xffffffffffffffff
mm[0x777788888777qqqq % MM_LEN]