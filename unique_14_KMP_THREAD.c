#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>

#define LENGTH 10000
#define THREADS 4
#define SEQUENCE_LENGTH 14

bool FOUND = false;

// Generate a random string of the given length
char* get_random_string(int length) {
    char* s = malloc((length + 1) * sizeof(char));
    for (int i = 0; i < length; i++) {
        s[i] = rand() % 26 + 'a';
    }
    s[length] = '\0';
    return s;
}

// KMP search algorithm
int kmp(char* pattern, char* text) {
    int m = strlen(pattern);
    int n = strlen(text);
    if (m > n) {
        return -1;
    }
    int* lps = malloc(m * sizeof(int));
    int j = 0;
    for (int i = 1; i < m; i++) {
        if (pattern[i] == pattern[j]) {
            lps[i] = j + 1;
            j++;
        } else {
            while (j > 0 && pattern[i] != pattern[j]) {
                j = lps[j - 1];
            }
            if (pattern[i] == pattern[j]) {
                lps[i] = j + 1;
                j++;
            } else {
                lps[i] = 0;
            }
        }
    }
    int i = 0;
    j = 0;
    while (i < n) {
        if (pattern[j] == text[i]) {
            i++;
            j++;
        }
        if (j == m) {
            return i - j;
        } else if (i < n && pattern[j] != text[i]) {
            if (j != 0) {
                j = lps[j - 1];
            } else {
                i++;
            }
        }
    }
    return -1;
}

// Find the first sequence of 14 unique characters in the given string
char* find_unique_characters(char* s) {
    int length = strlen(s);
    for (int i = 0; i < length - SEQUENCE_LENGTH; i++) {
        bool unique = true;
        for (int j = i; j < i + SEQUENCE_LENGTH; j++) {
            for (int k = j + 1; k < i + SEQUENCE_LENGTH; k++) {
                if (s[j] == s[k]) {
                    unique = false;
                    break;
                }
            }
            if (!unique) {
                break;
            }
        }
        if (unique) {
            char* sequence = malloc((SEQUENCE_LENGTH + 1) * sizeof(char));
            strncpy(sequence, s + i, SEQUENCE_LENGTH);
            sequence[SEQUENCE_LENGTH] = '\0';
            return sequence;
        }
    }
    return NULL;
}

// Struct to pass data to each thread
typedef struct {
    char* string;
    int start;
    int end;
} thread_data_t;

// Thread function
void* search_thread(void* arg) {
    thread_data_t* data = (thread_data_t*)arg;
    char* sequence = find_unique_characters(data->string + data->start);
    if (sequence != NULL && !FOUND) {
        FOUND = true;
        printf("Sequence found: %s\n", sequence);
        free(sequence);
        pthread_exit(NULL);
    pthread_exit(NULL);
    }       
}

int main(int argc, char* argv[]) {
    srand(time(NULL));
    char* s = get_random_string(LENGTH);

    pthread_t threads[THREADS];
    thread_data_t data[THREADS];
    int chunk_size = LENGTH / THREADS;

    // Record start time
    struct timespec start, finish;
    double elapsed;
    clock_gettime(CLOCK_MONOTONIC, &start);

    // Create threads to search the string
    for (int i = 0; i < THREADS; i++) {
        data[i].string = s;
        data[i].start = i * chunk_size;
        data[i].end = (i + 1) * chunk_size;
        int rc = pthread_create(&threads[i], NULL, search_thread, &data[i]);
        if (rc) {
            printf("Error creating thread\n");
            exit(-1);
        }
    }

    // Wait for threads to finish
    for (int i = 0; i < THREADS; i++) {
        int rc = pthread_join(threads[i], NULL);
        if (rc) {
            printf("Error joining thread\n");
            exit(-1);
        }
    }

    // Record finish time
    clock_gettime(CLOCK_MONOTONIC, &finish);

    // Calculate elapsed time
    elapsed = (finish.tv_sec - start.tv_sec);
    elapsed += (finish.tv_nsec - start.tv_nsec) / 1000000000.0;

    printf("Time elapsed: %f seconds\n", elapsed);

    free(s);
    return 0;
}
