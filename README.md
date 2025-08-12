# CS374_Assign3

## Description

A small, limited shell program created in the C language as a class project.

This program does the following

1. Provide a prompt for running commands
2. Handle blank lines and comments, which are lines beginning with the # character
3. Provide expansion for the variable $$
4. Execute 3 commands exit, cd, and status via code built into the shell
5. Execute other commands by creating new processes using a function from the exec family of functions
6. Support input and output redirection
7. Support running commands in foreground and background processes
8. Implement custom handlers for 2 signals, SIGINT and SIGTSTP

## File Information

### README.md

Provides an overview of what the program does when ran, as well as an overview of what each file does.

### USER.md

Contains the instructions for compiling and running this program using gcc

### LICENCE.md

MIT licence declaring how people may use this code.

### functions.h Entails

A header file for all of the functions implemented in functions.c. This file contains all of the function prototypes, as well as the function documentation. Also contains the command line struct which holds user input into the shell.

### functions.c Entails:

The code for all of the functions used to complete the above 8 tasks, used in both the main file and in related functions (still inside of functions.c file).

### main.c Entails:

This file contains all of the main function code to complete al of the 8 program objectives above. This file contains the signal handling code, while the main function revolves around implementing while loops to continuously get commands from the user.

### p3testscript

The file contains a bash script to output the results of attempting to complete the above 8 objectives for the program. 
