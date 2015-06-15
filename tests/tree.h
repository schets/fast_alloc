#include "../common.h"
#include "../block_alloc.h"
#include <stdint.h>

struct node;
#define UNFIXED_BLOCK

typedef struct tree {
    struct node *root;
    #ifdef UNFIXED_BLOCK
    struct unfixed_block blk;
    #endif
} tree;

//!Removes the element if there, otherwise adds it
void change_tree(tree *, uint32_t);

//!removes specified element from tree
void remove_tree(tree *, uint32_t);

//!adds element to tree
void add_tree(tree *, uint32_t);

tree create_tree (size_t extra, size_t blk_size);

void destroy_tree(tree *intree);

void print_tree(tree *intree);

