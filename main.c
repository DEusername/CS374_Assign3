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

    // loop through the command prompt process until user quits the program
    while (true)
    {
        // get initial command
        write(STDOUT_FILENO, ": ", 2);
        char *command = NULL;
        size_t bufferSize = 2048;
        int ret;
        ret = getline(&command, &bufferSize, stdin);

        // take out '\n' and get command length
        size_t commLen = strlen(command);
        command[commLen - 1] = '0';
        commLen -= 1;

        // get processID to check for "$$'s"
        pid_t PIDval = getpid();
        char PIDstr[10] = {0};
        sprintf(PIDstr, "%d", PIDval);
        size_t PIDlen = strlen(PIDstr);
        // parse the entire command and check if there is any instance of "$$". If so, create a new string to hold
        for (int i = 0; i < commLen; i++)
        {
            if (command[i] == '$' && command[i + 1] == '$')
            {
                // point command to a new string that has the inserted PID for the $$
                char *newCommand = calloc(strlen(command) + PIDlen, sizeof(char));
                int newCommandIndex = 0;
                for (int j = 0; j < i; j++)
                {
                    newCommand[newCommandIndex] = command[j];
                    newCommandIndex++;
                }
                for (int j = 0; j < PIDlen; j++)
                {
                    newCommand[newCommandIndex] = PIDstr[j];
                    newCommandIndex++;
                }
                for (int j = i + 2; j < commLen; j++)
                {
                    newCommand[newCommandIndex] = command[j];
                    newCommandIndex++;
                }

                // free initial command and give it the new command.
                free(command);
                command = newCommand;
                commLen = strlen(command);
            }
        }
        write(STDOUT_FILENO, command, commLen);
        free(command);
    }

    int closeCurr = closedir(currDir);

    return EXIT_SUCCESS;
}