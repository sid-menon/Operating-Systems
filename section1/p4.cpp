#include <iostream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>

#define NUM_CHILDREN 3
#define MESSAGE_SIZE 256

// Structure to represent the shared memory
struct SharedMemory {
    char messages[NUM_CHILDREN][MESSAGE_SIZE];
    int status[NUM_CHILDREN];
};

// Semaphore operations
struct sembuf sem_wait = {0, -1, 0}; // Decrement semaphore
struct sembuf sem_signal = {0, 1, 0}; // Increment semaphore

void childProcess(int childIndex, int shmid, int semid) {
    struct SharedMemory *sharedMemory = (struct SharedMemory *)shmat(shmid, nullptr, 0);
    
    while (true) {
        // Wait for exclusive access to shared memory
        semop(semid, &sem_wait, 1);

        // Check if the message is processed by the parent
        if (sharedMemory->status[childIndex] == 0) {
            // Deposit message
            sprintf(sharedMemory->messages[childIndex], "Message from Child %d", childIndex);
            sharedMemory->status[childIndex] = 1; // Set status to indicate message pending
            std::cout << "Child " << childIndex << ": Message deposited" << std::endl;
        }

        // Release exclusive access to shared memory
        semop(semid, &sem_signal, 1);

        sleep(rand() % 5); // Simulate some work
    }

    shmdt((void *)sharedMemory);
}

void parentProcess(int shmid, int semid) {
    struct SharedMemory *sharedMemory = (struct SharedMemory *)shmat(shmid, nullptr, 0);

    while (true) {
        // Wait for any child to deposit a message
        semop(semid, &sem_wait, 1);

        // Process messages
        for (int i = 0; i < NUM_CHILDREN; ++i) {
            if (sharedMemory->status[i] == 1) {
                std::cout << "Parent: Received message from Child " << i << ": " << sharedMemory->messages[i] << std::endl;
                sharedMemory->status[i] = 2; // Set status to indicate message processed
            }
        }

        // Release exclusive access to shared memory
        semop(semid, &sem_signal, 1);

        sleep(1);
    }

    shmdt((void *)sharedMemory);
}

int main() {
    int shmid = shmget(IPC_PRIVATE, sizeof(struct SharedMemory), IPC_CREAT | 0666);
    int semid = semget(IPC_PRIVATE, 1, IPC_CREAT | 0666);

    // Initialize semaphore
    semctl(semid, 0, SETVAL, 1);

    if (fork() == 0) {
        // Child processes
        int childIndex = 0; // You may need to adjust this index for each child
        childProcess(childIndex, shmid, semid);
        exit(0);
    } else if (fork() == 0) {
        int childIndex = 1; // Adjust index for the second child
        childProcess(childIndex, shmid, semid);
        exit(0);
    } else if (fork() == 0) {
        int childIndex = 2; // Adjust index for the third child
        childProcess(childIndex, shmid, semid);
        exit(0);
    } else {
        // Parent process
        parentProcess(shmid, semid);
    }

    // Cleanup code
    shmctl(shmid, IPC_RMID, nullptr);
    semctl(semid, 0, IPC_RMID);

    return 0;
}