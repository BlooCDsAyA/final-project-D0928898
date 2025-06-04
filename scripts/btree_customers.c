#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MIN_DEGREE 2
#define MAX_KEYS (2 * MIN_DEGREE - 1)
#define MAX_CHILDREN (2 * MIN_DEGREE)
#define TOTAL_CUSTOMERS 1000000

typedef struct BTreeNode {
    int n;                       /* number of keys */
    int keys[MAX_KEYS];          /* keys */
    struct BTreeNode *children[MAX_CHILDREN];
    int leaf;                    /* boolean */
} BTreeNode;

static BTreeNode *btree_create_node(int leaf) {
    BTreeNode *node = malloc(sizeof(BTreeNode));
    if (!node) {
        perror("malloc");
        exit(1);
    }
    node->n = 0;
    node->leaf = leaf;
    for (int i = 0; i < MAX_CHILDREN; ++i) node->children[i] = NULL;
    return node;
}

static void btree_split_child(BTreeNode *x, int i, BTreeNode *y) {
    BTreeNode *z = btree_create_node(y->leaf);
    z->n = MIN_DEGREE - 1;
    for (int j = 0; j < MIN_DEGREE - 1; ++j) {
        z->keys[j] = y->keys[j + MIN_DEGREE];
    }
    if (!y->leaf) {
        for (int j = 0; j < MIN_DEGREE; ++j) {
            z->children[j] = y->children[j + MIN_DEGREE];
        }
    }
    y->n = MIN_DEGREE - 1;
    for (int j = x->n; j >= i + 1; --j) {
        x->children[j + 1] = x->children[j];
    }
    x->children[i + 1] = z;
    for (int j = x->n - 1; j >= i; --j) {
        x->keys[j + 1] = x->keys[j];
    }
    x->keys[i] = y->keys[MIN_DEGREE - 1];
    x->n += 1;
}

static void btree_insert_nonfull(BTreeNode *x, int k) {
    int i = x->n - 1;
    if (x->leaf) {
        while (i >= 0 && k < x->keys[i]) {
            x->keys[i + 1] = x->keys[i];
            --i;
        }
        x->keys[i + 1] = k;
        x->n += 1;
    } else {
        while (i >= 0 && k < x->keys[i]) --i;
        ++i;
        if (x->children[i]->n == MAX_KEYS) {
            btree_split_child(x, i, x->children[i]);
            if (k > x->keys[i]) ++i;
        }
        btree_insert_nonfull(x->children[i], k);
    }
}

static void btree_insert(BTreeNode **root, int k) {
    if (*root == NULL) {
        *root = btree_create_node(1);
        (*root)->keys[0] = k;
        (*root)->n = 1;
        return;
    }
    BTreeNode *r = *root;
    if (r->n == MAX_KEYS) {
        BTreeNode *s = btree_create_node(0);
        *root = s;
        s->children[0] = r;
        btree_split_child(s, 0, r);
        int i = (k > s->keys[0]);
        btree_insert_nonfull(s->children[i], k);
    } else {
        btree_insert_nonfull(r, k);
    }
}

static int btree_search(BTreeNode *x, int k, int *comparisons) {
    int i = 0;
    while (i < x->n && k > x->keys[i]) {
        (*comparisons)++;
        ++i;
    }
    if (i < x->n) {
        (*comparisons)++;
        if (k == x->keys[i]) return 1;
    }
    if (x->leaf) return 0;
    return btree_search(x->children[i], k, comparisons);
}

static void btree_free(BTreeNode *node) {
    if (!node) return;
    if (!node->leaf) {
        for (int i = 0; i <= node->n; ++i) {
            btree_free(node->children[i]);
        }
    }
    free(node);
}

int main(void) {
    FILE *fp = fopen("customers.csv", "r");
    if (!fp) {
        perror("customers.csv");
        return 1;
    }
    unsigned char bom[3];
    if (fread(bom, 1, 3, fp) == 3) {
        if (!(bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF)) {
            fseek(fp, 0, SEEK_SET);
        }
    }
    char line[256];
    if (!fgets(line, sizeof(line), fp)) {
        fprintf(stderr, "Failed to read header\n");
        fclose(fp);
        return 1;
    }
    int *ids = malloc(sizeof(int) * TOTAL_CUSTOMERS);
    if (!ids) {
        perror("malloc ids");
        fclose(fp);
        return 1;
    }
    size_t count = 0;
    BTreeNode *root = NULL;
    while (fgets(line, sizeof(line), fp)) {
        char *p = strtok(line, ",");
        if (!p) continue;
        int id = atoi(p);
        ids[count++] = id;
        btree_insert(&root, id);
    }
    fclose(fp);

    if (count == 0) {
        fprintf(stderr, "No records loaded\n");
        free(ids);
        return 1;
    }

    srand((unsigned int)time(NULL));
    double total_time = 0.0;
    long long total_comparisons = 0;
    for (int i = 0; i < 100; ++i) {
        int key = ids[rand() % count];
        int comps = 0;
        struct timespec start, end;
        clock_gettime(CLOCK_MONOTONIC, &start);
        btree_search(root, key, &comps);
        clock_gettime(CLOCK_MONOTONIC, &end);
        long long micros = (end.tv_sec - start.tv_sec) * 1000000LL +
                           (end.tv_nsec - start.tv_nsec) / 1000LL;
        total_time += micros;
        total_comparisons += comps;
    }
    printf("Average comparisons: %.2f\n", (double)total_comparisons / 100.0);
    printf("Average time (microseconds): %.2f\n", total_time / 100.0);

    btree_free(root);
    free(ids);
    return 0;
}

