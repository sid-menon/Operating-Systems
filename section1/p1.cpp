#include <iostream>
#include <unistd.h>

int main() {
    pid_t childPid = fork();

    if (childPid == -1) {
        // Fork failed
        std::cerr << "Fork failed!" << std::endl;
        return 1;
    } else if (childPid == 0) {
        // This code is executed by the child process
        std::cout << "Child process (PID " << getpid() << ") is executing." << std::endl;
    } else {
        // This code is executed by the parent process
        std::cout << "Parent process (PID " << getpid() << ") created child process (PID " << childPid << ")." << std::endl;
    }

    return 0;
}