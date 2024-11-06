#include "functions.h"

char *insertPID(char *command, size_t *commLen)
{
    // get processID to check for "$$'s"
    pid_t PIDval = getpid();
    char PIDstr[10] = {0};
    sprintf(PIDstr, "%d", PIDval);
    size_t PIDlen = strlen(PIDstr);
    printf("%s\n", PIDstr);
    // parse the entire command and check if there is any instance of "$$". If so, create a new string to hold
    for (int i = 0; i < *commLen; i++)
    {
        if (command[i] == '$' && command[i + 1] == '$')
        {
            // point command to a new string that has the inserted PID for the $$
            char *newCommand = calloc(strlen(command) + PIDlen + 1, sizeof(char));
            int newCommandIndex = 0;
            // copy over the old command up to the first $
            for (int j = 0; j < i; j++)
                newCommand[newCommandIndex++] = command[j];
            // copy the PIDstr into the new command
            for (int j = 0; j < PIDlen; j++)
                newCommand[newCommandIndex++] = PIDstr[j];
            // skip the $$'s and start copying again from after it.
            for (int j = i + 2; j < *commLen; j++)
                newCommand[newCommandIndex++] = command[j];

            // free initial command and point it to the new command.
            printf("%s\n", command);
            printf("%s\n", newCommand);
            free(command);
            command = newCommand;
            printf("%d\n", *commLen);
            *commLen = strlen(command); // here I should be setting commLen to be equal to the newly created command.
            // it seems that maybe it isn't???
            printf("%d\n", *commLen);
        }
    }
    return command;
}
