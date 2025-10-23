#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define TABLE_SIZE 10000
#define NUM_INSERTIONS TABLE_SIZE
#define LOAD_FACTOR_STEPS 100
#define STEP_SIZE (NUM_INSERTIONS / LOAD_FACTOR_STEPS)

// --- Data Structures ---

// 1. Separate Chaining Node
typedef struct Node {
    int key;
    struct Node *next;
} Node;

// Hash Table for Chaining
Node* chaining_table[TABLE_SIZE];

// Hash Table for Probing (Open Addressing)
#define EMPTY_SLOT -1
int probing_table[TABLE_SIZE];


// --- Utility Functions ---

// Basic Hash Function
int hash1(int key) {
    return key % TABLE_SIZE;
}

// Second Hash Function for Double Hashing (must be non-zero)
int hash2(int key) {
    // A common choice: R - (key % R), where R is a prime slightly less than TABLE_SIZE
    return 7 - (key % 7);
}

// Function to generate keys that cause maximal clustering (Worst Case)
int generate_worst_case_key(int index) {
    // Generate keys that all map to the same small set of initial slots
    // For example, keys that are multiples of 1000 + a small offset
    return (index * 100) + (index % 5);
}

// Function to generate skewed keys
int generate_skewed_key(int index) {
    // Generate keys that tend to cluster slightly
    return (index * 7) + (rand() % 100);
}

// Function to generate uniform keys
int generate_uniform_key(int index) {
    // Pure random key
    return rand() * index;
}

// Function to clear tables
void initialize_tables() {
    for (int i = 0; i < TABLE_SIZE; i++) {
        chaining_table[i] = NULL;
        probing_table[i] = EMPTY_SLOT;
    }
}

// --- Collision Resolution Techniques (Return Probes) ---

// 1. Separate Chaining
long insert_chaining(int key) {
    int index = hash1(key);
    long probes = 1; // Count the initial index access

    Node *newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) { return 0; } // Allocation failure

    newNode->key = key;
    newNode->next = NULL;

    if (chaining_table[index] == NULL) {
        chaining_table[index] = newNode;
    } else {
        Node *current = chaining_table[index];
        while (current->next != NULL) {
            current = current->next;
            probes++; // Count traversal of existing list
        }
        current->next = newNode;
    }
    return probes;
}

// 2. Linear Probing
long insert_linear_probing(int key) {
    int initial_index = hash1(key);
    long probes = 0;

    for (int i = 0; i < TABLE_SIZE; i++) {
        probes++;
        int index = (initial_index + i) % TABLE_SIZE;

        if (probing_table[index] == EMPTY_SLOT) {
            probing_table[index] = key;
            return probes;
        }
        // Optimization: If the key already exists, stop (though our sim only inserts)
    }
    return probes; // Should not happen if TABLE_SIZE > NUM_INSERTIONS
}

// 3. Quadratic Probing
long insert_quadratic_probing(int key) {
    int initial_index = hash1(key);
    long probes = 0;

    for (int i = 0; i < TABLE_SIZE; i++) {
        probes++;
        // Formula: h(k, i) = (h(k) + i^2) mod M
        int index = (initial_index + i * i) % TABLE_SIZE;

        if (probing_table[index] == EMPTY_SLOT) {
            probing_table[index] = key;
            return probes;
        }
    }
    return probes;
}

// 4. Double Hashing
long insert_double_hashing(int key) {
    int initial_index = hash1(key);
    int step = hash2(key);
    long probes = 0;

    for (int i = 0; i < TABLE_SIZE; i++) {
        probes++;
        // Formula: h(k, i) = (h1(k) + i * h2(k)) mod M
        int index = (initial_index + i * step) % TABLE_SIZE;

        if (probing_table[index] == EMPTY_SLOT) {
            probing_table[index] = key;
            return probes;
        }
    }
    return probes;
}

// --- Simulation Driver ---

void run_simulation(const char* distribution_name, int (*key_generator)(int)) {
    // Total probes and time for all insertions in this run
    long total_probes_chaining = 0;
    long total_probes_lp = 0;
    long total_probes_qp = 0;
    long total_probes_dh = 0;

    clock_t start_time, end_time;
    double total_time_chaining = 0.0;
    double total_time_lp = 0.0;
    double total_time_qp = 0.0;
    double total_time_dh = 0.0;

    // Simulation loop
    for (int i = 0; i < NUM_INSERTIONS; i++) {
        int key = key_generator(i);
        long probes;
        double cpu_time_used;

        // Only record data at set intervals (steps) or at the end
        if (i % STEP_SIZE == 0 || i == NUM_INSERTIONS - 1 || i < 10) {
            // --- Separate Chaining ---
            start_time = clock();
            probes = insert_chaining(key);
            end_time = clock();
            total_probes_chaining += probes;
            cpu_time_used = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
            total_time_chaining += cpu_time_used * 1000.0; // Convert to milliseconds

            // --- Linear Probing ---
            start_time = clock();
            probes = insert_linear_probing(key);
            end_time = clock();
            total_probes_lp += probes;
            cpu_time_used = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
            total_time_lp += cpu_time_used * 1000.0;

            // --- Quadratic Probing ---
            start_time = clock();
            probes = insert_quadratic_probing(key);
            end_time = clock();
            total_probes_qp += probes;
            cpu_time_used = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
            total_time_qp += cpu_time_used * 1000.0;

            // --- Double Hashing ---
            start_time = clock();
            probes = insert_double_hashing(key);
            end_time = clock();
            total_probes_dh += probes;
            cpu_time_used = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
            total_time_dh += cpu_time_used * 1000.0;

            // Calculate Load Factor after i+1 insertions
            double load_factor = (double)(i + 1) / TABLE_SIZE;

            // Print output in CSV format to stdout
            printf("%d,%.6f,%s,%ld,%ld,%ld,%ld,%.6f,%.6f,%.6f,%.6f\n",
                   i + 1, // Key_Index (Number of keys inserted)
                   load_factor,
                   distribution_name,
                   total_probes_chaining,
                   total_probes_lp,
                   total_probes_qp,
                   total_probes_dh,
                   total_time_chaining,
                   total_time_lp,
                   total_time_qp,
                   total_time_dh
            );
        } else {
            // Perform insertion without printing results to keep total time accurate
            total_probes_chaining += insert_chaining(key);
            total_probes_lp += insert_linear_probing(key);
            total_probes_qp += insert_quadratic_probing(key);
            total_probes_dh += insert_double_hashing(key);
        }
    }
}

void cleanup_chaining_table() {
    for (int i = 0; i < TABLE_SIZE; i++) {
        Node *current = chaining_table[i];
        while (current != NULL) {
            Node *temp = current;
            current = current->next;
            free(temp);
        }
    }
}

int main() {
    // Seed the random number generator
    srand(time(NULL));

    // Print CSV Header
    printf("Key_Index,Load_Factor,Distribution,Chaining_Probes,Linear_Probing_Probes,Quadratic_Probing_Probes,Double_Hashing_Probes,Chaining_Time_ms,Linear_Probing_Time_ms,Quadratic_Probing_Time_ms,Double_Hashing_Time_ms\n");

    // --- Run 1: Uniform Distribution (Best Case) ---
    initialize_tables();
    run_simulation("Uniform", generate_uniform_key);
    cleanup_chaining_table();

    // --- Run 2: Skewed Distribution (Clustering) ---
    initialize_tables();
    run_simulation("Skewed", generate_skewed_key);
    cleanup_chaining_table();

    // --- Run 3: Worst Case Distribution (Maximum Collisions) ---
    initialize_tables();
    run_simulation("Worst_Case", generate_worst_case_key);
    cleanup_chaining_table();

    return 0;
}
