# My Shell Project

A simple **C-based shell** implementation demonstrating process management, pipelining, memory management, and command history.

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
6. [Examples](#examples)
7. [License](#license)

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
