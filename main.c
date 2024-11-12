#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <limits.h>
#include <signal.h>
#include <termios.h>

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "functions.h"

// SIGTSTP handler
void handle_SIGTSTP(int signo)
{
    // basically need to check if the signal came while getting user input (I.E. while getline was still getting input), or if it
    //      came after getline had finished and had moved the terminal cursor one line down
    char *inputName = "GETINPUT";
    char *inputMode = getenv(inputName);
    if (strcmp(inputMode, "1") == 0)
        write(1, "\n", 1);

    // determine if foreground mode was already activated or not
    char *foregroundName = "FOREMODE";
    char *foregroundMode = getenv(foregroundName);

    if (strcmp(foregroundMode, "0") == 0) // enter if foreground mode is off
    {
        write(1, "Entering Foreground-only mode (& is now ignored)", 48);
        setenv(foregroundName, "1", 1);
    }
    else // enter if foreground mode is on
    {
        write(1, "Exiting Foreground-only mode", 28);
        setenv(foregroundName, "0", 1);
    }

    // write a trailing \n
    if (strcmp(inputMode, "1") == 0)
        write(1, "\n", 1);
}

/**
 * @brief continuously displays a prompt to the user for a command to execute. It then takes the user input command and either runs
 *          one of the built in commands, or executes it directly using an exec() function call.
 * @return 0 on success, or non-0 on a failure
 */
int main(void)
{
    char *foregroundName = "FOREMODE";
    setenv(foregroundName, "0", 1);

    // Initialize SIGINT_action struct to ignore all sig int signals.
    struct sigaction SIGINT_action = {0};
    SIGINT_action.sa_handler = SIG_IGN;
    sigfillset(&SIGINT_action.sa_mask);
    sigaction(SIGINT, &SIGINT_action, NULL); // make the created sigaction SIGINT handler the active handler for SIGINT signal

    // Initialize SIGSTOP_action struct to handle route to the handle_SIGSTOP function, and block all signals if it is working.
    struct sigaction SIGTSTP_action = {0};
    SIGTSTP_action.sa_handler = handle_SIGTSTP;
    sigfillset(&SIGTSTP_action.sa_mask);
    sigaction(SIGTSTP, &SIGTSTP_action, NULL); // make the created sigaction SIGSTOP handler the active handler for SIGSTOP signal

    // create variable to house the exist status from the last ran program in this shell
    int exitStatus = 0;
    bool sigTerminated = false;
    bool ranProgram = false;

    // loop through the commandLine prompt process until user quits the program
    while (true)
    {
        // check for any completed background processes, and then print them
        int bpExitStatus;
        int bpStatusCode;
        int bpPID;
        while (bpPID = waitpid(-1, &bpStatusCode, WNOHANG))
        {
            if (bpPID == -1)
                break;
            if (WIFEXITED(bpStatusCode)) // enter if exited normally
            {
                bpExitStatus = WEXITSTATUS(bpStatusCode); // interperet the status code to get the exit status
                printf("Background process %d is done: exit value %d\n", bpPID, bpExitStatus);
            }
            else // enter if exited via signal.
            {
                bpExitStatus = WTERMSIG(bpStatusCode);
                printf("Background process %d is done: Terminated by signal %d\n", bpPID, bpExitStatus);
            }
        }

        // set up env var for if the SIGTSTP signal comes when getting user input
        char *inputName = "GETINPUT";
        setenv(inputName, "1", 1);

        // get initial commandLine
        write(STDOUT_FILENO, ": ", 2);
        char *commandLine = NULL;
        size_t bufferSize = 2048;
        int bytesRead = getline(&commandLine, &bufferSize, stdin);
        if (bytesRead == -1) // deal with a sigtstp signal by resetting the loop after handling signal
        {
            clearerr(stdin);
            continue;
        }
        // set the GETINPUT var to be 0 since getline has finished and has moved terminal output onto the next line
        setenv(inputName, "0", 1);

        // deal with comments / empty lines
        if (commandLine[0] == '\n' || commandLine[0] == '#')
            continue;

        // get commandLine length
        size_t commLen = strlen(commandLine);

        // replace instances of $$ with the processID
        commandLine = insertPID(commandLine, &commLen);

        // generate a processed commLineInput struct to store the command line data fields
        struct commLineInput *parsedCommandLine = parseCommand(commandLine);
        free(commandLine);

        // check to see if need to run any built in commands.
        bool ranBuiltCommand = false;
        builtInCommands(parsedCommandLine, exitStatus, ranProgram, &ranBuiltCommand, sigTerminated);
        if (ranBuiltCommand)
        {
            // freeing all of the parsedCommandLine fields
            free(parsedCommandLine->command);
            for (int i = 0; i < 512; i++)
                free(parsedCommandLine->arguments[i]);
            if (parsedCommandLine->inputFile != NULL)
                free(parsedCommandLine->inputFile);
            if (parsedCommandLine->outputFile != NULL)
                free(parsedCommandLine->outputFile);
            // don't need to free the background bool because it is just stored data in the parsedCommandLine struct on the heap
            free(parsedCommandLine);
            continue;
        }

        // get the number of arguments in the parsedCommandLine struct
        int argNum = 0;
        for (int i = 0; i < 512; i++)
        {
            if (parsedCommandLine->arguments[i] != 0)
                argNum++;
        }

        // run any other command using exec() from a child process
        sigTerminated = false;
        execCommand(parsedCommandLine, &exitStatus, argNum, &sigTerminated);
        ranProgram = true;

        // freeing all of the parsedCommandLine fields
        free(parsedCommandLine->command);
        for (int i = 0; i < 512; i++)
            free(parsedCommandLine->arguments[i]);
        if (parsedCommandLine->inputFile != NULL)
            free(parsedCommandLine->inputFile);
        if (parsedCommandLine->outputFile != NULL)
            free(parsedCommandLine->outputFile);
        // don't need to free the background bool because it is just stored data in the parsedCommandLine struct on the heap
        free(parsedCommandLine);

        // clear the standard input buffer, in case user wrote things while shell was waiting for a foreground process
        tcflush(STDIN_FILENO, TCIFLUSH);
    }

    return EXIT_SUCCESS;
}