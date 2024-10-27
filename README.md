# CS374_Assign3

**Description**

In the C language, create a small shell program.

This program does the following

1. Provide a prompt for running commands
2. Handle blank lines and comments, which are lines beginning with the # character
3. Provide expansion for the variable $$
4. Execute 3 commands exit, cd, and status via code built into the shell
5. Execute other commands by creating new processes using a function from the exec family of functions
6. Support input and output redirection
7. Support running commands in foreground and background processes
8. Implement custom handlers for 2 signals, SIGINT and SIGTSTP

### Additionally

use this command when trying to copy the repository to the server to test compilation: 

scp -r . eversond@os1.engr.oregonstate.edu:/nfs/stak/users/eversond/CS374/assign3_work

make sure this command is made from either within the directory, or one level above it to copy the items recursively into the server.