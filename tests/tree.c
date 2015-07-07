#include "tree.h"
#include <stdio.h>
#include <stdlib.h>

#define LEFTV 0
#define RIGHTV 1
#define OTHERV(x) (1^(x))

#define LEFT(x) (x->child[LEFTV])
#define RIGHT(x) (x->child[RIGHTV])
#define OTHER(x, lrv) (x->child[OTHERV(lrv)])

typedef struct node {
    struct node *child[2];
    uint32_t data;
} node;

static inline node *create_node(tree *intree, uint32_t data) {
    node *rval;
    rval = (node *)intree->myalloc->malloc(intree->myalloc, sizeof(node));
    rval->data = data;
    rval->child[LEFTV] = rval->child[RIGHTV] = 0;
    intree->elems++;
    return rval;
}

static inline void free_node(tree* intree, node *innode) {
    intree->elems--;
    intree->myalloc->free(intree->myalloc, innode);
}

static node *remove_minmax_from(node *start, size_t lr) {
    if (!start)
        return NULL;
    node *par = NULL;
    node *minn = start;
    while (minn->child[lr]) {
        par = minn;
        minn = minn->child[lr];
    }
    if (par) {
        par->child[lr] = minn->child[OTHERV(lr)];
        minn->child[OTHERV(lr)] = NULL;
    }
    return minn;
}

static node *remove_node_from(node *parent, node *start, size_t from) {
    if (!start)
        return NULL;

    node *newnode = NULL;
    if (LEFT(start) && RIGHT(start)) {
        // 'randomly' select min node from right or max from left as successor
        size_t lrv = start->data & 1;
        newnode = remove_minmax_from(start->child[OTHERV(lrv)], lrv);
        newnode->child[lrv] = start->child[lrv];
        if (newnode != start->child[OTHERV(lrv)])
            newnode->child[OTHERV(lrv)] = start->child[OTHERV(lrv)];
    }

    //only one node is there, shift up the child
    else if (LEFT(start)) {
        newnode = LEFT(start);
    }
    else if (RIGHT(start)) {
        newnode = RIGHT(start);
    }
    //add new node to parent
    if (parent) {
        parent->child[from] = newnode;
    }
    return newnode;
}

static void _add_tree(tree *from,
                      node *parent,
                      node *cnode,
                      uint32_t data,
                      size_t lr) {
    if (!cnode)
        parent->child[lr] = create_node(from, data);
    else if (data < cnode->data)
        _add_tree(from, cnode, LEFT(cnode), data, LEFTV);
    else if (data > cnode->data)
        _add_tree(from, cnode, RIGHT(cnode), data, RIGHTV);
    //end otherwise...
}

void add_tree(tree *intree, uint32_t data) {
    if (!intree->root)
        intree->root = create_node(intree, data);
    else
        _add_tree(intree, NULL, intree->root, data, 0);
}

static void _change_tree(tree *from,
                         node *parent,
                         node *cnode,
                         uint32_t data,
                         size_t lr) {
    if (!cnode)
        parent->child[lr] = create_node(from, data);
    else if (data < cnode->data)
        _change_tree(from, cnode, LEFT(cnode), data, LEFTV);
    else if (data > cnode->data)
        _change_tree(from, cnode, RIGHT(cnode), data, RIGHTV);
    else {
        remove_node_from(parent, cnode, lr);
        free_node(from, cnode);
    }
}

//copy/past is easier than doing the proper thing
void change_tree(tree *intree, uint32_t data) {
    node *cnode = intree->root;
    if (!cnode) 
        intree->root = create_node(intree, data);
    else if(cnode->data == data) {
        node *rmnode = intree->root;
        intree->root = remove_node_from(NULL, intree->root, 1);
        free_node(intree, rmnode);
    }
    else
        _change_tree(intree, NULL, cnode, data, 0);
}

static void _remove_tree(tree *from,
                         node *parent,
                         node *cnode,
                         uint32_t data,
                         size_t lr) {
    if (!cnode)
        return;
    else if (data < cnode->data)
        _remove_tree(from, cnode, LEFT(cnode), data, LEFTV);
    else if (data > cnode->data)
        _remove_tree(from, cnode, RIGHT(cnode), data, RIGHTV);
    else {
        remove_node_from(parent, cnode, lr);
        free_node(from, cnode);
    }
}

void remove_tree(tree *intree, uint32_t data) {
    if (!intree->root)
        return;
    else if(intree->root->data == data) {
        node *rmnode = intree->root;
        intree->root = remove_node_from(NULL, intree->root, 1);
        free_node(intree, rmnode);
    }
    else
        _remove_tree(intree, NULL, intree->root, data, 0);
}

char _contains(node *cur, uint32_t data) {
    if (cur) {
        if (cur->data == data)
            return 1;
        return _contains(cur->child[cur->data < data], data);
    }
    return 0;
}

char contains(tree *intree, uint32_t data) {
    return _contains(intree->root, data);
}

tree create_tree (struct alloc_type *myalloc) {
    tree rval;
    rval.root = NULL;
    rval.myalloc = myalloc;
    rval.elems = 0;
    return rval;
}

static void destroy_nodes(tree *intree, node *cur) {
    if (cur) {
        destroy_nodes(intree, LEFT(cur));
        destroy_nodes(intree, RIGHT(cur));
        free_node(intree, cur);
    }
}

void destroy_tree(tree *intree) {
    destroy_nodes(intree, intree->root);
    intree->root = NULL;
    intree->elems = 0;
}

void print_space(size_t depth) {
    for(size_t i = 0; i < depth; i++) {
        printf("  ");
    } 
}

static void print_nodes(node *cur, size_t depth) {
    if (cur) {
        print_nodes(LEFT(cur), depth + 1);
        print_space(depth);
        printf("%d\n", cur->data);
        print_nodes(RIGHT(cur), depth + 1);
    }
}

void print_tree(tree *intree) {
    print_nodes(intree->root, 0);
    printf("\n");
}

static size_t _depth(node *innode) {
    if(innode == NULL)
        return 0;
    size_t ldepth = _depth(LEFT(innode));
    size_t rdepth = _depth(RIGHT(innode));
    return 1 + (ldepth > rdepth ? ldepth : rdepth);
}

size_t depth(tree *intree) {
    return _depth(intree->root);
}

static void _copy_tree(tree *newt, node *old, node *newn) {
    if (old) {
        //if even go left first, otherwise right first
        //random so that locality doesn't favor a direction
        size_t copdir = old->data & 1; 
        if (LEFT(old))
            newn->child[LEFTV] = create_node(newt, LEFT(old)->data);
        if (RIGHT(old))
            newn->child[RIGHTV] = create_node(newt, RIGHT(old)->data);
        if (old->child[copdir])
            _copy_tree(newt, old->child[copdir], newn->child[copdir]);
        copdir = OTHERV(copdir);
        if (old->child[copdir])
            _copy_tree(newt, old->child[copdir], newn->child[copdir]);
    }
}

void copy_tree(tree *old, tree *to) {
    destroy_tree(to);
    if (old->root) {
        to->root = create_node(to, old->root->data);
        _copy_tree(to, old->root, to->root);
    }
}

char _equals(node *a, node *b) {
    if (a == b)
        return 1;
    if ((!a) || (!b))
        return 0;
    return (a->data == b->data) && (_equals(LEFT(a), LEFT(b))) && (_equals(RIGHT(a), RIGHT(b)));
}

char equals(tree *a, tree *b) {
    return _equals(a->root, b->root);
}





