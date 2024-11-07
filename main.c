#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <limits.h>

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "functions.h"

/**
 * @brief continuously requests user to input values 1-3 for different ways of getting a csv movie file to process. Then makes
 *      new directory and makes a new text file in the directory for every year a movie's come out in, and then loads that file
 *      with the titles of every movie that came out in that year.
 * @return 0 on success, or non-0 on a failure
 */
int main(void)
{
    // create variable to house the exist status from the last ran program in this shell
    int exitStatus = 0;
    bool ranProgram = false;

    // loop through the commandLine prompt process until user quits the program
    while (true)
    {
        // get initial commandLine
        write(STDOUT_FILENO, ": ", 2);
        char *commandLine = NULL;
        size_t bufferSize = 2048;
        int bytesRead;
        bytesRead = getline(&commandLine, &bufferSize, stdin);

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
        builtInCommands(parsedCommandLine, exitStatus, ranProgram, &ranBuiltCommand);
        if (ranBuiltCommand)
            continue;

        // get the number of arguments in the parsedCommandLine struct
        int argNum = 0;
        for (int i = 0; i < 512; i++)
        {
            if (parsedCommandLine->arguments[i] != 0)
                argNum++;
        }

        execCommand(parsedCommandLine, &exitStatus, argNum);
        printf("MADE IT THROUGH THE execCommand FUNCTION SUCCESSFULLY\n");

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