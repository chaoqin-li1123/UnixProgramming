#include <iostream>
#include <sys/wait.h>
#include <string>
#include <unistd.h>

#define MAXLINE 100

int main() {
    std::string cmd;
    pid_t pid;
    int status;
    std::cout << "user: " << getuid() << std::endl;
    std::cout << "group: " << getgid() << std::endl;
    while (std::getline(std::cin, cmd, '\n')) {
        pid = fork();
        // Inside the child
        if (pid == 0) {
            status = execlp(cmd.c_str(), cmd.c_str(), (char *)0);
            exit(status);
        }
        else {
            waitpid(pid, &status, 0);
        }
    }
    return 0;
}