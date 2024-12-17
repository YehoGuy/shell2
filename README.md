My Shell Project
A simple C-based shell implementation demonstrating process management, pipelining, memory management, and command history.

Table of Contents
Overview
Features
Project Structure
Installation & Compilation
Usage
Basic Commands
Built-in Commands
History Feature
Process Management
Examples
License
Overview
My Shell demonstrates low-level Unix concepts including:

Process creation and management via fork() and execvp().
Pipelining (cmd1 | cmd2) using UNIX pipes (pipe()).
Memory management (linked lists for process tracking & command history).
Signal handling to stop, resume, or terminate child processes.
Command history limited to the last 10 commands.
This shell includes an optional debug mode (invoked with -d) which logs child process creation and other internal details.

Features
Execute Commands: Runs standard commands like ls, cat, or echo.
Pipelining: Supports cmd1 | cmd2.
Redirection: Input & output redirection with < and >.
History:
history command to list up to 10 recent commands.
!! to repeat the last command.
!n to re-run the n-th command from the history.
Process List: Tracks child processes with a linked list, storing PID and status.
Signal Handling: Built-in commands to send signals to processes (stop, wake, term).
Project Structure
.
├── myshell.c             # Main shell logic in C
├── LineParser.h       # Provided header for parsing commands
├── LineParser.c       # LineParser - not written by me, but by my proffessor.
├── README.md          # This README
└── Makefile           # (Optional) For building
Key Components
parseCmdLines(char *input)
Splits input string into a structured cmdLine, handling arguments, redirection, and pipeline info.

History

addToHistory(cmdLine), printHistory(), getHistoryCommand(int), freeHistory().
Maintains a linked list of up to 10 commands.
Process Management

addProcess(process** list, cmdLine* cmd, pid_t pid)
printProcessList(process** list)
updateProcessList(process** list)
handle_signal_command(cmdLine *parsedLine)
Pipelining

handle_pipeline(cmdLine *pCmdLine): Sets up a pipe between two commands.
Installation & Compilation
Prerequisites
GCC or another C compiler.
Unix-like environment (Linux, macOS, WSL, etc.).
LineParser.h (either provided or from your assignment / external source).
Steps
Clone or download this repository:

bash

git clone https://github.com/YehoGuy/MyShell.git
cd MyShell

Compile the shell:
make clean
make
./myshell

Usage
When the shell starts, you’ll see a prompt showing your current working directory:

/path/to/directory> 
Type commands, then press Enter.

Basic Commands
Run system commands like ls, pwd, cat filename.txt, etc.

Pipelining:

ls -l | grep main
Redirection:

Input <:

cat < input.txt
Output >:

ls > output.txt
Exit:
Type quit to exit the shell.

Built-in Commands
cd <directory>: Changes current working directory.
procs: Prints the list of child processes (PID, status, and command).
history: Shows up to 10 recent commands.
History Feature
!!: Re-run the last command in history.
!n: Re-run the n-th command (1-based index).
Example usage:

# Suppose history is:
# 1: ls
# 2: pwd
# 3: date
!2   # re-run "pwd"
!!   # re-run "date" (the last command)
Process Management
stop <PID>: Sends SIGTSTP to suspend a process.
wake <PID>: Sends SIGCONT to resume a suspended process.
term <PID>: Sends SIGINT to terminate a process.
Note: Use procs to see the list of processes and their PIDs.