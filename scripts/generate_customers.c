/*
 * Generate 1,000,000 unique customer records and write to customers.csv.
 * CustomerID range is 1,000,000~9,999,999 to allow one million unique values.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define TOTAL_CUSTOMERS 1000000
#define ID_START 1000000
#define ID_END   9999999

static void shuffle(int *array, size_t n) {
    if (n > 1) {
        for (size_t i = n - 1; i > 0; --i) {
            size_t j = rand() % (i + 1);
            int temp = array[i];
            array[i] = array[j];
            array[j] = temp;
        }
    }
}

static void random_string(char *str, size_t len) {
    const char charset[] = "abcdefghijklmnopqrstuvwxyz";
    for (size_t i = 0; i < len; ++i) {
        int key = rand() % (int)(sizeof(charset) - 1);
        str[i] = charset[key];
    }
    str[len] = '\0';
}

int main(void) {
    if ((ID_END - ID_START + 1) < TOTAL_CUSTOMERS) {
        fprintf(stderr, "ID range is too small for unique values.\n");
        return 1;
    }

    FILE *fp = fopen("customers.csv", "w");
    if (!fp) {
        perror("Unable to open customers.csv");
        return 1;
    }

    /* Prepare pool of IDs and shuffle */
    int range_size = ID_END - ID_START + 1;
    int *ids = malloc(sizeof(int) * range_size);
    if (!ids) {
        perror("Memory allocation failed");
        fclose(fp);
        return 1;
    }

    for (int i = 0; i < range_size; ++i) {
        ids[i] = ID_START + i;
    }
    shuffle(ids, (size_t)range_size);

    srand((unsigned int)time(NULL));

    fprintf(fp, "CustomerID,Name,Age,Email,Phone\n");

    char name[11];
    char email[32];
    char phone[12];

    for (int i = 0; i < TOTAL_CUSTOMERS; ++i) {
        int name_len = 5 + rand() % 6; /* 5-10 letters */
        random_string(name, name_len);

        int age = 18 + rand() % 82; /* 18-99 */

        snprintf(email, sizeof(email), "%s%d@example.com", name, ids[i]);
        snprintf(phone, sizeof(phone), "09%08d", rand() % 100000000);

        fprintf(fp, "%d,%s,%d,%s,%s\n", ids[i], name, age, email, phone);
    }

    free(ids);
    fclose(fp);
    return 0;
}
