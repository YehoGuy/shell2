#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "LineParser.h" 
#include <signal.h> // Added for signal handling
#include <fcntl.h>
#include <ctype.h>


#define MAX_INPUT_SIZE 2048

int debug_mode = 0;

// Function Prototypes
void execute(cmdLine *pCmdLine);
void handle_signal_command(cmdLine *parsedLine);
void handle_pipeline(cmdLine *pCmdLine); 
void printCWD();

// History!
void addToHistory(const char *cmdLine);
void printHistory();
char* getHistoryCommand(int index);
void freeHistory();

#define HISTLEN 10
// A node in the history linked list
typedef struct historyNode {
    char *cmdLine;                
    struct historyNode *next;
} historyNode;
// Keep track of head, tail, and current history size
// for O(1) insertions and deletions
historyNode *historyHead = NULL;
historyNode *historyTail = NULL;
int historySize = 0;

void addToHistory(const char *cmdLine) {
    // Create new node
    historyNode *newNode = (historyNode*)malloc(sizeof(historyNode));
    newNode->cmdLine = strdup(cmdLine);  // store a copy
    newNode->next = NULL;

    // If empty list
    if (!historyHead) {
        historyHead = historyTail = newNode;
        historySize = 1;
        return;
    }

    // If we haven't reached HISTLEN yet
    if (historySize < HISTLEN) {
        historyTail->next = newNode;
        historyTail = newNode;
        historySize++;
    } else {
        // Remove oldest (head) to make room
        historyNode *oldHead = historyHead;
        historyHead = historyHead->next;
        free(oldHead->cmdLine);
        free(oldHead);

        // Insert new command at the tail
        historyTail->next = newNode;
        historyTail = newNode;
        // historySize remains HISTLEN
    }
}

void printHistory() {
    historyNode *curr = historyHead;
    int idx = 1;
    printf("Command History:\n");
    while (curr) {
        printf("%d: %s\n", idx, curr->cmdLine);
        curr = curr->next;
        idx++;
    }
}

char* getHistoryCommand(int index) {
    if (index < 1 || index > historySize) {
        return NULL;
    }
    historyNode *curr = historyHead;
    for (int i = 1; i < index; i++) {
        curr = curr->next;  // move forward
    }
    return curr->cmdLine;  // no need to strdup here, just return the pointer
}

void freeHistory() {
    historyNode *curr = historyHead;
    while (curr) {
        historyNode *next = curr->next;
        free(curr->cmdLine);
        free(curr);
        curr = next;
    }
    historyHead = historyTail = NULL;
    historySize = 0;
}


// define process list
#define TERMINATED  -1
#define RUNNING 1
#define SUSPENDED 0

typedef struct process{
        cmdLine* cmd;                         /* the parsed command line*/
        pid_t pid; 		                  /* the process id that is running the command*/
        int status;                           /* status of the process: RUNNING/SUSPENDED/TERMINATED */
        struct process *next;	                  /* next process in chain */
    } process;

process *process_list = NULL; // Define process_list globally

// function prototypes
void addProcess(process** process_list, cmdLine* cmd, pid_t pid);
void printProcessList(process** process_list);
void freeProcessList(process* process_list);
void updateProcessList(process **process_list);
void updateProcessStatus(process* process_list, int pid, int status);
void deleteProcess(process** process_list, pid_t pid);

void addProcess(process** process_list, cmdLine* cmd, pid_t pid){
    process* new_process = (process*)malloc(sizeof(process));
    if (new_process == NULL) {
        perror("Failed to allocate memory for new process");
        exit(1); // Exit or handle gracefully
    }
    new_process->cmd = cmd;
    new_process->pid = pid;
    new_process->status = RUNNING;
    new_process->next = NULL;
    if(*process_list == NULL){
        *process_list = new_process;
    }else{
        new_process->next = *process_list;
        *process_list = new_process;
    }
}

//TODO call freeProcessList in the end of the main
void freeProcessList(process* process_list){
    process *ptr = process_list;
    process *next;
    while(ptr != NULL) {
        next = ptr->next;
        freeCmdLines(ptr->cmd);
        free(ptr);
        ptr = next;
    }
}


// This method checks the status of each process in the list using waitpid() with WNOHANG 
// and updates the process state accordingly.
void updateProcessList(process **process_list) {
    // Start at first child process. Skip the parent process
    process *current = *process_list; 
    int status;                      

    while (current->next != NULL) { //to not go over main process
        // Use waitpid with WNOHANG to check the process status without blocking
        pid_t result = waitpid(current->pid, &status, WNOHANG | WUNTRACED | WCONTINUED);

        if (result == -1) { // likely because the process no longer exists
            current->status = TERMINATED;
        } else if (result > 0) { // A status change occurred
            if (WIFEXITED(status) || WIFSIGNALED(status)) {
                // Process has exited or was killed; mark as TERMINATED
                current->status = TERMINATED;
            } else if (WIFSTOPPED(status)) {
                // Process is currently stopped; mark as SUSPENDED
                current->status = SUSPENDED;
            } else if (WIFCONTINUED(status)) {
                // Process has continued (resumed execution); mark as RUNNING
                current->status = RUNNING;
            }
        }
        // Move to the next process in the list
        current = current->next;
    }
}

void updateProcessStatus(process* process_list, int pid, int status){
    process *ptr = process_list;
    while(ptr != NULL) {
        if(ptr->pid == pid){
            ptr->status = status;
            return;
        }
        ptr = ptr->next;
    }
}


void printProcessList(process **process_list) {
    // Update process statuses before printing
    updateProcessList(process_list);

    process *ptr = *process_list; // Start at the head of the process list
    int index = 0;               // Index counter for printing

    while (ptr != NULL) {
        // Print process details: index, PID, status, and the command
        printf("Index: %d. PID: %d, Status: %d, Command: ", index, ptr->pid, ptr->status);
        for (int i = 0; i < ptr->cmd->argCount; i++) {
            printf("%s ", ptr->cmd->arguments[i]);
        }
        if(ptr->cmd->next!=NULL){
            printf("| ");
            for (int i = 0; i < ptr->cmd->next->argCount; i++) {
                printf("%s ", ptr->cmd->next->arguments[i]);
            }
        }
        printf("\n");
        if(ptr->status == TERMINATED){
            process *tmp = ptr;
            ptr = ptr->next;
            deleteProcess(process_list, tmp->pid);
            continue;
        } else{
            ptr = ptr->next;
        }
        index++;         // Increment the index counter
    }
}

void deleteProcess(process** process_list, pid_t pid){
    process *ptr = *process_list;
    process *prev = NULL;
    while(ptr != NULL) {
        if(ptr->pid == pid){
            if(prev == NULL){
                *process_list = ptr->next;
            }else{
                prev->next = ptr->next;
            }
            freeCmdLines(ptr->cmd);
            free(ptr);
            return;
        }
        prev = ptr;
        ptr = ptr->next;
    }
}



void execute(cmdLine *pCmdLine) {
    if (pCmdLine->next != NULL) {
        // Handle pipeline if there's a next command in the chain
        handle_pipeline(pCmdLine);
        return;
    }

    // Handle input redirection
    if (pCmdLine->inputRedirect) {
        int input_fd = open(pCmdLine->inputRedirect, O_RDONLY);
        if (input_fd == -1) {
            perror("Error opening input file");
            return;
        }
        if (dup2(input_fd, STDIN_FILENO) == -1) {
            perror("Error redirecting input");
            close(input_fd);
            return;
        }
        close(input_fd);
    }

    // Handle output redirection
    if (pCmdLine->outputRedirect) {
        int output_fd = open(pCmdLine->outputRedirect, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (output_fd == -1) {
            perror("Error opening output file");
            return;
        }
        if (dup2(output_fd, STDOUT_FILENO) == -1) {
            perror("Error redirecting output");
            close(output_fd);
            return;
        }
        close(output_fd);
    }

    // Execute the command
    if (execvp(pCmdLine->arguments[0], pCmdLine->arguments) == -1) {
        perror("Execution failed");
        _exit(1); // Exit child process if exec fails
    }
}

// updated for 3.c
void handle_signal_command(cmdLine *parsedLine) {
    if (parsedLine->argCount < 2) {
        fprintf(stderr, "Error: Missing PID argument\n");
        return;
    }

    // Convert PID argument to integer
    int pid = atoi(parsedLine->arguments[1]);
    if (pid <= 0) {
        fprintf(stderr, "Error: Invalid PID\n");
        return;
    }

    int killResult = 0;

    if (strcmp(parsedLine->arguments[0], "stop") == 0) {
        // Send SIGTSTP (like Ctrl+Z)
        killResult = kill(pid, SIGTSTP);
        if (killResult == -1) {
            perror("Failed to send SIGTSTP");
        } else {
            if (debug_mode) fprintf(stderr, "Sent SIGTSTP (stop) to process %d\n", pid);
            updateProcessStatus(process_list, pid, SUSPENDED);
        }
    } 
    else if (strcmp(parsedLine->arguments[0], "term") == 0) {
        // Send SIGINT (like Ctrl+C)
        killResult = kill(pid, SIGINT);
        if (killResult == -1) {
            perror("Failed to send SIGINT");
        } else {
            if (debug_mode) fprintf(stderr, "Sent SIGINT (term) to process %d\n", pid);
            updateProcessStatus(process_list, pid, TERMINATED);
        }
    } 
    else if (strcmp(parsedLine->arguments[0], "wake") == 0) {
        // Send SIGCONT
        killResult = kill(pid, SIGCONT);
        if (killResult == -1) {
            perror("Failed to send SIGCONT");
        } else {
            if (debug_mode) fprintf(stderr, "Sent SIGCONT (wake) to process %d\n", pid);
            updateProcessStatus(process_list, pid, RUNNING);
        }
    } 
    else {
        fprintf(stderr, "Error: Unknown signal command\n");
    }
}



void handle_pipeline(cmdLine *pCmdLine) {
    int pipe_fd[2];
    pid_t left_pid, right_pid;

    // Create a pipe
    if (pipe(pipe_fd) == -1) {
        perror("Pipe creation failed");
        return;
    }

    // Fork the left-hand process
    left_pid = fork();
    if (left_pid == -1) {
        perror("Fork failed for left-hand process");
        return;
    }

    if (left_pid == 0){
        // Left-hand child process
        close(pipe_fd[0]); // Close the read-end (not used by left-hand process)
        close(STDOUT_FILENO); // Close stdout
        dup(pipe_fd[1]); // Redirect stdout to pipe write-end
        close(pipe_fd[1]); // Close the duplicated write-end

        // Execute the left-hand command
        execvp(pCmdLine->arguments[0], pCmdLine->arguments); //terminates child process after execution.
        perror("Execution failed for left-hand command");
        _exit(1); // Exit with error if exec fails
    }

    // Fork the right-hand process
    right_pid = fork();
    if (right_pid == -1) {
        perror("Fork failed for right-hand process");
        return;
    }

    if (right_pid == 0){
        // Right-hand child process
        close(pipe_fd[1]); // Close the write-end (not used by right-hand process)
        close(STDIN_FILENO); // Close stdin
        dup(pipe_fd[0]); // Redirect stdin to pipe read-end
        close(pipe_fd[0]); // Close the duplicated read-end

        // Execute the right-hand command
        printf("\n");
        execvp(pCmdLine->next->arguments[0], pCmdLine->next->arguments); //terminates child process after execution
        perror("Execution failed for right-hand command");
        _exit(1); // Exit with error if exec fails
    }

    // Parent process
    close(pipe_fd[0]); // Close the read-end in the parent process
    close(pipe_fd[1]); // Close the write-end in the parent process

    // Wait for both child processes to terminate
    waitpid(left_pid, NULL, 0);
    waitpid(right_pid, NULL, 0);

    if (debug_mode) fprintf(stderr, "Parent: Finished waiting for child processes\n");
    printCWD();
    return;
}



void printCWD() {
    char cwd[4096];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s> ", cwd);
        fflush(stdout);
    } else {
        perror("getcwd() error");
    }
}


int main(int argc, char **argv) {
    char input[MAX_INPUT_SIZE];     // Buffer to store user input
    cmdLine *parsedLine;            // Parsed command structure

    // Check for debug mode (-d flag)
    if (argc > 1 && (strcmp(argv[1], "-d") == 0 || strcmp(argv[1], "-D") == 0)) {
        debug_mode = 1;
        fprintf(stderr, "Debug mode activated\n");
    }

    // add current process:
    if(debug_mode)
        addProcess(&process_list, parseCmdLines("./myshell -d"), getpid());
    else
        addProcess(&process_list, parseCmdLines("./myshell"), getpid());

        
    while (1) {
        // Display the prompt (current working directory)
        printCWD();

        // Read user input
        if (fgets(input, MAX_INPUT_SIZE, stdin) == NULL) {
            perror("fgets failed");
            continue;
        }

        // Remove trailing newline from input
        input[strcspn(input, "\n")] = '\0';

        // Skip empty input
        if (strlen(input) == 0) continue;

        // Check for "quit" command
        if (strcmp(input, "quit") == 0) {
            if (debug_mode) fprintf(stderr, "Exiting shell...\n");
            break;
        }

    // handling commands with respect to history mechanism:
        if (strcmp(input, "!!") == 0) {
            // If no commands in history yet
            if (historySize == 0) {
                fprintf(stderr, "No commands in history\n");
                continue;  // Skip the rest of the loop
            }

            char *lastCmd = getHistoryCommand(historySize);
            if (!lastCmd) {
                fprintf(stderr, "Error retrieving last command\n");
                continue;
            }
            strcpy(input, lastCmd); // runtime copy
        }

        // handle !n command
        if (input[0] == '!' && isdigit(input[1])) {
            int n = atoi(&input[1]);   // parse the number after '!'

            // Attempt to retrieve the nth command from history
            char *histCmd = getHistoryCommand(n);
            if (!histCmd) {
                fprintf(stderr, "Error: no such entry in history\n");
                continue;
            }
            strcpy(input, histCmd); 
        }

        // Add the command to history
        addToHistory(input);

        if(strcspn(input, "procs") == 0){
            printProcessList(&process_list);
            continue;
        }

        if(strcspn(input, "history") == 0){
            printHistory();
            continue;
        }

        // Parse the command
        parsedLine = parseCmdLines(input);
        if (parsedLine == NULL) {
            if (debug_mode) fprintf(stderr, "Failed to parse command\n");
            continue;
        }

        // Handle "cd" built-in command
        if (strcmp(parsedLine->arguments[0], "cd") == 0) {
            if (parsedLine->argCount < 2) {
                fprintf(stderr, "cd: Missing argument\n");
            } else if (chdir(parsedLine->arguments[1]) == -1) {
                perror("cd failed");
            } else if (debug_mode) {
                fprintf(stderr, "Changed directory to: %s\n", parsedLine->arguments[1]);
            }
            continue;
        }

        // Handle signal commands: stop, wake, term
        if (strcmp(parsedLine->arguments[0], "stop") == 0 ||
            strcmp(parsedLine->arguments[0], "wake") == 0 ||
            strcmp(parsedLine->arguments[0], "term") == 0) {
            handle_signal_command(parsedLine);
            continue;
        }

        // Fork a new process to execute the command
        int pid = fork();
        if (pid == -1) {
            perror("Fork failed");
            freeCmdLines(parsedLine);
            continue;
        }
        
        if (pid!=0){
            // add new child process to process list
            addProcess(&process_list, parsedLine, pid);
            if(debug_mode) fprintf(stderr, "process %d added to process list\n", pid);
        }

        if (pid == 0) {
            // Child process: execute the command
            if (debug_mode) fprintf(stderr, "Child process (PID: %d) executing command: %s\n", getpid(), parsedLine->arguments[0]);
            execute(parsedLine);
            _exit(0); // Ensure the child process exits after execution
        } else {
            // Parent process
            if (debug_mode) fprintf(stderr, "Parent process (PID: %d) forked child with PID: %d\n", getpid(), pid);
            // Wait for the child process if blocking
            if (parsedLine->blocking) {
                waitpid(pid, NULL, 0);
                if (debug_mode) fprintf(stderr, "Child process (PID: %d) terminated\n", pid);
            }
        }
        
    }

    // Free the process list before exiting
    freeProcessList(process_list);
    freeHistory();
    return 0; // Exit shell successfully
}
