#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "headers/algorithm.h"
#include "headers/common.h"

void bst_internal_insert(rbtree_internal_t *tree,
                         rbtree_node_interface *i_node,
                         uint64_t node_id)
{
  if (tree == NULL)
  {
    return;
  }
  assert(tree->update_root != NULL);
  assert(i_node->is_null_node != NULL);
  assert(i_node->set_parent != NULL);
  assert(i_node->set_leftchild != NULL);
  assert(i_node->set_rightchild != NULL);
  assert(i_node->set_color != NULL);
  assert(i_node->set_key != NULL);
  assert(i_node->get_parent != NULL);
  assert(i_node->get_leftchild != NULL);
  assert(i_node->get_rightchild != NULL);
  assert(i_node->get_color != NULL);
  assert(i_node->get_key != NULL);
  assert(i_node->is_null_node(node_id) == 0);

  if (i_node->is_null_node(tree->root) == 1)
  {
    i_node->set_parent(node_id, NULL_ID);
    i_node->set_leftchild(node_id, NULL_ID);
    i_node->set_rightchild(node_id, NULL_ID);
    i_node->set_color(node_id, COLOR_BLACK);

    tree->update_root(tree, node_id);
    return;
  }

  uint64_t p = tree->root;
  uint64_t n_key = i_node->get_key(node_id);

  while (i_node->is_null_node(p) == 0)
  {
    uint64_t p_key = i_node->get_key(p);

    if (n_key < p_key)
    {
      uint64_t p_left = i_node->get_leftchild(p);

      if (i_node->is_null_node(p_left) == 1)
      {
        // insert node to p->left
        i_node->set_leftchild(p, node_id);
        i_node->set_parent(node_id, p);
        return;
      }
      else
      {
        p = p_left;
      }
    }
    else // if (n_key >= p_key)
    {
      uint64_t p_right = i_node->get_rightchild(p);

      if (i_node->is_null_node(p_right) == 1)
      {
        // insert node to p->left
        i_node->set_rightchild(p, node_id);
        i_node->set_parent(node_id, p);
        return;
      }
      else
      {
        p = p_right;
      }
    }
  }
}

void bst_internal_delete(rbtree_internal_t *tree,
                         rbtree_node_interface *i_node,
                         uint64_t node_id)
{
  if (tree == NULL)
  {
    return;
  }
  assert(tree->update_root != NULL);
  assert(i_node->is_null_node != NULL);
  assert(i_node->set_parent != NULL);
  assert(i_node->set_leftchild != NULL);
  assert(i_node->set_rightchild != NULL);
  assert(i_node->set_color != NULL);
  assert(i_node->set_key != NULL);
  assert(i_node->get_parent != NULL);
  assert(i_node->get_leftchild != NULL);
  assert(i_node->get_rightchild != NULL);
  assert(i_node->get_color != NULL);
  assert(i_node->get_key != NULL);

  if (i_node->is_null_node(tree->root) == 1)
  {
    // nothing to delete
    return;
  }

  if (i_node->is_null_node(node_id) == 1)
  {
    // delete a null
    return;
  }

  uint64_t n_left = i_node->get_leftchild(node_id);
  uint64_t n_right = i_node->get_rightchild(node_id);

  int is_n_left_null = i_node->is_null_node(n_left);
  int is_n_right_null = i_node->is_null_node(n_right);

  if (is_n_left_null == 1 && is_n_right_null == 1)
  {
    //////////////////////////////////////////////
    // case 1: leaf node                        //
    //////////////////////////////////////////////
    uint64_t parent = i_node->get_parent(node_id);
    uint64_t parent_left = i_node->get_leftchild(parent);

    if (i_node->is_null_node(parent) == 1)
    {
      // node is the root of the tree
      assert(i_node->compare_nodes(tree->root, node_id) == 0);

      tree->update_root(tree, NULL_ID);
      i_node->destruct_node(node_id);
      return;
    }
    else
    {
      // a normal leaf node
      if (i_node->compare_nodes(node_id, parent_left) == 0)
      {
        // node is the left child of parent
        i_node->set_leftchild(parent, NULL_ID);
      }
      else
      {
        // node is the right child of parent
        i_node->set_rightchild(parent, NULL_ID);
      }

      i_node->destruct_node(node_id);
      return;
    }
  }
  else if (is_n_left_null == 1 || is_n_right_null == 1)
  {
    //////////////////////////////////////////////
    // case 2: one sub-tree is empty            //
    //////////////////////////////////////////////

    // the only non-null sub-tree
    uint64_t x = NULL_ID;

    if (is_n_left_null == 0)
    {
      x = n_left;
    }
    else if (is_n_right_null == 0)
    {
      x = n_right;
    }
    else
    {
      assert(0);
    }

    uint64_t x_key = i_node->get_key(x);
    uint64_t x_value = i_node->get_value(x);
    uint64_t x_left = i_node->get_leftchild(x);   // beta
    uint64_t x_right = i_node->get_rightchild(x); // gamma

    i_node->set_key(node_id, x_key);
    i_node->set_value(node_id, x_value);

    i_node->set_leftchild(node_id, x_left);
    i_node->set_rightchild(node_id, x_right);

    if (i_node->is_null_node(x_left) == 0)
    {
      i_node->set_parent(x_left, node_id);
    }

    if (i_node->is_null_node(x_right) == 0)
    {
      i_node->set_parent(x_right, node_id);
    }

    // actually free n_right
    i_node->destruct_node(x);
    return;
  }
  else
  {
    //////////////////////////////////////////////
    // case 3: neither sub-tree is empty        //
    //////////////////////////////////////////////

    // check the n->right->left
    uint64_t n_right_left = i_node->get_leftchild(n_right);
    int is_n_right_left_null = i_node->is_null_node(n_right_left);

    if (is_n_right_left_null == 1)
    {
      // 3.1: a simple remove will do the job
      uint64_t n_right_key = i_node->get_key(n_right);
      uint64_t n_right_value = i_node->get_value(n_right);
      uint64_t n_right_right = i_node->get_rightchild(n_right);

      i_node->set_key(node_id, n_right_key);
      i_node->set_value(node_id, n_right_value);
      i_node->set_rightchild(node_id, n_right_right);
      i_node->set_parent(n_right_right, node_id);

      // actually free n_right
      i_node->destruct_node(n_right);
      return;
    }
    else
    {
      // 3.2: float up the upper bound
      // as root of the sub-tree of node_id
      uint64_t q = n_right;
      uint64_t q_left = i_node->get_leftchild(q);
      while (i_node->is_null_node(q_left) == 0)
      {
        q = q_left;
        q_left = i_node->get_leftchild(q);
      }

      // q is the upper bound of node_id
      uint64_t q_key = i_node->get_key(q);
      uint64_t q_value = i_node->get_value(q);
      uint64_t q_parent = i_node->get_parent(q);
      q_left = i_node->get_leftchild(q);
      uint64_t q_right = i_node->get_rightchild(q);

      assert(i_node->is_null_node(q_parent) == 0);
      assert(i_node->is_null_node(q_left) == 1);
      assert(i_node->compare_nodes(q, i_node->get_leftchild(q_parent)) == 0);

      // update the key in-place
      i_node->set_key(node_id, q_key);
      i_node->set_value(node_id, q_value);

      // remove the old q node from tree
      i_node->set_leftchild(q_parent, q_right);

      if (i_node->is_null_node(q_right) == 0)
      {
        i_node->set_parent(q_right, q_parent);
      }

      i_node->destruct_node(q);
      return;
    }
  }
}

uint64_t bst_internal_find(rbtree_internal_t *tree,
                           rbtree_node_interface *i_node,
                           uint64_t key)
{
  if (tree == NULL)
  {
    return NULL_ID;
  }
  assert(i_node->is_null_node != NULL);
  assert(i_node->get_leftchild != NULL);
  assert(i_node->get_rightchild != NULL);
  assert(i_node->get_key != NULL);

  if (i_node->is_null_node(tree->root) == 1)
  {
    return NULL_ID;
  }

  uint64_t p = tree->root;

  while (i_node->is_null_node(p) == 0)
  {
    uint64_t p_key = i_node->get_key(p);

    if (key == p_key)
    {
      // return the first found key
      // which is the most left node of equals
      return p;
    }
    else if (key < p_key)
    {
      p = i_node->get_leftchild(p);
    }
    else // if (n_key > p_key)
    {
      p = i_node->get_rightchild(p);
    }
  }

  return NULL_ID;
}

// The returned node should have the key >= target key
uint64_t bst_internal_find_succ(rbtree_internal_t *tree,
                                rbtree_node_interface *i_node,
                                uint64_t key)
{
  if (tree == NULL)
  {
    return NULL_ID;
  }
  assert(i_node->is_null_node != NULL);
  assert(i_node->get_leftchild != NULL);
  assert(i_node->get_rightchild != NULL);
  assert(i_node->get_key != NULL);

  if (i_node->is_null_node(tree->root) == 1)
  {
    return NULL_ID;
  }

  uint64_t p = tree->root;

  uint64_t successor = NULL_ID;
  // positive infinite. should be large enough
  uint64_t successor_key = 0x7FFFFFFFFFFFFFFF;

  while (i_node->is_null_node(p) == 0)
  {
    uint64_t p_key = i_node->get_key(p);

    if (key == p_key)
    {
      // return the first found key
      // which is the most left node of equals
      return p;
    }
    else if (key < p_key)
    {
      if (p_key <= successor_key)
      {
        // key < p_key <= successor_key
        // p is more close to target key than
        // the recorded successor
        successor = p;
        successor_key = p_key;
      }

      p = i_node->get_leftchild(p);
    }
    else // if (n_key > p_key)
    {
      p = i_node->get_rightchild(p);
    }
  }

  // if no node key >= target key, return NULL_ID
  return successor;
}

static void tree_print_dfs(uint64_t node, rbtree_node_interface *i_node)
{
  assert(i_node != NULL);
  assert(i_node->is_null_node != NULL);
  assert(i_node->get_leftchild != NULL);
  assert(i_node->get_rightchild != NULL);
  assert(i_node->get_key != NULL);
  assert(i_node->get_color != NULL);

  if (i_node->is_null_node(node) == 1)
  {
    printf("#");
    return;
  }

  if (i_node->get_color(node) == COLOR_RED)
  {
    printf("(\033[31m%lu\033[0m,", i_node->get_key(node));
  }
  else
  {
    printf("(%lu,", i_node->get_key(node));
  }

  tree_print_dfs(i_node->get_leftchild(node), i_node);
  printf(",");
  tree_print_dfs(i_node->get_rightchild(node), i_node);
  printf(")");
}

void tree_internal_print(rbtree_internal_t *tree, rbtree_node_interface *i_node)
{
  if (tree != NULL)
  {
    assert(i_node != NULL);
    tree_print_dfs(tree->root, i_node);
  }
  printf("\n");
}

// For test use
// the format of string str:
// 1. NULL node - `#`
// 2. (root node key, left tree key, right tree key)
void internal_tree_construct_keystr(rbtree_internal_t *tree, rbtree_node_interface *i_node, char *str)
{
  assert(i_node != NULL);
  assert(tree != NULL);
  assert(tree->update_root != NULL);
  assert(i_node->is_null_node != NULL);
  assert(i_node->compare_nodes != NULL);
  assert(i_node->construct_node != NULL);
  assert(i_node->get_parent != NULL);
  assert(i_node->get_leftchild != NULL);
  assert(i_node->get_rightchild != NULL);
  assert(i_node->get_key != NULL);
  assert(i_node->get_color != NULL);
  assert(i_node->set_parent != NULL);
  assert(i_node->set_leftchild != NULL);
  assert(i_node->set_rightchild != NULL);
  assert(i_node->set_key != NULL);
  assert(i_node->set_color != NULL);

  // a node on STACK, a LOCAL variable!
  // this is the sentinel to mark the unprocessed sub-tree
  // this node_id is different from NULL_ID, and should be
  // different from all possibilities from node constructor
  uint64_t todo = 1;

  // pointer stack (at most 1K nodes) for node ids
  uint64_t stack[1000];
  int top = -1;

  int i = 0;
  while (i < strlen(str))
  {
    if (str[i] == '(')
    {
      // push the node as being processed
      top++;

      uint64_t x = i_node->construct_node();
      i_node->set_parent(x, NULL_ID);
      i_node->set_leftchild(x, todo);
      i_node->set_rightchild(x, todo);

      // scan the value
      // (value,
      int j = i + 1;
      while ('0' <= str[j] && str[j] <= '9')
      {
        ++j;
      }
      i_node->set_key(x, string2uint_range(str, i + 1, j - 1));

      // push to stack
      stack[top] = x;

      // move to next node
      i = j + 1;
      continue;
    }
    else if (str[i] == ')')
    {
      // pop the being processed node
      if (top == 0)
      {
        // pop root
        uint64_t first = stack[0];
        uint64_t first_left = i_node->get_leftchild(first);
        uint64_t first_right = i_node->get_rightchild(first);

        assert(i_node->compare_nodes(first_left, todo) != 0 &&
               i_node->compare_nodes(first_right, todo) != 0);

        tree->update_root(tree, first);
        return;
      }

      // pop a non-root node
      uint64_t p = stack[top - 1]; // the parent of top
      uint64_t t = stack[top];     // the poped top - can be left or right child of parent

      // when pop top, its left & right subtree are all reduced
      assert(i_node->compare_nodes(t, todo) != 0 &&
             i_node->compare_nodes(t, todo) != 0);

      top--;

      // pop this node
      uint64_t p_left = i_node->get_leftchild(p);
      uint64_t p_right = i_node->get_rightchild(p);

      if (i_node->compare_nodes(p_left, todo) == 0)
      {
        // check left child of parent FIRST
        // it's not yet processed
        // thus top is parent's left child
        i_node->set_leftchild(p, t);
        i_node->set_parent(t, p);
        i++;
        continue;
      }
      else if (i_node->compare_nodes(p_right, todo) == 0)
      {
        // check right child of parent THEN
        // left sub-tree has been reduced
        // but right sub-tree has not yet
        i_node->set_rightchild(p, t);
        i_node->set_parent(t, p);
        i++;
        continue;
      }

      printf("node %lx:%lx is not having any unprocessed sub-tree\n  while %lx:%lx is redblack into it.\n",
             p, i_node->get_key(p), t, i_node->get_key(t));
      assert(0);
    }
    else if (str[i] == '#')
    {
      if (top < 0)
      {
        assert(strlen(str) == 1);
        return;
      }

      uint64_t top_id = stack[top];

      // push NULL node
      // pop NULL node
      if (i_node->compare_nodes(
              i_node->get_leftchild(top_id),
              todo) == 0)
      {
        // must check parent's left node first
        i_node->set_leftchild(top_id, NULL_ID);
        i++;
        continue;
      }
      else if (i_node->compare_nodes(
                   i_node->get_rightchild(top_id),
                   todo) == 0)
      {
        // then check parent's right node
        i_node->set_rightchild(top_id, NULL_ID);
        i++;
        continue;
      }

      printf("node %lx:(%lx) is not having any unprocessed sub-tree\n  while NULL is redblack into it.\n",
             top_id, i_node->get_key(top_id));
      assert(0);
    }
    else
    {
      // space, comma, new line
      i++;
      continue;
    }
  }
}

// a and b are node ids
int internal_tree_compare(uint64_t a, uint64_t b, rbtree_node_interface *i_node, int is_rbt)
{
  assert(i_node != NULL);
  assert(i_node->is_null_node != NULL);

  int is_a_null = i_node->is_null_node(a);
  int is_b_null = i_node->is_null_node(b);

  if (is_a_null == 1 && is_b_null == 1)
  {
    return 1;
  }

  if (is_a_null == 1 || is_b_null == 1)
  {
    return 0;
  }

  // both not NULL
  if (i_node->get_key(a) == i_node->get_key(b))
  {
    if (is_rbt == 0)
    {
      return internal_tree_compare(i_node->get_leftchild(a), i_node->get_leftchild(b), i_node, is_rbt) &&
             internal_tree_compare(i_node->get_rightchild(a), i_node->get_rightchild(b), i_node, is_rbt);
    }
    else if (is_rbt == 1)
    {
      // red-black tree needs to compare the node color
      if (i_node->get_color(a) == i_node->get_color(b))
      {
        return internal_tree_compare(i_node->get_leftchild(a), i_node->get_leftchild(b), i_node, is_rbt) &&
               internal_tree_compare(i_node->get_rightchild(a), i_node->get_rightchild(b), i_node, is_rbt);
      }
    }
    assert(0);
  }

  return 0;
}

/*======================================*/
/*      Default Implementation          */
/*======================================*/

// Implementation of the binary search tree node access

static int is_null_node(uint64_t node_id)
{
  if (node_id == NULL_ID)
  {
    return 1;
  }
  return 0;
}

static uint64_t construct_node()
{
  rb_node_t *node = malloc(sizeof(rb_node_t));
  if (node != NULL)
  {
    node->parent = NULL;
    node->left = NULL;
    node->right = NULL;
    node->color = COLOR_BLACK;
    node->key = 0;
    node->value = 0;

    return (uint64_t)node;
  }
  return NULL_ID;
}

static int destruct_node(uint64_t node_id)
{
  if (is_null_node(node_id) == 1)
  {
    return 0;
  }

  rb_node_t *ptr = (rb_node_t *)node_id;
  if (ptr != NULL)
  {
    free(ptr);
  }

  return 1;
}

static int compare_nodes(uint64_t first, uint64_t second)
{
  return !(first == second);
}

static uint64_t get_parent(uint64_t node_id)
{
  if (is_null_node(node_id) == 1)
  {
    return NULL_ID;
  }
  return (uint64_t)(((rb_node_t *)node_id)->parent);
}

static int set_parent(uint64_t node_id, uint64_t parent_id)
{
  if (is_null_node(node_id) == 1)
  {
    return 0;
  }
  *(uint64_t *)&(((rb_node_t *)node_id)->parent) = parent_id;
  return 1;
}

static uint64_t get_leftchild(uint64_t node_id)
{
  if (is_null_node(node_id) == 1)
  {
    return NULL_ID;
  }
  return (uint64_t)(((rb_node_t *)node_id)->left);
}

static int set_leftchild(uint64_t node_id, uint64_t left_id)
{
  if (is_null_node(node_id) == 1)
  {
    return 0;
  }
  *(uint64_t *)&(((rb_node_t *)node_id)->left) = left_id;
  return 1;
}

static uint64_t get_rightchild(uint64_t node_id)
{
  if (is_null_node(node_id) == 1)
  {
    return NULL_ID;
  }
  return (uint64_t)(((rb_node_t *)node_id)->right);
}

static int set_rightchild(uint64_t node_id, uint64_t right_id)
{
  if (is_null_node(node_id) == 1)
  {
    return 0;
  }
  *(uint64_t *)&(((rb_node_t *)node_id)->right) = right_id;
  return 1;
}

static rb_color_t get_color(uint64_t node_id)
{
  if (is_null_node(node_id) == 1)
  {
    return NULL_ID;
  }
  return ((rb_node_t *)node_id)->color;
}

static int set_color(uint64_t node_id, rb_color_t color)
{
  if (is_null_node(node_id) == 1)
  {
    return 0;
  }
  ((rb_node_t *)node_id)->color = color;
  return 1;
}

static uint64_t get_key(uint64_t node_id)
{
  if (is_null_node(node_id) == 1)
  {
    return NULL_ID;
  }
  return ((rb_node_t *)node_id)->key;
}

static int set_key(uint64_t node_id, uint64_t key)
{
  if (is_null_node(node_id) == 1)
  {
    return 0;
  }
  ((rb_node_t *)node_id)->key = key;
  return 1;
}

static uint64_t get_value(uint64_t node_id)
{
  if (is_null_node(node_id) == 1)
  {
    return NULL_ID;
  }
  return ((rb_node_t *)node_id)->value;
}

static int set_value(uint64_t node_id, uint64_t value)
{
  if (is_null_node(node_id) == 1)
  {
    return 0;
  }
  ((rb_node_t *)node_id)->value = value;
  return 1;
}

// shared with red-black tree
rbtree_node_interface default_i_rbt_node =
    {
        .construct_node = &construct_node,
        .destruct_node = &destruct_node,
        .is_null_node = &is_null_node,
        .compare_nodes = &compare_nodes,
        .get_parent = &get_parent,
        .set_parent = &set_parent,
        .get_leftchild = &get_leftchild,
        .set_leftchild = &set_leftchild,
        .get_rightchild = &get_rightchild,
        .set_rightchild = &set_rightchild,
        .get_color = &get_color,
        .set_color = &set_color,
        .get_key = &get_key,
        .set_key = &set_key,
        .get_value = &get_value,
        .set_value = &set_value,
};

// child class of base class
static int update_root(rbtree_internal_t *this, uint64_t new_root)
{
  if (this == NULL)
  {
    return 0;
  }
  this->root = new_root;
  return 1;
}

//
//  The exposed interfaces
//

// constructor and destructor
rb_tree_t *bst_construct()
{
  rb_tree_t *tree = malloc(sizeof(rb_tree_t));
  tree->root = NULL_ID;
  tree->update_root = &update_root;
  return tree;
}

// for test use
rb_tree_t *bst_construct_keystr(char *str)
{
  rb_tree_t *tree = bst_construct();
  internal_tree_construct_keystr(&(tree->base), &default_i_rbt_node, str);
  return tree;
}

void bst_print(rb_tree_t *tree)
{
  tree_internal_print(&(tree->base), &default_i_rbt_node);
}

static void bst_destruct_subtree(uint64_t root)
{
  if (default_i_rbt_node.is_null_node(root) == 1)
  {
    return;
  }

  // (sub)root is not null
  // free its left and right childs
  bst_destruct_subtree(default_i_rbt_node.get_leftchild(root));
  bst_destruct_subtree(default_i_rbt_node.get_rightchild(root));

  // free the (sub)root node
  default_i_rbt_node.destruct_node(root);

  return;
}

void bst_free(rb_tree_t *tree)
{
  if (tree == NULL)
  {
    return;
  }

  // remove the tree from root
  bst_destruct_subtree(tree->root);

  // finally, free the struct for tree
  free(tree);
}

void bst_add(rb_tree_t *tree, uint64_t key)
{
  rb_node_t *n = (rb_node_t *)default_i_rbt_node.construct_node();
  n->key = key;

  bst_internal_insert(&(tree->base), &default_i_rbt_node, (uint64_t)n);
}

void bst_insert(rb_tree_t *tree, rb_node_t *node)
{
  bst_internal_insert(&(tree->base), &default_i_rbt_node, (uint64_t)node);
}

void bst_remove(rb_tree_t *tree, uint64_t key)
{
  rb_node_t *node = bst_find(tree, key);
  if (node == NULL)
  {
    return;
  }
  bst_delete(tree, node);
}

void bst_delete(rb_tree_t *tree, rb_node_t *node)
{
  bst_internal_delete(&(tree->base), &default_i_rbt_node, (uint64_t)node);
}

rb_node_t *bst_find(rb_tree_t *tree, uint64_t key)
{
  uint64_t node_id = bst_internal_find(&(tree->base), &default_i_rbt_node, key);

  if (default_i_rbt_node.is_null_node(node_id) == 1)
  {
    return NULL;
  }

  return (rb_node_t *)node_id;
}

rb_node_t *bst_find_succ(rb_tree_t *tree, uint64_t key)
{
  uint64_t node_id = bst_internal_find_succ(&(tree->base), &default_i_rbt_node, key);

  if (default_i_rbt_node.is_null_node(node_id) == 1)
  {
    return NULL;
  }

  return (rb_node_t *)node_id;
}

int bst_compare(rb_tree_t *a, rb_tree_t *b)
{
  if (a == NULL && b == NULL)
  {
    return 1;
  }
  if (a == NULL || b == NULL)
  {
    return 0;
  }

  return internal_tree_compare(a->root, b->root, &default_i_rbt_node, 0);
}