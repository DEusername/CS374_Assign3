#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <limits.h>

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

//  Declares the command line input struct and the variables inside of it
struct commLineInput
{
    char *command;    // a string that is the command
    char *arguments;  // basically an array of arguments
    char *inputFile;  // the input file to be used for program input
    char *outputFile; // the output file to store the output of the ran program
    bool background;  // a boolean to determine if the process should be run in the background or not.
};

/**
 * @brief replaces all instances of $$ with the processID
 * @param commandLine the user input that will have all of it's $$ instances replaced.
 * @param commLen reference to the commandLine length, that ultimately get's updated on the way out of the function
 * @return a pointer to a heap string that houses the new commandLine with the PIDs inside of it.
 */
char *insertPID(char *commandLine, size_t *commLen);

/**
 * @brief breaks the command into smaller sub parts that can then be interpreted.
 */
struct commLineInput *parseCommand(char *command);