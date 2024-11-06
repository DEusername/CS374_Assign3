#include "functions.h"

char *insertPID(char *commandLine, size_t *commLen)
{
    // get processID to check for "$$'s"
    pid_t PIDval = getpid();
    char PIDstr[10] = {0};
    sprintf(PIDstr, "%d", PIDval);
    size_t PIDlen = strlen(PIDstr);
    // parse the entire commandLine and check if there is any instance of "$$". If so, create a new string to hold
    for (int i = 0; i < *commLen; i++)
    {
        if (commandLine[i] == '$' && commandLine[i + 1] == '$')
        {
            // point commandLine to a new string that has the inserted PID for the $$
            char *newcommandLine = calloc(strlen(commandLine) + PIDlen + 1, sizeof(char));
            int newcommandLineIndex = 0;
            // copy over the old commandLine up to the first $
            for (int j = 0; j < i; j++)
                newcommandLine[newcommandLineIndex++] = commandLine[j];
            // copy the PIDstr into the new commandLine
            for (int j = 0; j < PIDlen; j++)
                newcommandLine[newcommandLineIndex++] = PIDstr[j];
            // skip the $$'s and start copying again from after it.
            for (int j = i + 2; j < *commLen; j++)
                newcommandLine[newcommandLineIndex++] = commandLine[j];

            // free initial commandLine and point it to the new commandLine.
            free(commandLine);
            commandLine = newcommandLine;
            *commLen = strlen(commandLine);
        }
    }
    return commandLine;
}

struct commLineInput *parseCommand(char *commandLine)
{
    // can use the tokens in main because parse string is on heap, and so can return a commandLineInput struct with all of the items.

    struct commLineInput *parsedCommandLine = malloc(sizeof(struct commLineInput));

    parsedCommandLine->command = NULL;
    memset(parsedCommandLine->arguments, 0, sizeof(parsedCommandLine->arguments));
    parsedCommandLine->inputFile = NULL;
    parsedCommandLine->outputFile = NULL;
    parsedCommandLine->background = false;

    // prepare to parse a copy of commandLine
    char *saveptr;
    char *token;
    char *parseString = calloc(strlen(commandLine) + 1, sizeof(char));
    strcpy(parseString, commandLine);

    // get the initial command.
    token = strtok_r(parseString, " \n", &saveptr);

    // create a new heap string to save into the struct so can free parseString
    char *command = calloc(strlen(token), sizeof(char));
    strcpy(command, token);

    parsedCommandLine->command = command;

    // parse the rest of parseString
    if (*saveptr == '\n')
    {
        // there are no arguments, and the command was the last item.
        free(parseString);
        return parsedCommandLine;
    }
    else
    {
        int storedArguments = 0;
        // then there are arguments, or perhaps there is a < or a > sign or a &
        while (token = strtok_r(NULL, " \n", &saveptr))
        {
            // check if it was & and the & was the last parameter in the command line input
            printf("%s\n", token);
            printf("%c\n", *saveptr);
            if (strcmp(token, "&") == 0 && *saveptr == 0) // already swapped out the '\n' from it, so end should be a 0
            {
                parsedCommandLine->background = true;
                free(parseString);
                return parsedCommandLine;
            }

            // check if it started with > for output file
            if (strcmp(token, ">") == 0)
            {
                token = strtok_r(NULL, " \n", &saveptr);

                // create a new heap memory item so that can free parseString before returning
                char *outFile = calloc(strlen(token), sizeof(char));
                strcpy(outFile, token);
                parsedCommandLine->outputFile = outFile;

                // return if the delimiter was \n or continue loop if not.
                if (*saveptr == '\n')
                {
                    free(parseString);
                    return parsedCommandLine;
                }
                else
                    continue;
            }

            // check if it started with < for input file
            if (strcmp(token, "<") == 0)
            {
                token = strtok_r(NULL, " \n", &saveptr);

                // create a new heap memory item so that can free parseString before returning
                char *inFile = calloc(strlen(token), sizeof(char));
                strcpy(inFile, token);
                parsedCommandLine->inputFile = inFile;

                // return if the delimiter was \n or continue loop if not.
                if (*saveptr == '\n')
                {
                    free(parseString);
                    return parsedCommandLine;
                }
                else
                    continue;
            }

            // otherwise, it is an argument
            // create a new heap memory item so that can free parseString before returning
            char *argument = calloc(strlen(token), sizeof(char));
            strcpy(argument, token);

            // append the argument into the struct's array of arguments. It will persit, because all of the struct data exists on
            //  the heap, so it can go out of scope and be returned.
            parsedCommandLine->arguments[storedArguments++] = argument;
        }
    }
    free(parseString);
    return parsedCommandLine;
}