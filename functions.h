#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <limits.h>

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

/**
 * @brief replaces all instances of $$ with the processID
 * @param command the user input that will have all of it's $$ instances replaced.
 * @param commLen reference to the command length, that ultimately get's updated on the way out of the function
 * @return a pointer to a heap string that houses the new command with the PIDs inside of it.
 */
char *insertPID(char *command, size_t *commLen);