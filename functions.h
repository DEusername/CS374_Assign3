#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <limits.h>
#include <fcntl.h>

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

//  Declares the command line input struct and the variables inside of it
struct commLineInput
{
    char *command;        // a string that is the command
    char *arguments[512]; // basically an array of arguments string pointers
    char *inputFile;      // the input file to be used for program input
    char *outputFile;     // the output file to store the output of the ran program
    bool background;      // a boolean to determine if the process should be run in the background or not.
};

/**
 * @brief replaces all instances of $$ with the processID
 * @param commandLine the user input that will have all of it's $$ instances replaced.
 * @param commLen reference to the commandLine length, that ultimately get's updated on the way out of the function
 * @return a pointer to a heap string that houses the new commandLine with the PIDs inside of it.
 */
char *insertPID(char *commandLine, size_t *commLen);

/**
 * @brief breaks the given command into smaller sub parts that can then be interpreted
 * @param command the string to be broken into smaller parts and stored in a commLineInput struct
 * @return a commLineInput struct instance on the heap that is loaded with a command, and possibly an array of arguments,
 *          a inputFile name, an outputFile name, and whether or not the commandLine designated the command to be run in the
 *          background
 */
struct commLineInput *parseCommand(char *commandLine);

/**
 * @brief check to see if the given command was one of the built in commands, and then perform the command's functionality.
 * @param parsedCommandData the parsed command line data that has references to the command and the array of arguments.
 * @param exitStatus the previously stored exit status of the last ran process
 * @param ranProgram a boolean that communicates if a non-built in command has been executed.
 * @param ranBuiltProgram a reference boolean to store and return whether a program was ran or not
 */
void builtInCommands(struct commLineInput *parsedCommandData, int exitStatus, bool ranProgram, bool *ranBuiltCommand);

/**
 * @brief execute the command help within the passed commLineInput, and attach any arguments to it
 * @param parsedCommandData the struct that houses the command line information provided to the program by the user.
 * @param
 * @param
 */
void execCommand(struct commLineInput *parsedCommandData, int *exitStatus, int argNum);