<h1 align="center">Bshell Implementation</h1>
<p align="center"><strong>Introduction to Operating Systems - Homework 1</strong>
</p>
<h2>About</h2>
This homework is focused on OS processes and the communication between them. The aim is to implement a "bshell" which can run basic terminal commands ("touch", "echo", "grep" etc.), create pipes between commands and use file redirection, like a regular shell. The difference is, this bshell has the ability to run multiple commands at once including pipes and file redirection. 

<h2>Requirements</h2>

- C/C++
- Mandatory usage of only the unnamed pipes as a IPC (Inter Process Communication)
- Parallel execution of the processes
- No zombie processes left after termination with "quit"

<h2>Key learnings</h2>

- Fork concept
- Open file tables and their behaviour with "fork", "open", "close", and "pipe" system calls.
- File redirection with "dup2" system call
- Creating a pipe between processes using open file tables

