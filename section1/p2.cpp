#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <cctype>
#include <cstdlib>
#include <algorithm>


struct Message {
    long mtype;
    char mtext[100];
};

int main() {
    key_t key = ftok("/tmp", 'A');
    if (key == -1) {
        std::cerr << "ftok failed!" << std::endl;
        return 1;
    }

    int msgQueueID = msgget(key, 0666 | IPC_CREAT);
    if (msgQueueID == -1) {
        std::cerr << "msgget failed!" << std::endl;
        return 1;
    }

    pid_t childPid = fork();

    if (childPid == -1) {
        std::cerr << "Fork failed!" << std::endl;
        return 1;
    } else if (childPid == 0) {
        Message message;
        if (msgrcv(msgQueueID, &message, sizeof(message.mtext), 1, 0) == -1) {
            std::cerr << "Error receiving message in child process" << std::endl;
            return 1;
        }

        std::transform(message.mtext, message.mtext + strlen(message.mtext), message.mtext, ::toupper);
        std::cout << "Child received: " << message.mtext << std::endl;

        message.mtype = 2;
        if (msgsnd(msgQueueID, &message, sizeof(message.mtext), 0) == -1) {
            std::cerr << "Error sending message from child process" << std::endl;
            return 1;
        }
    } else {
        Message message;
        std::strcpy(message.mtext, "Hello, child process!");
        message.mtype = 1;
        if (msgsnd(msgQueueID, &message, sizeof(message.mtext), 0) == -1) {
            std::cerr << "Error sending message from parent process" << std::endl;
            return 1;
        }

        if (msgrcv(msgQueueID, &message, sizeof(message.mtext), 2, 0) == -1) {
            std::cerr << "Error receiving message in parent process" << std::endl;
            return 1;
        }

        std::cout << "Parent received: " << message.mtext << std::endl;
    }

    msgctl(msgQueueID, IPC_RMID, nullptr);

    return 0;
}