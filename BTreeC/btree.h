#ifndef BTREE_H
#define BTREE_H

#define ORDER 4

#include <stdbool.h>

typedef struct BTreeNode {
    int keys[ORDER - 1];
    struct BTreeNode *children[ORDER];
    int count;             // number of keys
    bool leaf;             // true if node is leaf
} BTreeNode;

BTreeNode* create_node(bool leaf);
void insert(BTreeNode **root, int key);
BTreeNode* search(BTreeNode *root, int key, int *comparisons);

#endif // BTREE_H
