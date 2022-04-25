#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include "headers/allocator.h"
#include "headers/algorithm.h"

static int internal_heap_init();
static uint64_t internal_malloc(uint32_t size);
static void internal_free(uint64_t payload_vaddr);

/* ------------------------------------- */
/*  Implementation of the Interfaces     */
/* ------------------------------------- */

#ifdef REDBLACK_TREE

int heap_init()
{
  return internal_heap_init();
}

uint64_t mem_alloc(uint32_t size)
{
  return internal_malloc(size);
}

void mem_free(uint64_t payload_vaddr)
{
  internal_free(payload_vaddr);
}

#ifdef DEBUG_MALLOC
void on_sigabrt(int signum)
{
  // like a try-catch for the asserts
  printf("%s\n", debug_message);
  print_heap();
  exit(0);
}
#endif

#endif

#define MIN_REDBLACK_TREE_BLOCKSIZE (20)

#define BLACK (0)
#define RED (1)

static uint64_t redblack_tree_root_node = NIL;

/* ------------------------------------- */
/*  Operations for Tree Block Structure  */
/* ------------------------------------- */

static int destruct_node(uint64_t header_vaddr)
{
  // do nothing here
  return 1;
}

static int compare_nodes(uint64_t first, uint64_t second)
{
  return !(first == second);
}

static uint32_t get_redblack_tree_parent(uint64_t header_vaddr)
{
  return get_field32_block_ptr(header_vaddr, MIN_REDBLACK_TREE_BLOCKSIZE, 4);
}

static uint32_t get_redblack_tree_left(uint64_t header_vaddr)
{
  return get_field32_block_ptr(header_vaddr, MIN_REDBLACK_TREE_BLOCKSIZE, 8);
}

static uint32_t get_redblack_tree_right(uint64_t header_vaddr)
{
  return get_field32_block_ptr(header_vaddr, MIN_REDBLACK_TREE_BLOCKSIZE, 12);
}

static uint32_t get_redblack_tree_color(uint64_t header_vaddr)
{
  if (header_vaddr == NIL)
  {
    // default BLACK
    return BLACK;
  }

  assert(get_prologue() <= header_vaddr && header_vaddr <= get_epilogue());
  assert((header_vaddr & 0x3) == 0x0); // header & footer should be 4 bytes alignment

  uint32_t header_value = *(uint32_t *)&heap[header_vaddr];
  return ((header_value >> 1) & 0x1);
}

static void set_redblack_tree_parent(uint64_t header_vaddr, uint64_t prev_vaddr)
{
  set_field32_block_ptr(header_vaddr, prev_vaddr, MIN_REDBLACK_TREE_BLOCKSIZE, 4);
}

static void set_redblack_tree_left(uint64_t header_vaddr, uint64_t left_vaddr)
{
  set_field32_block_ptr(header_vaddr, left_vaddr, MIN_REDBLACK_TREE_BLOCKSIZE, 8);
}

static void set_redblack_tree_right(uint64_t header_vaddr, uint64_t right_vaddr)
{
  set_field32_block_ptr(header_vaddr, right_vaddr, MIN_REDBLACK_TREE_BLOCKSIZE, 12);
}

static void set_redblack_tree_color(uint64_t header_vaddr, uint32_t color)
{
  if (header_vaddr == NIL)
  {
    return;
  }

  assert(color == BLACK || color == RED);
  assert(get_prologue() <= header_vaddr && header_vaddr <= get_epilogue());
  assert((header_vaddr & 0x3) == 0x0); // header & footer should be 4 bytes alignment

  *(uint32_t *)&heap[header_vaddr] &= 0xFFFFFFFD;
  *(uint32_t *)&heap[header_vaddr] |= ((color & 0x1) << 1);
}

static rbtree_node_interface i_node =
    {
        .construct_node = NULL,
        .destruct_node = &destruct_node,
        .compare_nodes = &compare_nodes,
        .get_parent = &get_redblack_tree_parent,
        .set_parent = &set_redblack_tree_parent,
        .get_leftchild = &get_redblack_tree_left,
        .set_leftchild = &set_redblack_tree_left,
        .get_rightchild = &get_redblack_tree_right,
        .set_rightchild = &set_redblack_tree_right,
        .get_color = &get_redblack_tree_color,
        .set_color = &set_redblack_tree_color,
};

/* ------------------------------------- */
/*  Operations for Red-Black Tree        */
/* ------------------------------------- */

static int update_root(rbtree_internal_t *this, uint64_t block_vaddr)
{
  if (this == NULL)
  {
    return 0;
  }

  assert(block_vaddr == NULL_ID || (get_firstblock() <= block_vaddr && block_vaddr <= get_lastblock()));
  assert(block_vaddr == NULL_ID || block_vaddr % 8 == 4);
  assert(block_vaddr == NULL_ID || get_blocksize(block_vaddr) >= MIN_REDBLACK_TREE_BLOCKSIZE);

  this->root = block_vaddr;
  return 1;
}

// The explicit free linked list
static rbtree_internal_t rbt;

static void redblack_tree_init()
{
  rbt.root = NULL_ID;
  rbt.update_root = &update_root;
}

static void redblack_tree_insert(uint64_t node_ptr)
{
  // BST for now
  bst_internal_insert(&rbt, &i_node, node_ptr);
}

static void redblack_tree_delete(uint64_t node_ptr)
{
  // BST for now
  // bst_internal_delete(&rbt, &i_node, node_ptr);
}

static uint64_t redblack_tree_search(uint32_t size)
{
  // BST for now
  return bst_internal_find_succ(&rbt, &i_node, (uint64_t)size);
}

/* ------------------------------------- */
/*  For Debugging                        */
/* ------------------------------------- */

/* ------------------------------------- */
/*  Implementation                       */
/* ------------------------------------- */

static int internal_heap_init()
{
  // reset all to 0
  for (int i = 0; i < HEAP_MAX_SIZE / 8; i += 8)
  {
    *(uint64_t *)&heap[i] = 0;
  }

  // heap_start_vaddr is the starting address of the first block
  // the payload of the first block is 8B aligned ([8])
  // so the header address of the first block is [8] - 4 = [4]
  heap_start_vaddr = 0;
  heap_end_vaddr = 4096;

  // set the prologue block
  uint64_t prologue_header = get_prologue();
  set_blocksize(prologue_header, 8);
  set_allocated(prologue_header, ALLOCATED);

  uint64_t prologue_footer = prologue_header + 4;
  set_blocksize(prologue_footer, 8);
  set_allocated(prologue_footer, ALLOCATED);

  // set the epilogue block
  // it's a footer only
  uint64_t epilogue = get_epilogue();
  set_blocksize(epilogue, 0);
  set_allocated(epilogue, ALLOCATED);

  // set the block size & allocated of the only regular block
  uint64_t first_header = get_firstblock();
  set_blocksize(first_header, 4096 - 4 - 8 - 4);
  set_allocated(first_header, FREE);
  set_redblack_tree_color(first_header, COLOR_BLACK);

  uint64_t first_footer = get_footer(first_header);
  set_blocksize(first_footer, 4096 - 4 - 8 - 4);
  set_allocated(first_footer, FREE);
  set_redblack_tree_color(first_footer, COLOR_BLACK);

#ifdef DEBUG_MALLOC
  // like a try-catch
  signal(SIGABRT, &on_sigabrt);
#endif

  set_redblack_tree_parent(first_header, NULL_ID);
  set_redblack_tree_left(first_header, NULL_ID);
  set_redblack_tree_right(first_header, NULL_ID);

  redblack_tree_init();
  redblack_tree_insert(first_header);

  return 1;
}

static uint64_t internal_malloc(uint32_t size)
{
  assert(0 < size && size < HEAP_MAX_SIZE - 4 - 8 - 4);

  uint64_t payload_vaddr = NIL;

  uint32_t request_blocksize = round_up(size, 8) + 4 + 4;
  request_blocksize = request_blocksize < MIN_REDBLACK_TREE_BLOCKSIZE ? MIN_REDBLACK_TREE_BLOCKSIZE : request_blocksize;

  // not thread safe

  // best fit from binary search tree
  uint64_t b = redblack_tree_search(request_blocksize);

  if (b != NIL)
  {
    uint32_t b_old_blocksize = get_blocksize(b);
    payload_vaddr = try_alloc_with_splitting(b, request_blocksize, MIN_REDBLACK_TREE_BLOCKSIZE);

    if (payload_vaddr != NIL)
    {
      uint32_t b_new_blocksize = get_blocksize(b);
      assert(b_new_blocksize <= b_old_blocksize);
      redblack_tree_delete(b);

      if (b_old_blocksize > b_new_blocksize)
      {
        // b has been splitted
        uint64_t a = get_nextheader(b);
        assert(get_allocated(a) == FREE);
        assert(get_blocksize(a) == b_old_blocksize - b_new_blocksize);
        redblack_tree_insert(a);
      }

#ifdef DEBUG_MALLOC
      check_heap_correctness();
#endif
      return payload_vaddr;
    }
    assert(0);
  }

  // when no enough free block for current heap
  // request a new free physical & virtual page from OS
  uint64_t old_last = get_lastblock();
  if (get_allocated(old_last) == FREE)
  {
    redblack_tree_delete(old_last);
  }

  payload_vaddr = try_extend_heap_to_alloc(request_blocksize, MIN_REDBLACK_TREE_BLOCKSIZE);

  uint64_t new_last = get_lastblock();
  if (get_allocated(new_last) == FREE)
  {
    redblack_tree_insert(new_last);
  }

#ifdef DEBUG_MALLOC
  check_heap_correctness();
#endif

  return payload_vaddr;
}

static void internal_free(uint64_t payload_vaddr)
{
  if (payload_vaddr == NIL)
  {
    return;
  }

  assert(get_firstblock() < payload_vaddr && payload_vaddr < get_epilogue());
  assert((payload_vaddr & 0x7) == 0x0);

  // request can be first or last block
  uint64_t req = get_header(payload_vaddr);
  uint64_t req_footer = get_footer(req); // for last block, it's 0

  uint32_t req_allocated = get_allocated(req);
  uint32_t req_blocksize = get_blocksize(req);
  assert(req_allocated == ALLOCATED); // otherwise it's free twice

  // block starting address of next & prev blocks
  uint64_t next = get_nextheader(req); // for req last block, it's 0
  uint64_t prev = get_prevheader(req); // for req first block, it's 0

  uint32_t next_allocated = get_allocated(next); // for req last, 1
  uint32_t prev_allocated = get_allocated(prev); // for req first, 1

  if (next_allocated == ALLOCATED && prev_allocated == ALLOCATED)
  {
    // case 1: *A(A->F)A*
    // ==> *AFA*
    set_allocated(req, FREE);
    set_allocated(req_footer, FREE);

    redblack_tree_insert(req);
#ifdef DEBUG_MALLOC
    check_heap_correctness();
#endif
  }
  else if (next_allocated == FREE && prev_allocated == ALLOCATED)
  {
    // case 2: *A(A->F)FA
    // ==> *AFFA ==> *A[FF]A merge current and next
    redblack_tree_delete(next);

    uint64_t one_free = merge_blocks_as_free(req, next);

    redblack_tree_insert(one_free);
#ifdef DEBUG_MALLOC
    check_heap_correctness();
#endif
  }
  else if (next_allocated == ALLOCATED && prev_allocated == FREE)
  {
    // case 3: AF(A->F)A*
    // ==> AFFA* ==> A[FF]A* merge current and prev
    redblack_tree_delete(prev);

    uint64_t one_free = merge_blocks_as_free(prev, req);

    redblack_tree_insert(one_free);
#ifdef DEBUG_MALLOC
    check_heap_correctness();
#endif
  }
  else if (next_allocated == FREE && prev_allocated == FREE)
  {
    // case 4: AF(A->F)FA
    // ==> AFFFA ==> A[FFF]A merge current and prev and next
    redblack_tree_delete(prev);
    redblack_tree_delete(next);

    uint64_t one_free = merge_blocks_as_free(merge_blocks_as_free(prev, req), next);

    redblack_tree_insert(one_free);
#ifdef DEBUG_MALLOC
    check_heap_correctness();
#endif
  }
  else
  {
#ifdef DEBUG_MALLOC
    printf("exception for free\n");
    exit(0);
#endif
  }
}



// #include <assert.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <stdint.h>
// #include "headers/allocator.h"
// #include "headers/algorithm.h"

// // Manage small blocks - 8 Bytes
// void small_list_init();
// void small_list_insert(uint64_t free_header);
// void small_list_delete(uint64_t free_header);
// linkedlist_internal_t small_list;
// void small_list_check_free_blocks();

// // Manage small blocks - 16 Bytes
// void explist_list_init();
// uint64_t explicit_list_search(uint64_t free_blocksize);
// void explicit_list_insert(uint64_t free_header);
// void explicit_list_delete(uint64_t free_header);
// linkedlist_internal_t explicit_list;
// uint64_t get_nextfree(uint64_t header_vaddr);
// void check_block16_correctness();

// void bst_internal_print(uint64_t node, rbtree_node_interface *i_node);

// /* ------------------------------------- */
// /*  Operations for Tree Block Structure  */
// /* ------------------------------------- */

// static int compare_nodes(uint64_t first, uint64_t second)
// {
//     return !(first == second);
// }

// static int is_null_node(uint64_t header_vaddr)
// {
//     if (get_firstblock() <= header_vaddr &&
//         header_vaddr <= get_lastblock() &&
//         header_vaddr % 8 == 4)
//     {
//         return 0;
//     }
//     return 1;
// }

// static uint64_t get_redblack_tree_parent(uint64_t header_vaddr)
// {
//     return get_field32_block_ptr(header_vaddr, 
//         MIN_REDBLACK_TREE_BLOCKSIZE, 4);
// }

// static uint64_t get_redblack_tree_left(uint64_t header_vaddr)
// {
//     return get_field32_block_ptr(header_vaddr, 
//         MIN_REDBLACK_TREE_BLOCKSIZE, 8);
// }

// static uint64_t get_redblack_tree_right(uint64_t header_vaddr)
// {
//     return get_field32_block_ptr(header_vaddr, 
//         MIN_REDBLACK_TREE_BLOCKSIZE, 12);
// }

// static rb_color_t get_redblack_tree_color(uint64_t header_vaddr)
// {
//     if (header_vaddr == NIL)
//     {
//         // default BLACK
//         return COLOR_BLACK;
//     }

//     assert(get_prologue() <= header_vaddr && 
//         header_vaddr <= get_epilogue());
//     assert(header_vaddr % 8 == 4);
//     assert(get_blocksize(header_vaddr) >= MIN_REDBLACK_TREE_BLOCKSIZE);

//     uint64_t footer_vaddr = get_footer(header_vaddr);
//     uint32_t footer_value = *(uint32_t *)&heap[footer_vaddr];
//     return (rb_color_t)((footer_value >> 1) & 0x1);
// }

// static uint64_t get_redblack_tree_key(uint64_t header_vaddr)
// {
//     uint32_t blocksize = get_blocksize(header_vaddr);
//     return blocksize;
// }

// static int set_redblack_tree_parent(uint64_t header_vaddr,
//     uint64_t prev_vaddr)
// {
//     set_field32_block_ptr(header_vaddr, prev_vaddr, 
//         MIN_REDBLACK_TREE_BLOCKSIZE, 4);
//     return 1;
// }

// static int set_redblack_tree_left(uint64_t header_vaddr, 
//     uint64_t left_vaddr)
// {
//     set_field32_block_ptr(header_vaddr, left_vaddr, 
//         MIN_REDBLACK_TREE_BLOCKSIZE, 8);
//     return 1;
// }

// static int set_redblack_tree_right(uint64_t header_vaddr, 
//     uint64_t right_vaddr)
// {
//     set_field32_block_ptr(header_vaddr, right_vaddr, 
//         MIN_REDBLACK_TREE_BLOCKSIZE, 12);
//     return 1;
// }

// static int set_redblack_tree_color(uint64_t header_vaddr, rb_color_t color)
// {
//     if (header_vaddr == NIL)
//     {
//         return 0;
//     }

//     assert(color == COLOR_BLACK || color == COLOR_RED);
//     assert(get_prologue() <= header_vaddr && 
//         header_vaddr <= get_epilogue());
//     assert(header_vaddr % 8 == 4);
//     assert(get_blocksize(header_vaddr) >= MIN_REDBLACK_TREE_BLOCKSIZE);

//     uint64_t footer_vaddr = get_footer(header_vaddr);
//     *(uint32_t *)&heap[footer_vaddr] &= 0xFFFFFFFD;
//     *(uint32_t *)&heap[footer_vaddr] |= ((color & 0x1) << 1);

//     return 1;
// }

// static int set_redblack_tree_key(uint64_t header_vaddr, uint64_t blocksize)
// {
//     assert((blocksize & 0xFFFFFFFF00000000) == 0);
//     set_blocksize(header_vaddr, (uint32_t)blocksize);
//     return 1;
// }

// static rbtree_node_interface i_node = 
// {
//     .construct_node = NULL,
//     .destruct_node = NULL,
//     .compare_nodes = &compare_nodes,
//     .is_null_node = &is_null_node,
//     .get_parent = &get_redblack_tree_parent,
//     .set_parent = &set_redblack_tree_parent,
//     .get_leftchild = &get_redblack_tree_left,
//     .set_leftchild = &set_redblack_tree_left,
//     .get_rightchild = &get_redblack_tree_right,
//     .set_rightchild = &set_redblack_tree_right,
//     .get_color = &get_redblack_tree_color,
//     .set_color = &set_redblack_tree_color,
//     .get_key = &get_redblack_tree_key,
//     .set_key = &set_redblack_tree_key,
// };

// /* ------------------------------------- */
// /*  Operations for Red-Black Tree        */
// /* ------------------------------------- */

// static int update_root(rbtree_internal_t *this, uint64_t block_vaddr)
// {
//     if (this == NULL)
//     {
//         return 0;
//     }
    
//     assert(block_vaddr == NULL_ID ||
//         (get_firstblock() <= block_vaddr && block_vaddr <= get_lastblock()));
//     assert(block_vaddr == NULL_ID || 
//         block_vaddr % 8 == 4);
//     assert(block_vaddr == NULL_ID || 
//         get_blocksize(block_vaddr) >= MIN_REDBLACK_TREE_BLOCKSIZE);

//     this->root = block_vaddr;
//     return 1;
// }

// // The red-black tree
// static rbtree_internal_t rbt;

// static void redblack_tree_init()
// {
//     rbt.root = NULL_ID;
//     rbt.update_root = &update_root;
// }

// static void redblack_tree_print()
// {
//     bst_internal_print(rbt.root, &i_node);
// }

// static void redblack_tree_insert(uint64_t node_ptr)
// {
//     set_redblack_tree_parent(node_ptr, NIL);
//     set_redblack_tree_left(node_ptr, NIL);
//     set_redblack_tree_right(node_ptr, NIL);
//     set_redblack_tree_color(node_ptr, COLOR_RED);

//     rbt_internal_insert(&rbt, &i_node, node_ptr);
// }

// static void redblack_tree_delete(uint64_t node_ptr)
// {
//     rbt_internal_delete(&rbt, &i_node, node_ptr);

//     set_redblack_tree_parent(node_ptr, NIL);
//     set_redblack_tree_left(node_ptr, NIL);
//     set_redblack_tree_right(node_ptr, NIL);
// }

// static uint64_t redblack_tree_search(uint32_t size)
// {
//     // search logic is the same: rbt & bst
//     return bst_internal_find_succ(&rbt, &i_node, (uint64_t)size);
// }

// /* ------------------------------------- */
// /*  Implementation                       */
// /* ------------------------------------- */

// int redblack_tree_initialize_free_block()
// {
//     uint64_t first_header = get_firstblock();
    
//     // init rbt for block >= 24
//     redblack_tree_init();
//     set_redblack_tree_parent(first_header, NIL);
//     set_redblack_tree_left(first_header, NIL);
//     set_redblack_tree_right(first_header, NIL);
//     redblack_tree_insert(first_header);

//     // init list for small block size == 16
//     explist_list_init();

//     // init small block list size == 8
//     small_list_init();

//     return 1;
// }

// uint64_t redblack_tree_search_free_block(uint32_t payload_size, uint32_t *alloc_blocksize)
// {
//     // search 8-byte block list
//     if (payload_size <= 4)
//     {
//         // a small block
//         *alloc_blocksize = 8;

//         if (small_list.count != 0)
//         {
//             // 8-byte list is not empty
//             return small_list.head;
//         }
//     }
//     else
//     {
//         *alloc_blocksize = round_up(payload_size, 8) + 4 + 4;
//     }
    
//     // search explicit free list
//     if ((*alloc_blocksize) == 16)
//     {
//         // This search is O(1) search since the list is fixed with size 16
//         // if the list is empty, return NIL
//         // else, return list head
//         uint64_t b16 = explicit_list_search(*alloc_blocksize);
//         if (b16 != NIL)
//         {
//             return b16;
//         }
//     }

//     // search RBT
//     return redblack_tree_search(*alloc_blocksize);
// }

// int redblack_tree_insert_free_block(uint64_t free_header)
// {
//     assert(free_header % 8 == 4);
//     assert(get_firstblock() <= free_header && free_header <= get_lastblock());
//     assert(get_allocated(free_header) == FREE);

//     uint32_t blocksize = get_blocksize(free_header);
//     assert(blocksize % 8 == 0);
//     assert(blocksize >= 8);

//     switch (blocksize)
//     {
//         case 8:
//             small_list_insert(free_header);
//             break;

//         case 16:
//             explicit_list_insert(free_header);
//             break;
        
//         default:
//             redblack_tree_insert(free_header);
//             break;
//     }

//     return 1;
// }

// int redblack_tree_delete_free_block(uint64_t free_header)
// {
//     assert(free_header % 8 == 4);
//     assert(get_firstblock() <= free_header && free_header <= get_lastblock());
//     assert(get_allocated(free_header) == FREE);

//     uint32_t blocksize = get_blocksize(free_header);
//     assert(blocksize % 8 == 0);
//     assert(blocksize >= 8);

//     switch (blocksize)
//     {
//         case 8:
//             small_list_delete(free_header);
//             break;

//         case 16:
//             explicit_list_delete(free_header);
//             break;
        
//         default:
//             redblack_tree_delete(free_header);
//             break;
//     }

//     return 1;
// }

// void redblack_tree_check_free_block()
// {
//     small_list_check_free_blocks();
//     check_block16_correctness();
// }