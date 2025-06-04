#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TABLE_SIZE 2097152
#define MIN_DEGREE 2
#define MAX_KEYS (2 * MIN_DEGREE - 1)
#define MAX_CHILDREN (2 * MIN_DEGREE)
#define MAX_CUSTOMERS 1000000

/* Hash table structures */
typedef struct HashNode {
    int id;
    struct HashNode *next;
} HashNode;
static HashNode *hash_table[TABLE_SIZE];

static unsigned hash(int key) { return ((unsigned)key) % TABLE_SIZE; }

static void hash_put(int id) {
    HashNode *n = malloc(sizeof(HashNode));
    if (!n) { perror("malloc"); exit(1); }
    n->id = id;
    unsigned idx = hash(id);
    n->next = hash_table[idx];
    hash_table[idx] = n;
}

static int hash_search(int key, int *comps) {
    unsigned idx = hash(key);
    for (HashNode *n = hash_table[idx]; n; n = n->next) {
        (*comps)++;
        if (n->id == key) return 1;
    }
    return 0;
}

static void hash_free(void) {
    for (size_t i = 0; i < TABLE_SIZE; ++i) {
        HashNode *n = hash_table[i];
        while (n) {
            HashNode *tmp = n; n = n->next; free(tmp);
        }
    }
}

/* B-tree structures */
typedef struct BTreeNode {
    int n;
    int keys[MAX_KEYS];
    struct BTreeNode *children[MAX_CHILDREN];
    int leaf;
} BTreeNode;

static BTreeNode *btree_create_node(int leaf) {
    BTreeNode *node = malloc(sizeof(BTreeNode));
    if (!node) { perror("malloc"); exit(1); }
    node->n = 0;
    node->leaf = leaf;
    for (int i = 0; i < MAX_CHILDREN; ++i) node->children[i] = NULL;
    return node;
}

static void btree_split_child(BTreeNode *x, int i, BTreeNode *y) {
    BTreeNode *z = btree_create_node(y->leaf);
    z->n = MIN_DEGREE - 1;
    for (int j = 0; j < MIN_DEGREE - 1; ++j) z->keys[j] = y->keys[j + MIN_DEGREE];
    if (!y->leaf) {
        for (int j = 0; j < MIN_DEGREE; ++j) z->children[j] = y->children[j + MIN_DEGREE];
    }
    y->n = MIN_DEGREE - 1;
    for (int j = x->n; j >= i + 1; --j) x->children[j + 1] = x->children[j];
    x->children[i + 1] = z;
    for (int j = x->n - 1; j >= i; --j) x->keys[j + 1] = x->keys[j];
    x->keys[i] = y->keys[MIN_DEGREE - 1];
    x->n += 1;
}

static void btree_insert_nonfull(BTreeNode *x, int k) {
    int i = x->n - 1;
    if (x->leaf) {
        while (i >= 0 && k < x->keys[i]) { x->keys[i + 1] = x->keys[i]; --i; }
        x->keys[i + 1] = k; x->n += 1;
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
        *root = s; s->children[0] = r;
        btree_split_child(s, 0, r);
        int i = (k > s->keys[0]);
        btree_insert_nonfull(s->children[i], k);
    } else {
        btree_insert_nonfull(r, k);
    }
}

static int btree_search(BTreeNode *x, int k, int *comps) {
    int i = 0;
    while (i < x->n && k > x->keys[i]) { (*comps)++; ++i; }
    if (i < x->n) { (*comps)++; if (k == x->keys[i]) return 1; }
    if (x->leaf) return 0;
    return btree_search(x->children[i], k, comps);
}

static void btree_free(BTreeNode *node) {
    if (!node) return;
    if (!node->leaf) for (int i = 0; i <= node->n; ++i) btree_free(node->children[i]);
    free(node);
}

/* Utility functions */
static long long diff_micro(struct timespec s, struct timespec e) {
    return (e.tv_sec - s.tv_sec) * 1000000LL + (e.tv_nsec - s.tv_nsec) / 1000LL;
}

static int linear_search(int *arr, size_t n, int key, int *comps) {
    for (size_t i = 0; i < n; ++i) {
        (*comps)++;
        if (arr[i] == key) return 1;
    }
    return 0;
}

static int binary_search_arr(int *arr, size_t n, int key, int *comps) {
    size_t l = 0, r = n;
    while (l < r) {
        size_t m = l + (r - l) / 2;
        (*comps)++;
        if (arr[m] == key) return 1;
        else if (key < arr[m]) r = m;
        else l = m + 1;
    }
    return 0;
}

static int cmp_int(const void *a, const void *b) {
    int ia = *(const int*)a, ib = *(const int*)b;
    return (ia > ib) - (ia < ib);
}

static int load_ids(const char *path, int **out, size_t *count) {
    FILE *fp = fopen(path, "r");
    if (!fp) { perror(path); return -1; }
    unsigned char bom[3];
    if (fread(bom,1,3,fp)==3) {
        if (!(bom[0]==0xEF && bom[1]==0xBB && bom[2]==0xBF)) fseek(fp,0,SEEK_SET);
    }
    char line[256];
    if (!fgets(line,sizeof(line),fp)) { fclose(fp); return -1; }
    int *ids = malloc(sizeof(int)*MAX_CUSTOMERS);
    if (!ids) { perror("malloc"); fclose(fp); return -1; }
    size_t cnt=0;
    while (cnt<MAX_CUSTOMERS && fgets(line,sizeof(line),fp)) {
        char *tok = strtok(line, ",");
        if (!tok) continue;
        ids[cnt++] = atoi(tok);
    }
    fclose(fp);
    *out = ids; *count = cnt; return 0;
}

int main(int argc, char *argv[]) {
    const char *csv = (argc > 1) ? argv[1] : "customers.csv";
    int *ids = NULL; size_t count = 0;
    if (load_ids(csv, &ids, &count) != 0) return 1;

    int *sorted = malloc(sizeof(int)*count);
    if (!sorted) { perror("malloc"); free(ids); return 1; }
    memcpy(sorted, ids, sizeof(int)*count);
    qsort(sorted, count, sizeof(int), cmp_int);

    BTreeNode *root = NULL;
    for (size_t i=0;i<count;++i) {
        hash_put(ids[i]);
        btree_insert(&root, ids[i]);
    }

    srand((unsigned)time(NULL));

    long long t_linear=0, c_linear=0;
    long long t_binary=0, c_binary=0;
    long long t_hash=0, c_hash=0;
    long long t_btree=0, c_btree=0;
    for (int i=0;i<100;++i) {
        int key = ids[rand()%count];
        struct timespec s,e;
        int comps;

        comps=0; clock_gettime(CLOCK_MONOTONIC,&s); linear_search(ids,count,key,&comps); clock_gettime(CLOCK_MONOTONIC,&e); t_linear+=diff_micro(s,e); c_linear+=comps;
        comps=0; clock_gettime(CLOCK_MONOTONIC,&s); binary_search_arr(sorted,count,key,&comps); clock_gettime(CLOCK_MONOTONIC,&e); t_binary+=diff_micro(s,e); c_binary+=comps;
        comps=0; clock_gettime(CLOCK_MONOTONIC,&s); hash_search(key,&comps); clock_gettime(CLOCK_MONOTONIC,&e); t_hash+=diff_micro(s,e); c_hash+=comps;
        comps=0; clock_gettime(CLOCK_MONOTONIC,&s); btree_search(root,key,&comps); clock_gettime(CLOCK_MONOTONIC,&e); t_btree+=diff_micro(s,e); c_btree+=comps;
    }

    printf("Linear Search: avg comps %.2f, avg time %.2f us\n", (double)c_linear/100.0, (double)t_linear/100.0);
    printf("Binary Search: avg comps %.2f, avg time %.2f us\n", (double)c_binary/100.0, (double)t_binary/100.0);
    printf("Hash Table Search: avg comps %.2f, avg time %.2f us\n", (double)c_hash/100.0, (double)t_hash/100.0);
    printf("B-tree Search: avg comps %.2f, avg time %.2f us\n", (double)c_btree/100.0, (double)t_btree/100.0);

    /* cleanup */
    free(ids); free(sorted); hash_free(); btree_free(root);
    return 0;
}

