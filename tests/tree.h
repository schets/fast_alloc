#include "../common.h"
#include "../block_alloc.h"
#include <stdint.h>

struct node;

typedef struct tree {
    struct node *root;
    struct alloc_type *myalloc;
    size_t elems;
} tree;

//!Removes the element if there, otherwise adds it
void change_tree(tree *, uint32_t);

//!removes specified element from tree
void remove_tree(tree *, uint32_t);

//!adds element to tree
void add_tree(tree *, uint32_t);

char contains(tree *, uint32_t);

tree create_tree (struct alloc_type *inalloc);

void destroy_tree(tree *intree);

void print_tree(tree *intree);

void copy_tree(tree *from, tree *to);

char equals(tree *a, tree *b);

size_t depth(tree *intree);
