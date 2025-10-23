#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define TABLE_SIZE 10000
#define NUM_INSERTIONS TABLE_SIZE
#define MICRO_SIZE 10
#define MICRO_TABLE_SIZE 13 // Small prime size for micro-analysis

#define LOAD_FACTOR_STEPS 100
#define STEP_SIZE (NUM_INSERTIONS / LOAD_FACTOR_STEPS)

// --- Data Structures ---

// 1. Separate Chaining Node
typedef struct Node {
    int key;
    struct Node *next;
} Node;

// Hash Table arrays (dynamically sized based on run context)
Node** chaining_table_ptr;
int* probing_table_ptr;
int current_table_size;

#define EMPTY_SLOT -1

// --- Utility Functions ---

// Basic Hash Function
int hash1(int key) {
    return key % current_table_size;
}

// Second Hash Function for Double Hashing
int hash2(int key) {
    return 7 - (key % 7);
}

// Key Generation Functions (remain the same)
int generate_worst_case_key(int index) {
    // Generate keys that all map to the same small set of initial slots
    return (index * 100) + (index % 5);
}

int generate_skewed_key(int index) {
    // Generate keys that tend to cluster slightly
    return (index * 7) + (rand() % 100);
}

int generate_uniform_key(int index) {
    // Pure random key
    return rand() * index;
}

void initialize_tables(int size) {
    current_table_size = size;
    chaining_table_ptr = (Node**)malloc(sizeof(Node*) * size);
    probing_table_ptr = (int*)malloc(sizeof(int) * size);
    
    if (chaining_table_ptr == NULL || probing_table_ptr == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(1);
    }

    for (int i = 0; i < size; i++) {
        chaining_table_ptr[i] = NULL;
        probing_table_ptr[i] = EMPTY_SLOT;
    }
}

void cleanup_tables(int size) {
    for (int i = 0; i < size; i++) {
        Node *current = chaining_table_ptr[i];
        while (current != NULL) {
            Node *temp = current;
            current = current->next;
            free(temp);
        }
    }
    free(chaining_table_ptr);
    free(probing_table_ptr);
}


// --- Collision Resolution Techniques (Return Probes) ---

// 1. Separate Chaining
long insert_chaining(int key) {
    int index = hash1(key);
    long probes = 1;

    Node *newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) { return 0; }

    newNode->key = key;
    newNode->next = NULL;

    if (chaining_table_ptr[index] == NULL) {
        chaining_table_ptr[index] = newNode;
    } else {
        Node *current = chaining_table_ptr[index];
        while (current->next != NULL) {
            current = current->next;
            probes++;
        }
        current->next = newNode;
    }
    return probes;
}

// 2. Linear Probing
long insert_linear_probing(int key) {
    int initial_index = hash1(key);
    long probes = 0;

    for (int i = 0; i < current_table_size; i++) {
        probes++;
        int index = (initial_index + i) % current_table_size;

        if (probing_table_ptr[index] == EMPTY_SLOT) {
            probing_table_ptr[index] = key;
            return probes;
        }
    }
    return probes;
}

// 3. Quadratic Probing
long insert_quadratic_probing(int key) {
    int initial_index = hash1(key);
    long probes = 0;

    for (int i = 0; i < current_table_size; i++) {
        probes++;
        int index = (initial_index + i * i) % current_table_size;

        if (probing_table_ptr[index] == EMPTY_SLOT) {
            probing_table_ptr[index] = key;
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

    for (int i = 0; i < current_table_size; i++) {
        probes++;
        int index = (initial_index + i * step) % current_table_size;

        if (probing_table_ptr[index] == EMPTY_SLOT) {
            probing_table_ptr[index] = key;
            return probes;
        }
    }
    return probes;
}

// --- Simulation Driver ---

void run_simulation(const char* scale_name, const char* distribution_name, int (*key_generator)(int), int num_keys, int table_size) {
    
    initialize_tables(table_size);

    long total_probes_chaining = 0;
    long total_probes_lp = 0;
    long total_probes_qp = 0;
    long total_probes_dh = 0;

    clock_t start_time, end_time;
    double total_time_chaining = 0.0;
    double total_time_lp = 0.0;
    double total_time_qp = 0.0;
    double total_time_dh = 0.0;

    int print_step = (num_keys > 50) ? (num_keys / LOAD_FACTOR_STEPS) : 1;
    if (num_keys <= MICRO_SIZE) { print_step = 1; }

    for (int i = 0; i < num_keys; i++) {
        int key = key_generator(i);
        long probes;
        double cpu_time_used;

        if (i % print_step == 0 || i == num_keys - 1 || i < MICRO_SIZE) {
            // --- Separate Chaining ---
            start_time = clock();
            probes = insert_chaining(key);
            end_time = clock();
            total_probes_chaining += probes;
            cpu_time_used = ((double)(end_time - start_time)) / CLOCKS_PER_SEC;
            total_time_chaining += cpu_time_used * 1000.0; // ms

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

            double load_factor = (double)(i + 1) / table_size;

            // Print output in CSV format
            printf("%d,%.6f,%s,%s,%ld,%ld,%ld,%ld,%.6f,%.6f,%.6f,%.6f\n",
                   i + 1, // Key_Index
                   load_factor,
                   scale_name,
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
            // Insert without printing to keep metrics accurate
            total_probes_chaining += insert_chaining(key);
            total_probes_lp += insert_linear_probing(key);
            total_probes_qp += insert_quadratic_probing(key);
            total_probes_dh += insert_double_hashing(key);
        }
    }
    cleanup_tables(table_size);
}

int main() {
    srand(time(NULL));

    // Print CSV Header
    printf("Key_Index,Load_Factor,Scale,Distribution,Chaining_Probes,Linear_Probing_Probes,Quadratic_Probing_Probes,Double_Hashing_Probes,Chaining_Time_ms,Linear_Probing_Time_ms,Quadratic_Probing_Time_ms,Double_Hashing_Time_ms\n");

    // --- 1. Micro-Scale Simulations (10 Keys) ---
    run_simulation("Micro", "Uniform", generate_uniform_key, MICRO_SIZE, MICRO_TABLE_SIZE);
    run_simulation("Micro", "Skewed", generate_skewed_key, MICRO_SIZE, MICRO_TABLE_SIZE);
    run_simulation("Micro", "Worst_Case", generate_worst_case_key, MICRO_SIZE, MICRO_TABLE_SIZE);

    // --- 2. Macro-Scale Simulations (10,000 Keys) ---
    run_simulation("Macro", "Uniform", generate_uniform_key, NUM_INSERTIONS, TABLE_SIZE);
    run_simulation("Macro", "Skewed", generate_skewed_key, NUM_INSERTIONS, TABLE_SIZE);
    run_simulation("Macro", "Worst_Case", generate_worst_case_key, NUM_INSERTIONS, TABLE_SIZE);

    return 0;
}
