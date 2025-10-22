/*
 * Dynamic Hash Table Analyzer - C Core Simulation Engine
 *
 * This program simulates hash table performance using four collision resolution techniques
 * (Separate Chaining, Linear Probing, Quadratic Probing, Double Hashing)
 * under varying load factors and outputs the data in CSV format to stdout.
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

// 1. Separate Chaining Node
typedef struct Node {
    int key;
    struct Node *next;
} Node;

// 2. Open Addressing Array (Probing)
#define EMPTY_SLOT -1

typedef struct HashTable {
    int table[TABLE_SIZE];
    Node* chain_table[TABLE_SIZE];
    int size; // Current number of elements
    double total_probes_chaining;
    double total_probes_linear;
    double total_probes_quadratic;
    double total_probes_double;
} HashTable;

// --- Hash Functions ---

// Primary Hash Function: h1(k) = k mod M
int hash1(int key) {
    return abs(key) % TABLE_SIZE;
}

// Secondary Hash Function for Double Hashing: h2(k) = R - (k mod R)
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

    for (int i = 0; i < TABLE_SIZE; i++) {
        ht->table[i] = EMPTY_SLOT;
        ht->chain_table[i] = NULL;
    }
}

// --- Collision Resolution Techniques (Insertion) ---

// 1. Separate Chaining
void insert_chaining(HashTable *ht, int key) {
    int index = hash1(key);
    int probes = 1; // Count 1 probe for the initial hash

    Node *newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }
    newNode->key = key;
    newNode->next = NULL;

    // Traverse the list to check for duplicates and find the end
    if (ht->chain_table[index] == NULL) {
        ht->chain_table[index] = newNode;
    } else {
        Node *current = ht->chain_table[index];
        while (current->next != NULL) {
            probes++;
            current = current->next;
        }
        probes++; // Count the probe to the last node
        current->next = newNode;
    }

    ht->total_probes_chaining += probes;
}

// 2. Linear Probing
void insert_linear_probing(HashTable *ht, int key) {
    int start_index = hash1(key);
    int probes = 1;

    for (int i = 0; i < TABLE_SIZE; i++) {
        int index = (start_index + i) % TABLE_SIZE;

        if (ht->table[index] == EMPTY_SLOT) {
            ht->table[index] = key;
            ht->total_probes_linear += probes;
            ht->size++;
            return;
        }
        probes++;
    }
    fprintf(stderr, "Error: Hash table overflow (Linear Probing).\n");
}

// 3. Quadratic Probing
void insert_quadratic_probing(HashTable *ht, int key) {
    int start_index = hash1(key);
    int probes = 1;

    for (int i = 0; i < TABLE_SIZE; i++) {
        // h(k, i) = (h(k) + i^2) mod M
        int index = (start_index + (i * i)) % TABLE_SIZE;

        if (ht->table[index] == EMPTY_SLOT) {
            ht->table[index] = key;
            ht->total_probes_quadratic += probes;
            ht->size++;
            return;
        }
        probes++;
    }
    fprintf(stderr, "Error: Hash table overflow (Quadratic Probing).\n");
}

// 4. Double Hashing
void insert_double_hashing(HashTable *ht, int key) {
    int start_index = hash1(key);
    int step = hash2(key);
    int probes = 1;

    for (int i = 0; i < TABLE_SIZE; i++) {
        // h(k, i) = (h1(k) + i * h2(k)) mod M
        int index = (start_index + i * step) % TABLE_SIZE;

        if (ht->table[index] == EMPTY_SLOT) {
            ht->table[index] = key;
            ht->total_probes_double += probes;
            ht->size++;
            return;
        }
        probes++;
    }
    fprintf(stderr, "Error: Hash table overflow (Double Hashing).\n");
}

// --- Key Generation ---

// Generates uniform random keys
int generate_uniform_key() {
    return rand();
}

// Generates keys designed to cause clustering (skewed distribution)
int generate_skewed_key() {
    // Keys are mostly multiples of 100, which will cause poor distribution
    // if a hash function relies only on simple division.
    return (rand() % (TABLE_SIZE / 100)) * 100 + (rand() % 5);
}

// --- Simulation Driver ---

void run_simulation(const char* distribution_name, int (*key_generator)()) {
    HashTable ht;
    init_hash_table(&ht);

    double num_insertions = 0;
    double keys_processed = 0;

    for (int i = 1; i <= MAX_INSERTIONS; i++) {
        int key = key_generator();

        // Separate Chaining doesn't worry about load factor > 1
        insert_chaining(&ht, key);

        // Open addressing only inserts if load factor is reasonable
        if ((double)ht.size < TABLE_SIZE * 0.95) {
            // Note: We use ht.size only to track open addressing fullness
            insert_linear_probing(&ht, key);
            // Re-initialize for the next probing technique simulation
            ht.size = ht.size - 1; // undo size increase from linear probing
            insert_quadratic_probing(&ht, key);
            ht.size = ht.size - 1;
            insert_double_hashing(&ht, key);
        } else {
             // Stop open addressing insertion when table gets too full (alpha > ~1.0)
        }

        keys_processed++;

        // Record metrics at fixed intervals
        if ((int)keys_processed % STEP_SIZE == 0 && keys_processed > 0) {
            double load_factor = keys_processed / TABLE_SIZE;

            double avg_probes_chaining = ht.total_probes_chaining / keys_processed;
            double avg_probes_linear = ht.total_probes_linear / keys_processed;
            double avg_probes_quadratic = ht.total_probes_quadratic / keys_processed;
            double avg_probes_double = ht.total_probes_double / keys_processed;

            // Print CSV row to stdout
            printf("%s,%.4f,%.4f,%.4f,%.4f,%.4f\n",
                   distribution_name,
                   load_factor,
                   avg_probes_chaining,
                   avg_probes_linear,
                   avg_probes_quadratic,
                   avg_probes_double);
        }
    }
}

// --- Main ---

int main() {
    // Seed the random number generator
    srand(time(NULL));

    // Print CSV header to stdout
    printf("Distribution,Load_Factor,Chaining_Probes,Linear_Probing_Probes,Quadratic_Probing_Probes,Double_Hashing_Probes\n");

    // Run simulation for Uniform Keys
    fprintf(stderr, "Running simulation for Uniform Keys...\n");
    run_simulation("Uniform", generate_uniform_key);

    // Run simulation for Skewed Keys
    fprintf(stderr, "Running simulation for Skewed Keys...\n");
    run_simulation("Skewed", generate_skewed_key);

    fprintf(stderr, "Simulation complete. Data written to stdout.\n");
    return 0;
}

