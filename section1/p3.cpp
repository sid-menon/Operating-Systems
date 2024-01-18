#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <cctype>
#include <cstdlib>
#include <algorithm>
#include <sys/sem.h>  // for semaphores

struct SharedMemory {
    int semaphore;  // Semaphore for synchronization
    char mtext[100];
};

int main() {
    key_t key = ftok("/tmp", 'A');
    if (key == -1) {
        std::cerr << "ftok failed!" << std::endl;
        return 1;
    }

    int shmid = shmget(key, sizeof(SharedMemory), 0666 | IPC_CREAT);
    if (shmid == -1) {
        std::cerr << "shmget failed!" << std::endl;
        return 1;
    }

    SharedMemory* sharedMemory = (SharedMemory*)shmat(shmid, nullptr, 0);

    // Initialize semaphore
    union semun {
        int val;
        struct semid_ds *buf;
        unsigned short *array;
    } arg;

    int semid = semget(key, 1, 0666 | IPC_CREAT);
    if (semid == -1) {
        std::cerr << "semget failed!" << std::endl;
        return 1;
    }

    arg.val = 1;  // Initialize semaphore to 1 (binary semaphore)
    if (semctl(semid, 0, SETVAL, arg) == -1) {
        std::cerr << "semctl failed!" << std::endl;
        return 1;
    }

    pid_t childPid = fork();

    if (childPid == -1) {
        std::cerr << "Fork failed!" << std::endl;
        return 1;
    } else if (childPid == 0) {
        // Child process
        
        while (1) {
            // Wait for the semaphore to become available
            struct sembuf waitOperation;
            waitOperation.sem_num = 0;
            waitOperation.sem_op = -1;
            waitOperation.sem_flg = 0;
            if (semop(semid, &waitOperation, 1) == -1) {
                std::cerr << "semop (wait) failed in child process!" << std::endl;
                return 1;
            }

            if (strcmp(sharedMemory->mtext, "exit") == 0) {
                // Exit when the parent sends the "exit" message
                break;
            }

            // Transform and display the message
            std::transform(sharedMemory->mtext, sharedMemory->mtext + strlen(sharedMemory->mtext), sharedMemory->mtext, ::toupper);
            std::cout << "Child received: " << sharedMemory->mtext << std::endl;

            // Release the semaphore
            struct sembuf signalOperation;
            signalOperation.sem_num = 0;
            signalOperation.sem_op = 1;
            signalOperation.sem_flg = 0;
            if (semop(semid, &signalOperation, 1) == -1) {
                std::cerr << "semop (signal) failed in child process!" << std::endl;
                return 1;
            }
        }
    } else {
        // Parent process
        while (1) {
            // Wait for the semaphore to become available
            struct sembuf waitOperation;
            waitOperation.sem_num = 0;
            waitOperation.sem_op = -1;
            waitOperation.sem_flg = 0;
            if (semop(semid, &waitOperation, 1) == -1) {
                std::cerr << "semop (wait) failed in parent process!" << std::endl;
                return 1;
            }

            // Send a message to the child
            std::strcpy(sharedMemory->mtext, "Hello, child process!");

            // Release the semaphore
            struct sembuf signalOperation;
            signalOperation.sem_num = 0;
            signalOperation.sem_op = 1;
            signalOperation.sem_flg = 0;
            if (semop(semid, &signalOperation, 1) == -1) {
                std::cerr << "semop (signal) failed in parent process!" << std::endl;
                return 1;
            }

            if (strcmp(sharedMemory->mtext, "exit") == 0) {
                // Exit when the child sends the "exit" message
                break;
            }

            // Wait for the semaphore to become available
            struct sembuf waitOperation2;
            waitOperation2.sem_num = 0;
            waitOperation2.sem_op = -1;
            waitOperation2.sem_flg = 0;
            if (semop(semid, &waitOperation2, 1) == -1) {
                std::cerr << "semop (wait2) failed in parent process!" << std::endl;
                return 1;
            }

            // Display the message received from the child
            std::cout << "Parent received: " << sharedMemory->mtext << std::endl;

            // Release the semaphore
            struct sembuf signalOperation2;
            signalOperation2.sem_num = 0;
            signalOperation2.sem_op = 1;
            signalOperation2.sem_flg = 0;
            if (semop(semid, &signalOperation2, 1) == -1) {
                std::cerr << "semop (signal2) failed in parent process!" << std::endl;
                return 1;
            }
        }
    }

    // Clean up shared memory and semaphore
    shmdt(sharedMemory);
    shmctl(shmid, IPC_RMID, nullptr);
    semctl(semid, 0, IPC_RMID);

    return 0;
}