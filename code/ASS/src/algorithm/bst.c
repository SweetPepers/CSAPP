#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include "headers/common.h"
#include "headers/algorithm.h"

void bstree_internal_insert(rbtree_internal_t *tree, rbtree_node_interface *i_node, uint64_t node_id)
{
  if (tree == NULL)
  {
    return;
  }
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
        //insert node to p_left
        i_node->set_leftchild(p, node_id);
        return;
      }
      p = p_left;
    }
    else //if(n_key >= p_key)
    {
      uint64_t p_right = i_node->get_rightchild(p);
      if (i_node->is_null_node(p_right) == 1)
      {
        //insert node to p_right
        i_node->set_rightchild(p, node_id);
        return;
      }
      p = p_right;
    }
  }
}

void bstree_internal_delete(rbtree_internal_t *tree, rbtree_node_interface *i_node, uint64_t node_id)
{
  if (tree == NULL)
  {
    return;
  }
  assert(i_node->is_null_node != NULL);
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
    return;
  }
  if (i_node->is_null_node(node_id) == 1)
  {
    return;
  }

  

  uint64_t n_left = i_node->get_leftchild(node_id);
  uint64_t n_right = i_node->get_rightchild(node_id);

  int is_n_left_null = i_node->is_null_node(n_left);
  int is_n_right_nill = i_node->is_null_node(n_right);

  if (is_n_left_null == 1 && is_n_right_nill == 1)
  {

    //case 1 : leaf node
    uint64_t parent = i_node->get_parent(node_id);
    uint64_t parent_left = i_node->get_leftchild(parent)

    if(i_node->is_null_node(parent) == 1){
      //node is the root of the tree
      assert(i_node->compare_nodes(tree->root, node_id) == 0);
      tree->update_root(tree, NULL_ID);
      i_node->destruct_node(node_id);
    }else{
      // a normal leaf node
      
      if(i_node->construct_node(node_id,parent_left ) == 0){
        i_node->set_leftchild(parent, NULL_ID);
      }else{
        i_node->set_rightchild(parent, NULL_ID);
      }
      i_node->destruct_node(node_id);
    }
    
  }
  else if (is_n_left_null == 1 || is_n_right_nill == 1)
  {
    // case 2  : one sub-tree is null
    uint64_t x = NULL_ID;

    if (is_n_left_null == 0)
    {
      x = n_left;
    }
    else
    {
      x = n_right;
    }

    uint64_t x_key = i_node->get_key(x);
    uint64_t x_value = i_node->get_value(x);
    uint64_t x_left = i_node->get_leftchild(x);
    uint64_t x_right = i_node->get_rightchild(x);

    i_node->set_key(node_id, x_key);
    i_node->set_value(node_id, x_value);

    i_node->set_leftchild(node_id, x_left);
    i_node->set_rightchild(node_id, x_right);

    //set parent
    if(i_node->is_null_node(x_left) == 0){
      i_node->set_parent(x_left, node_id);
    }
    if(i_node->is_null_node(x_right) == 0){
      i_node->set_parent(x_right, node_id);
    }
    
    i_node->destruct_node(x);

  }
  else
  {
    // case 3: neither sub-tree is empty

    uint64_t n_right_left = i_node->get_leftchild(n_right);
    int is_n_right_left_null = i_node->is_null_node(n_right_left);

    if (is_n_right_left_null == 1)
    {
      //3.1 right_left is null 
      uint64_t n_right_right = i_node->get_rightchild(n_right);
      // update 
      i_node->set_key(node_id, i_node->get_key(n_right));
      i_node->set_value(node_id, i_node->get_value(n_right));
      i_node->set_rightchild(node_id, n_right_right);
      i_node->set_parent(n_right_right, node_id);
      
      i_node->destruct_node(n_right); 
    }
    else
    {
      // 3.2
      // float up the upper bound 
      //as root of the sub-tree of node_id
      uint64_t q = n_right;
      uint64_t q_left = i_node->get_leftchild(q);
      while(i_node->is_null_node(q) == 0){
        q = q_left;
        q_left = i_node->get_leftchild(q);
      }

      //q_left is null, q is the upper bound of node_id
      i_node->set_key(node_id, i_node->get_key(q));
      uint64_t q_parent = i_node->get_parent(q);
      uint64_t q_right = i_node->get_rightchild(q);
      uint64_t q_key = i_node->get_key(q);
      uint64_t q_value = i_node->get_value(q);
      assert(i_node->is_null_node(q_parent) == 0);
      assert(i_node->is_null_node(q_left) == 1);

      //update the key in-place
      i_node->set_key(node_id, q_key);
      i_node->set_value(node_id, q_value);

      //remove the old q node from tree
      i_node->set_leftchild(q_parent, q_right);
      if(i_node->is_null_node(q_right) == 0){
        i_node->set_parent(q_right, q_parent);
      }
      i_node->destruct_node(q);

    }
  }
}

void bstree_internal_find(rbtree_internal_t *tree, rbtree_node_interface *i_node, uint64_t key)
{
  if (tree == NULL)
  {
    return;
  }
  assert(i_node->is_null_node != NULL);

  assert(i_node->get_leftchild != NULL);
  assert(i_node->get_rightchild != NULL);
  assert(i_node->get_color != NULL);
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
      return p;
    }
    if (key < p_key)
    {
      p = i_node->get_leftchild(p);
    }
    else //if(n_key >= p_key)
    {
      p = i_node->get_rightchild(p);
    }
  }
  return NULL_ID;
}

#ifdef DEBUF_BST

int main()
{
  return 0;
}

#endif
