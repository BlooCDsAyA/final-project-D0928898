#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TABLE_SIZE 2097152
#define TOTAL_CUSTOMERS 1000000

typedef struct Customer {
    int id;
    char name[16];
    int age;
    char email[64];
    char phone[16];
    struct Customer *next;
} Customer;

static Customer *hash_table[TABLE_SIZE];

static unsigned hash(int key) {
    return ((unsigned)key) % TABLE_SIZE;
}

static void put(Customer *c) {
    unsigned idx = hash(c->id);
    c->next = hash_table[idx];
    hash_table[idx] = c;
}

static Customer *search(int key) {
    unsigned idx = hash(key);
    for (Customer *c = hash_table[idx]; c; c = c->next) {
        if (c->id == key) return c;
    }
    return NULL;
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

    while (fgets(line, sizeof(line), fp)) {
        char *token = strtok(line, ",");
        if (!token) continue;
        int id = atoi(token);
        Customer *c = malloc(sizeof(Customer));
        if (!c) {
            perror("malloc customer");
            fclose(fp);
            return 1;
        }
        c->id = id;
        token = strtok(NULL, ",");
        if (token) strncpy(c->name, token, sizeof(c->name));
        c->name[sizeof(c->name)-1] = '\0';
        token = strtok(NULL, ",");
        c->age = token ? atoi(token) : 0;
        token = strtok(NULL, ",");
        if (token) strncpy(c->email, token, sizeof(c->email));
        c->email[sizeof(c->email)-1] = '\0';
        token = strtok(NULL, "\n");
        if (token) {
            /* phone is stored as ="09xxxxxxxx" */
            if (token[0] == '=' && token[1] == '"') {
                strncpy(c->phone, token+2, sizeof(c->phone));
                size_t len = strlen(c->phone);
                if (len > 0 && c->phone[len-1] == '"') c->phone[len-1] = '\0';
            } else {
                strncpy(c->phone, token, sizeof(c->phone));
            }
        } else {
            c->phone[0] = '\0';
        }
        c->phone[sizeof(c->phone)-1] = '\0';

        put(c);
        ids[count++] = id;
    }
    fclose(fp);

    if (count == 0) {
        fprintf(stderr, "No records loaded\n");
        free(ids);
        return 1;
    }

    srand((unsigned)time(NULL));
    double total_time = 0.0;
    for (int i = 0; i < 100; ++i) {
        int key = ids[rand() % count];
        struct timespec start, end;
        clock_gettime(CLOCK_MONOTONIC, &start);
        search(key);
        clock_gettime(CLOCK_MONOTONIC, &end);
        long long micros = (end.tv_sec - start.tv_sec) * 1000000LL +
                           (end.tv_nsec - start.tv_nsec) / 1000LL;
        total_time += micros;
    }

    double avg_sec = (total_time / 100.0) / 1000000.0;
    printf("Average search time: %.6f seconds\n", avg_sec);

    free(ids);
    /* free hash table */
    for (size_t i = 0; i < TABLE_SIZE; ++i) {
        Customer *c = hash_table[i];
        while (c) {
            Customer *tmp = c;
            c = c->next;
            free(tmp);
        }
    }
    return 0;
}
