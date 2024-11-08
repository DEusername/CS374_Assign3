#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <limits.h>
#include <signal.h>

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "functions.h"

void handle_SIGTSTP(int signo)
{
    // printf("Shell process ID: %d\n", getpid());
    // char *message = "Caught SIGTSTP, sleeping for 10 seconds\n";
    // // We are using write rather than printf
    // write(STDOUT_FILENO, message, 39);
    // sleep(10);
}

/**
 * @brief continuously requests user to input values 1-3 for different ways of getting a csv movie file to process. Then makes
 *      new directory and makes a new text file in the directory for every year a movie's come out in, and then loads that file
 *      with the titles of every movie that came out in that year.
 * @return 0 on success, or non-0 on a failure
 */
int main(void)
{
    // Initialize SIGINT_action struct to ignore all sig int signals.
    struct sigaction SIGINT_action = {0};
    SIGINT_action.sa_handler = SIG_IGN;
    sigfillset(&SIGINT_action.sa_mask);
    sigaction(SIGINT, &SIGINT_action, NULL); // make the created sigaction SIGINT handler the active handler for SIGINT signal

    // Initialize SIGSTOP_action struct to handle route to the handle_SIGSTOP function, and block all signals if it is working.
    struct sigaction SIGTSTP_action = {0};
    SIGTSTP_action.sa_handler = handle_SIGTSTP;
    SIGTSTP_action.sa_flags = SA_RESTART;
    sigfillset(&SIGTSTP_action.sa_mask);
    sigaction(SIGTSTP, &SIGTSTP_action, NULL); // make the created sigaction SIGSTOP handler the active handler for SIGSTOP signal

    // create variable to house the exist status from the last ran program in this shell
    int exitStatus = 0;
    bool sigTerminated = false;
    bool ranProgram = false;

    // loop through the commandLine prompt process until user quits the program
    while (true)
    {
        // get initial commandLine
        write(STDOUT_FILENO, ": ", 2);
        char *commandLine = NULL;
        size_t bufferSize = 2048;
        int bytesRead = getline(&commandLine, &bufferSize, stdin);
        if (bytesRead == -1)
        {
            clearerr(stdin);
            printf("\n");
            continue;
        }

        if (commandLine[0] == '\n' || commandLine[0] == '#')
            continue;

        // get commandLine length
        size_t commLen = strlen(commandLine);

        // replace instances of $$ with the processID
        commandLine = insertPID(commandLine, &commLen);

        struct commLineInput *parsedCommandLine = parseCommand(commandLine);

        // freeing the commandLine that the user input
        free(commandLine);

        bool ranBuiltCommand = false;
        builtInCommands(parsedCommandLine, exitStatus, ranProgram, &ranBuiltCommand, sigTerminated);
        if (ranBuiltCommand)
            continue;

        // get the number of arguments in the parsedCommandLine struct
        int argNum = 0;
        for (int i = 0; i < 512; i++)
        {
            if (parsedCommandLine->arguments[i] != 0)
                argNum++;
        }

        // printf("BEFORE execCommand FUNCTION\n");
        sigTerminated = false;
        execCommand(parsedCommandLine, &exitStatus, argNum, &sigTerminated);
        ranProgram = true;
        // printf("MADE IT THROUGH THE execCommand FUNCTION SUCCESSFULLY\n");

        // printf("%s\n", parsedCommandLine->command);
        // for (int i = 0; i < 512; i++)
        // {
        //     if (parsedCommandLine->arguments[i] != 0)
        //         printf("%s\n", parsedCommandLine->arguments[i]);
        // }
        // if (parsedCommandLine->inputFile != NULL)
        //     printf("%s\n", parsedCommandLine->inputFile);
        // if (parsedCommandLine->outputFile != NULL)
        //     printf("%s\n", parsedCommandLine->outputFile);
        // printf("%d\n", parsedCommandLine->background);

        // check for any completed background processes, and then print them
        int bpExitStatus;
        int bpPID;
        while (bpPID = waitpid(-1, &bpExitStatus, WNOHANG))
        {
            // printf("bpPID = ... %d\n", bpPID);
            if (bpPID == -1)
                break;
            printf("Background process %d is done: exit value %d\n", bpPID, bpExitStatus);
        }

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
    }

    return EXIT_SUCCESS;
}