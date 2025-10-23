/*
 * Dynamic Hash Table Analyzer - C Core Simulation Engine (v2.0)
 *
 * Adds: Insertion Timing (Time_ms) and Worst-Case Key generation.
 *
 * To compile: gcc hash_table_analyzer.c -o analyzer -lm
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

// --- Configuration ---
#define TABLE_SIZE 10007 // A large prime number for the hash table size (M)
#define MAX_INSERTIONS 15000 // Max number of keys to insert (N)
#define STEP_SIZE 500 // Record metrics every STEP_SIZE insertions
#define DOUBLE_HASH_R 10003 // Second prime number R < M for double hashing

// --- Data Structures ---
typedef struct Node {
    int key;
    struct Node *next;
} Node;

#define EMPTY_SLOT -1

typedef struct HashTable {
    int table[TABLE_SIZE];
    Node* chain_table[TABLE_SIZE];
    int size;
    double total_probes_chaining;
    double total_probes_linear;
    double total_probes_quadratic;
    double total_probes_double;
    clock_t total_time_linear; // New: Clock cycles for timing
    clock_t total_time_quadratic;
    clock_t total_time_double;
} HashTable;

// --- Hash Functions ---

int hash1(int key) {
    return abs(key) % TABLE_SIZE;
}

int hash2(int key) {
    return DOUBLE_HASH_R - (abs(key) % DOUBLE_HASH_R);
}

// --- Initialization ---

void init_hash_table(HashTable *ht) {
    ht->size = 0;
    ht->total_probes_chaining = 0.0;
    ht->total_probes_linear = 0.0;
    ht->total_probes_quadratic = 0.0;
    ht->total_probes_double = 0.0;
    ht->total_time_linear = 0;
    ht->total_time_quadratic = 0;
    ht->total_time_double = 0;

    for (int i = 0; i < TABLE_SIZE; i++) {
        ht->table[i] = EMPTY_SLOT;
        ht->chain_table[i] = NULL;
    }
}

// --- Collision Resolution Techniques (Insertion with Timing) ---

// 1. Separate Chaining (Timing generally less critical here)
void insert_chaining(HashTable *ht, int key) {
    int index = hash1(key);
    int probes = 1;
    Node *newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) { exit(EXIT_FAILURE); }
    newNode->key = key;
    newNode->next = NULL;

    if (ht->chain_table[index] == NULL) {
        ht->chain_table[index] = newNode;
    } else {
        Node *current = ht->chain_table[index];
        while (current->next != NULL) {
            probes++;
            current = current->next;
        }
        probes++;
        current->next = newNode;
    }
    ht->total_probes_chaining += probes;
}

// 2. Linear Probing
void insert_linear_probing(HashTable *ht, int key) {
    clock_t start = clock();
    int start_index = hash1(key);
    int probes = 1;

    for (int i = 0; i < TABLE_SIZE; i++) {
        int index = (start_index + i) % TABLE_SIZE;
        if (ht->table[index] == EMPTY_SLOT) {
            ht->table[index] = key;
            ht->total_probes_linear += probes;
            ht->size++;
            clock_t end = clock();
            ht->total_time_linear += (end - start);
            return;
        }
        probes++;
    }
    clock_t end = clock();
    ht->total_time_linear += (end - start);
    fprintf(stderr, "Error: Hash table overflow (Linear Probing).\n");
}

// 3. Quadratic Probing
void insert_quadratic_probing(HashTable *ht, int key) {
    clock_t start = clock();
    int start_index = hash1(key);
    int probes = 1;

    for (int i = 0; i < TABLE_SIZE; i++) {
        // h(k, i) = (h(k) + i^2) mod M
        int index = (start_index + (i * i)) % TABLE_SIZE;
        if (ht->table[index] == EMPTY_SLOT) {
            ht->table[index] = key;
            ht->total_probes_quadratic += probes;
            ht->size++;
            clock_t end = clock();
            ht->total_time_quadratic += (end - start);
            return;
        }
        probes++;
    }
    clock_t end = clock();
    ht->total_time_quadratic += (end - start);
    fprintf(stderr, "Error: Hash table overflow (Quadratic Probing).\n");
}

// 4. Double Hashing
void insert_double_hashing(HashTable *ht, int key) {
    clock_t start = clock();
    int start_index = hash1(key);
    int step = hash2(key);
    int probes = 1;

    for (int i = 0; i < TABLE_SIZE; i++) {
        int index = (start_index + i * step) % TABLE_SIZE;
        if (ht->table[index] == EMPTY_SLOT) {
            ht->table[index] = key;
            ht->total_probes_double += probes;
            ht->size++;
            clock_t end = clock();
            ht->total_time_double += (end - start);
            return;
        }
        probes++;
    }
    clock_t end = clock();
    ht->total_time_double += (end - start);
    fprintf(stderr, "Error: Hash table overflow (Double Hashing).\n");
}

// --- Key Generation ---

int generate_uniform_key() {
    return rand();
}

int generate_skewed_key() {
    // Keys that hash to a small range, simulating realistic clustering
    return (rand() % (TABLE_SIZE / 100)) * 100 + (rand() % 5);
}

int generate_worst_case_key() {
    // Keys that all hash to the *exact* same primary index (e.g., index 100)
    // This creates the absolute worst-case scenario, maximizing collision length.
    return (rand() % 1000) * TABLE_SIZE + 100;
}

// --- Simulation Driver ---

void run_simulation(const char* distribution_name, int (*key_generator)()) {
    HashTable ht;
    init_hash_table(&ht);

    double keys_processed = 0;

    for (int i = 1; i <= MAX_INSERTIONS; i++) {
        int key = key_generator();

        insert_chaining(&ht, key);

        if ((double)ht.size < TABLE_SIZE * 0.95) {
            insert_linear_probing(&ht, key);
            ht.size = ht.size - 1;
            insert_quadratic_probing(&ht, key);
            ht.size = ht.size - 1;
            insert_double_hashing(&ht, key);
        }

        keys_processed++;

        if ((int)keys_processed % STEP_SIZE == 0 && keys_processed > 0) {
            double load_factor = keys_processed / TABLE_SIZE;
            double num_keys = keys_processed;

            // Average Probe Calculations
            double avg_probes_chaining = ht.total_probes_chaining / num_keys;
            double avg_probes_linear = ht.total_probes_linear / num_keys;
            double avg_probes_quadratic = ht.total_probes_quadratic / num_keys;
            double avg_probes_double = ht.total_probes_double / num_keys;

            // New: Average Time Calculations (Convert clock cycles to milliseconds)
            double avg_time_linear = ((double)ht.total_time_linear / num_keys) * 1000.0 / CLOCKS_PER_SEC;
            double avg_time_quadratic = ((double)ht.total_time_quadratic / num_keys) * 1000.0 / CLOCKS_PER_SEC;
            double avg_time_double = ((double)ht.total_time_double / num_keys) * 1000.0 / CLOCKS_PER_SEC;

            // Print CSV row to stdout
            printf("%s,%.4f,%.4f,%.4f,%.4f,%.4f,%.8f,%.8f,%.8f\n",
                   distribution_name,
                   load_factor,
                   avg_probes_chaining,
                   avg_probes_linear,
                   avg_probes_quadratic,
                   avg_probes_double,
                   avg_time_linear,
                   avg_time_quadratic,
                   avg_time_double);
        }
    }
}

// --- Main ---

int main() {
    srand(time(NULL));

    // UPDATED CSV HEADER: Added Time_ms metrics
    printf("Distribution,Load_Factor,Chaining_Probes,Linear_Probing_Probes,Quadratic_Probing_Probes,Double_Hashing_Probes,Linear_Time_ms,Quadratic_Time_ms,Double_Time_ms\n");

    // 1. Uniform Keys
    fprintf(stderr, "Running simulation for Uniform Keys...\n");
    run_simulation("Uniform", generate_uniform_key);

    // 2. Skewed Keys (Clustering)
    fprintf(stderr, "Running simulation for Skewed (Clustering) Keys...\n");
    run_simulation("Skewed", generate_skewed_key);

    // 3. Worst-Case Keys
    fprintf(stderr, "Running simulation for Worst_Case Keys (Max Collisions)...\n");
    run_simulation("Worst_Case", generate_worst_case_key);

    fprintf(stderr, "Simulation complete. Data written to stdout.\n");
    return 0;
}

