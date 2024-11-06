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
    // open current directory
    DIR *currDir = opendir(".");

    // loop through the commandLine prompt process until user quits the program
    while (true)
    {
        // get initial commandLine
        write(STDOUT_FILENO, ": ", 2);
        char *commandLine = NULL;
        size_t bufferSize = 2048;
        int ret;
        ret = getline(&commandLine, &bufferSize, stdin);

        // take out '\n' and get commandLine length
        size_t commLen = strlen(commandLine);
        commandLine[commLen - 1] = 0;
        commLen -= 1;

        // replace instances of $$ with the processID
        commandLine = insertPID(commandLine, &commLen);

        struct commLineInput *parsedCommandLine = parseCommand(commandLine);

        printf("%s\n", parsedCommandLine->command);
        printf("%d\n", parsedCommandLine->background);
        free(commandLine);
    }

    int closeCurr = closedir(currDir);

    return EXIT_SUCCESS;
}