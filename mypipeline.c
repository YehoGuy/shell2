#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int main() {
    // Pipe I/O file descriptors (identifiers): 
    // pipe_fd[0] for reading, 
    // pipe_fd[1] for writing.
    int pipe_fd[2]; 
    pid_t pid1, pid2;

    // 1. Create a pipe
    if (pipe(pipe_fd) == -1) {
        perror("Pipe creation failed");
        exit(1);
    }
    // printf("pipe_fd[0]: %d (read-end)\n", pipe_fd[0]); => pipe_fd[0]: 3 (read-end)
    // printf("pipe_fd[1]: %d (write-end)\n", pipe_fd[1]); => pipe_fd[1]: 4 (write-end)

    // 2. Fork the first child process (child1)
    fprintf(stderr, "(parent_process>forking…)\n");
    pid1 = fork();

    if (pid1 == -1) {
        perror("Fork failed");
        exit(1);
    }

    if (pid1 == 0) {
        // Child process 1 (child1)
        fprintf(stderr, "(child1>redirecting stdout to the write end of the pipe…)\n");

        // 3.1. Close standard output
        close(STDOUT_FILENO);
        // 3.2. Duplicate the write-end of the pipe to standard output
        dup(pipe_fd[1]); // returns the ne descriptor (1)
        // because we close stdout, dup will reuse its descriptor value (1): 
        // redirect data coming to stdout to the pipe write-end
        close(pipe_fd[1]); // 3.3. Close the (write-end) in child1
        

        fprintf(stderr, "(child1>going to execute cmd: ls -l)\n");

        // 3.4. Execute "ls -l"
        execlp("ls", "ls", "-l", NULL);
        // In case exec fails (the child process wont stop)
        perror("Execution of ls -l failed"); 
        exit(1);
    }

    // Parent process
    fprintf(stderr, "(parent_process>created process with id: %d)\n", pid1);
    fprintf(stderr, "(parent_process>closing the write end of the pipe…)\n");
    close(pipe_fd[1]); // 4. Close the write end of the pipe in the parent

    // 5. Fork the second child process (child2)
    fprintf(stderr, "(parent_process>forking…)\n");
    pid2 = fork();

    if (pid2 == -1) {
        perror("Fork failed");
        exit(1);
    }

    if (pid2 == 0) {
        // Child process 2 (child2)
        fprintf(stderr, "(child2>redirecting stdin to the read end of the pipe…)\n");

        // 6.1. Close standard input
        close(STDIN_FILENO);
        // 6.2. Duplicate the read-end of the pipe to standard input
        dup(pipe_fd[0]);
        close(pipe_fd[0]); // 6.3. Close the duplicated file descriptor

        fprintf(stderr, "(child2>going to execute cmd: tail -n 2)\n");

        // 6.4. Execute "tail -n 2"
        execlp("tail", "tail", "-n", "2", NULL);
        perror("Execution of tail -n 2 failed"); // In case exec fails
        exit(1);
    }

    // Parent process
    fprintf(stderr, "(parent_process>created process with id: %d)\n", pid2);
    fprintf(stderr, "(parent_process>closing the read end of the pipe…)\n");
    close(pipe_fd[0]); // 7. Close the read end of the pipe in the parent

    fprintf(stderr, "(parent_process>waiting for child processes to terminate…)\n");
    waitpid(pid1, NULL, 0); // 8. Wait for the first child to finish
    waitpid(pid2, NULL, 0); // 8. Wait for the second child to finish

    fprintf(stderr, "(parent_process>exiting…)\n");
    return 0;

}
