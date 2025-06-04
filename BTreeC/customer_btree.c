#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define ORDER 4

typedef struct BTreeNode {
    int keys[ORDER - 1];
    struct BTreeNode *children[ORDER];
    int count;         // number of keys
    bool leaf;         // true if node is leaf
} BTreeNode;

static BTreeNode* create_node(bool leaf) {
    BTreeNode* node = (BTreeNode*)malloc(sizeof(BTreeNode));
    node->leaf = leaf;
    node->count = 0;
    for (int i = 0; i < ORDER; ++i) {
        node->children[i] = NULL;
    }
    return node;
}

static void split_child(BTreeNode *parent, int index, BTreeNode *child) {
    BTreeNode *new_child = create_node(child->leaf);
    new_child->count = ORDER/2 - 1; // for ORDER=4 ->1

    for (int j = 0; j < ORDER/2 - 1; ++j) {
        new_child->keys[j] = child->keys[j + ORDER/2];
    }
    if (!child->leaf) {
        for (int j = 0; j < ORDER/2; ++j) {
            new_child->children[j] = child->children[j + ORDER/2];
        }
    }
    child->count = ORDER/2 - 1;

    for (int j = parent->count; j >= index + 1; --j) {
        parent->children[j + 1] = parent->children[j];
    }
    parent->children[index + 1] = new_child;

    for (int j = parent->count - 1; j >= index; --j) {
        parent->keys[j + 1] = parent->keys[j];
    }
    parent->keys[index] = child->keys[ORDER/2 - 1];
    parent->count += 1;
}

static void insert_nonfull(BTreeNode *node, int key) {
    int i = node->count - 1;
    if (node->leaf) {
        while (i >= 0 && key < node->keys[i]) {
            node->keys[i + 1] = node->keys[i];
            i--;
        }
        node->keys[i + 1] = key;
        node->count += 1;
    } else {
        while (i >= 0 && key < node->keys[i]) {
            i--;
        }
        i++;
        if (node->children[i]->count == ORDER - 1) {
            split_child(node, i, node->children[i]);
            if (key > node->keys[i]) {
                i++;
            }
        }
        insert_nonfull(node->children[i], key);
    }
}

static void insert(BTreeNode **root, int key) {
    if (*root == NULL) {
        *root = create_node(true);
        (*root)->keys[0] = key;
        (*root)->count = 1;
        return;
    }
    if ((*root)->count == ORDER - 1) {
        BTreeNode *new_root = create_node(false);
        new_root->children[0] = *root;
        split_child(new_root, 0, *root);
        int i = 0;
        if (key > new_root->keys[0])
            i++;
        insert_nonfull(new_root->children[i], key);
        *root = new_root;
    } else {
        insert_nonfull(*root, key);
    }
}

static BTreeNode* search(BTreeNode *root, int key, int *comparisons) {
    int i = 0;
    while (i < root->count && key > root->keys[i]) {
        (*comparisons)++;
        i++;
    }
    if (i < root->count) {
        (*comparisons)++;
        if (key == root->keys[i])
            return root;
    }
    if (root->leaf)
        return NULL;
    return search(root->children[i], key, comparisons);
}

int main() {
    const int NUM_INSERT = 1000;
    const int NUM_SEARCH = 100;
    BTreeNode *root = NULL;
    srand(42);
    int keys[NUM_INSERT];
    for (int i = 0; i < NUM_INSERT; ++i) {
        int key = rand() % 5000 + 1; // random customer ID
        keys[i] = key;
        insert(&root, key);
    }

    int total_comparisons = 0;
    clock_t start = clock();
    for (int i = 0; i < NUM_SEARCH; ++i) {
        int idx = rand() % NUM_INSERT;
        int comparisons = 0;
        search(root, keys[idx], &comparisons);
        total_comparisons += comparisons;
    }
    clock_t end = clock();

    double avg_comp = (double)total_comparisons / NUM_SEARCH;
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    double avg_time = elapsed / NUM_SEARCH;

    printf("Average comparisons: %.2f\n", avg_comp);
    printf("Average search time: %.6f seconds\n", avg_time);

    return 0;
}

