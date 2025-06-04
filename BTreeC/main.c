#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "btree.h"

#define NUM_INSERT 1000
#define NUM_SEARCH 100

int main() {
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

    double avg_comparisons = (double)total_comparisons / NUM_SEARCH;
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    double avg_time = elapsed / NUM_SEARCH;

    printf("Average comparisons: %.2f\n", avg_comparisons);
    printf("Average search time: %.6f seconds\n", avg_time);

    return 0;
}

