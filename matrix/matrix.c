#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define MAX_SIZE 100      // upper bound on matrix size
#define NUM_THREADS 10    // total threads used per operation

int matA[MAX_SIZE][MAX_SIZE];
int matB[MAX_SIZE][MAX_SIZE];

int matSumResult[MAX_SIZE][MAX_SIZE];
int matDiffResult[MAX_SIZE][MAX_SIZE];
int matProductResult[MAX_SIZE][MAX_SIZE];

int MAX = 20;             // default size; can be overridden from the command line

// Work assigned to one thread: a contiguous block of rows [startRow, endRow).
typedef struct {
    int id;
    int startRow;
    int endRow;
} ThreadData;

void fillMatrix(int matrix[MAX_SIZE][MAX_SIZE]) {
    for (int i = 0; i < MAX; i++) {
        for (int j = 0; j < MAX; j++) {
            matrix[i][j] = rand() % 10 + 1;
        }
    }
}

void printMatrix(int matrix[MAX_SIZE][MAX_SIZE]) {
    for (int i = 0; i < MAX; i++) {
        for (int j = 0; j < MAX; j++) {
            printf("%5d", matrix[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

// Each thread adds its block of rows: C[i][j] = A[i][j] + B[i][j]
void* computeSum(void* args) {
    ThreadData* data = (ThreadData*)args;
    for (int i = data->startRow; i < data->endRow; i++) {
        for (int j = 0; j < MAX; j++) {
            matSumResult[i][j] = matA[i][j] + matB[i][j];
        }
    }
    return NULL;
}

// Each thread subtracts its block of rows: C[i][j] = A[i][j] - B[i][j]
void* computeDiff(void* args) {
    ThreadData* data = (ThreadData*)args;
    for (int i = data->startRow; i < data->endRow; i++) {
        for (int j = 0; j < MAX; j++) {
            matDiffResult[i][j] = matA[i][j] - matB[i][j];
        }
    }
    return NULL;
}

// Each thread computes its block of rows of A x B (row of A dot column of B).
void* computeProduct(void* args) {
    ThreadData* data = (ThreadData*)args;
    for (int i = data->startRow; i < data->endRow; i++) {
        for (int j = 0; j < MAX; j++) {
            int sum = 0;
            for (int k = 0; k < MAX; k++) {
                sum += matA[i][k] * matB[k][j];
            }
            matProductResult[i][j] = sum;
        }
    }
    return NULL;
}

// Spawn NUM_THREADS threads running func, then wait for all of them.
void runThreads(void* (*func)(void*), ThreadData threadData[]) {
    pthread_t threads[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(&threads[i], NULL, func, &threadData[i]) != 0) {
            perror("pthread_create");
            exit(1);
        }
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
}

int main(int argc, char* argv[]) {
    srand(time(0));  // Do Not Remove. Just ignore and continue below.

    // 0. Get the matrix size from the command line and assign it to MAX
    if (argc > 1) {
        int n = atoi(argv[1]);
        if (n < 1 || n > MAX_SIZE) {
            fprintf(stderr, "Matrix size must be between 1 and %d\n", MAX_SIZE);
            return 1;
        }
        MAX = n;
    }

    // 1. Fill the matrices (matA and matB) with random values.
    fillMatrix(matA);
    fillMatrix(matB);

    // 2. Print the initial matrices.
    printf("Matrix A:\n");
    printMatrix(matA);
    printf("Matrix B:\n");
    printMatrix(matB);

    // 3. Divide the rows among the threads (divide and conquer).
    //    20 rows / 10 threads = 2 rows each; leftovers go to the first threads.
    ThreadData threadData[NUM_THREADS];
    int rowsPerThread = MAX / NUM_THREADS;
    int extraRows = MAX % NUM_THREADS;
    int currentRow = 0;
    for (int i = 0; i < NUM_THREADS; i++) {
        int rows = rowsPerThread + (i < extraRows ? 1 : 0);
        threadData[i].id = i;
        threadData[i].startRow = currentRow;
        threadData[i].endRow = currentRow + rows;
        currentRow += rows;
    }

    // 4 & 5. Create the threads for each operation and wait for them to finish.
    runThreads(computeSum, threadData);
    runThreads(computeDiff, threadData);
    runThreads(computeProduct, threadData);

    // 6. Print the results.
    printf("Results:\n");
    printf("Sum:\n");
    printMatrix(matSumResult);
    printf("Difference:\n");
    printMatrix(matDiffResult);
    printf("Product:\n");
    printMatrix(matProductResult);

    return 0;
}
