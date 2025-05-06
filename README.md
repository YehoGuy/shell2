# My Shell Project

A **C-based shell** implementation demonstrating process management, inter-process communictation using pipelining, memory management, and command history.

## Table of Contents
1. [Overview](#overview)
2. [Features](#features)
3. [Project Structure](#project-structure)
4. [Installation & Compilation](#installation--compilation)
5. [Usage](#usage)
   - [Basic Commands](#basic-commands)
   - [Built-in Commands](#built-in-commands)
   - [History Feature](#history-feature)
   - [Process Management](#process-management)
---

## Overview

My Shell demonstrates low-level Unix concepts including:

- **Process creation and management** via `fork()` and `execvp()`.
- **Pipelining** (`cmd1 | cmd2`) using UNIX pipes (`pipe()`).
- **Memory management** (linked lists for process tracking & command history).
- **Signal handling** to stop, resume, or terminate child processes.
- **Command history** limited to the last 10 commands.

This shell includes an optional **debug mode** (invoked with `-d`) which logs child process creation and other internal details.

---

## Features

- **Execute Commands**: Runs standard commands like `ls`, `cat`, or `echo`.
- **Pipelining**: Supports `cmd1 | cmd2`.
- **Redirection**: Input & output redirection with `<` and `>`.
- **History**:
  - `history` command to list up to 10 recent commands.
  - `!!` to repeat the last command.
  - `!n` to re-run the n-th command from the history.
- **Process List**: Tracks child processes with a linked list, storing PID and status.
- **Signal Handling**: Built-in commands to send signals to processes (`stop`, `wake`, `term`).

---

## Project Structure

```plaintext
.
├── myshell.c         # Main shell logic in C
├── LineParser.h      # Provided header for parsing commands
├── LineParser.c      # LineParser - provided by the professor
├── README.md         # This README
└── Makefile          # (Optional) For building
## Installation & Compilation

To compile and start the shell, run the following commands in the project directory:

```sh
make clean
make
./myshell
```

## Usage

### Basic Commands

You can run standard Unix commands like `ls`, `cat`, `echo`, etc., directly in the shell.

```sh
myshell> ls
myshell> echo "Hello, World!"
```

### Built-in Commands

The shell includes several built-in commands:

- `cd <directory>`: Change the current directory.
- `quit`: Exit the shell.
- `history`: Display the last 10 commands entered.

### History Feature

The shell maintains a history of the last 10 commands. You can use:

- `history`: List the last 10 commands.
- `!!`: Repeat the last command.
- `!n`: Re-run the n-th command from the history.

### Process Management

The shell supports process management commands:

- `stop <pid>`: Stop a process with the given PID.
- `wake <pid>`: Resume a stopped process with the given PID.
- `term <pid>`: Terminate a process with the given PID.

### Pipelining

You can use the pipe (`|`) to pass the output of one command as input to another:

```sh
myshell> ls -l | tail -n 3 // will print info of the last 3 files by alphanumeric order in this folder
```

### Redirection

The shell supports input and output redirection:

- `<`: Redirect input from a file.
- `>`: Redirect output to a file.

```sh
myshell> cat < input.txt
myshell> echo "Hello" > output.txt
```
