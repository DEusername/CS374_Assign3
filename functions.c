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

void builtInCommands(struct commLineInput *parsedCommandData, int exitStatus, bool ranProgram, bool *ranBuiltCommand)
{
    // exit functionality
    if (strcmp(parsedCommandData->command, "exit") == 0)
    {
        *ranBuiltCommand = true;
        // TODO: terminate any running programs

        // terminate self
        // freeing all of the parsedCommandData fields and the parsedCommandData item itself
        free(parsedCommandData->command);
        for (int i = 0; i < 512; i++)
            free(parsedCommandData->arguments[i]);
        if (parsedCommandData->inputFile != NULL)
            free(parsedCommandData->inputFile);
        if (parsedCommandData->outputFile != NULL)
            free(parsedCommandData->outputFile);
        // don't need to free the background bool because it is just stored data in the parsedCommandData struct on the heap
        free(parsedCommandData);

        exit(EXIT_SUCCESS);
    }

    // cd functionality
    if (strcmp(parsedCommandData->command, "cd") == 0)
    {
        *ranBuiltCommand = true;
        char *path;
        if (parsedCommandData->arguments[0] != 0) // if an argument was passed with command
            path = parsedCommandData->arguments[0];
        else // if no argument was passed, get the home directory absolute path
        {
            path = getenv("HOME");
            if (!path)
                printf("FAILED TO GET HOME PATH\n");
        }
        // printf("PATH STRING: %s\n", path);
        int ret = chdir(path); // works with both absolute and relative paths.
        if (ret)
            printf("FAILED dir change\n");
        // char currDir[100];
        // char *currDirPtr = currDir;
        // char *cwdPtr = getcwd(currDirPtr, 100);
        // printf("CURRENT WORKING DIRECTORY: %s\n", cwdPtr);
    }

    // status functionality
    if (strcmp(parsedCommandData->command, "status") == 0)
    {
        *ranBuiltCommand = true;
        if (!ranProgram)
            printf("exit value %d\n", 0);
        else
            printf("exit value %d\n", exitStatus);
    }

    return;
}

void execCommand(struct commLineInput *parsedCommandData, int *exitStatus, int argNum)
{
    // fork process
    pid_t childPID = fork();

    int statusCode;
    switch (childPID)
    {
    case -1: // error
        printf("ERROR IN FORKING PROCESS\n");
        break;
    case 0: // if child, exec using the parsedCommandData
    {
        // check if need to open input file and make stdin point to that input file
        int inFD;
        if (parsedCommandData->inputFile != NULL)
        {
            inFD = open(parsedCommandData->inputFile, O_RDONLY, 0777);
            if (inFD < 0)
            {
                printf("Failed to open the input file\n");
                exit(1);
            }

            int dupRet = dup2(inFD, STDIN_FILENO);
            if (dupRet < 0)
                printf("failed to duplicate the fd into STDIN\n");
        }
        else
        {
            // deal with scenario where background is true and input file in not given
            if (parsedCommandData->background == true)
            {
                inFD = open("/dev/null", O_WRONLY | O_CREAT | O_TRUNC, 0777);
                if (inFD < 0)
                {
                    printf("Failed to open the output file\n");
                    exit(1);
                }

                int dupRet = dup2(inFD, STDOUT_FILENO);
                if (dupRet < 0)
                    printf("failed to duplicate the fd into STDOUT\n");
            }
        }

        // check if need to open output file and make stdout point to that output file
        int outFD;
        if (parsedCommandData->outputFile != NULL)
        {
            outFD = open(parsedCommandData->outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0777);
            if (outFD < 0)
            {
                printf("Failed to open the output file\n");
                exit(1);
            }

            int dupRet = dup2(outFD, STDOUT_FILENO);
            if (dupRet < 0)
                printf("failed to duplicate the fd into STDOUT\n");
        }
        else
        {
            // deal with scenario where background is true and output file in not given
            if (parsedCommandData->background == true)
            {
                outFD = open("/dev/null", O_WRONLY | O_CREAT | O_TRUNC, 0777);
                if (outFD < 0)
                {
                    printf("Failed to open the output file\n");
                    exit(1);
                }

                int dupRet = dup2(outFD, STDOUT_FILENO);
                if (dupRet < 0)
                    printf("failed to duplicate the fd into STDOUT\n");
            }
        }

        // load up an argv to give to the execution
        char *passArgV[argNum + 2];
        passArgV[0] = parsedCommandData->command;
        for (int i = 0; i < argNum; i++)
            passArgV[i + 1] = parsedCommandData->arguments[i];
        passArgV[argNum + 1] = NULL;
        int ret = execvp(parsedCommandData->command, passArgV);
        if (ret)
        {
            printf("Error: your command was not found in the PATH\n");
            exit(1);
        }
    }
    break;
    default: // if parent, wait for the child to finish before returning.
        if (parsedCommandData->background == true)
        {
            // create the formatted string for writing to console
            char childPIDstr[50] = {0};
            char *childPIDstrPtr = childPIDstr;
            sprintf(childPIDstr, "The background process' PID is %d\n", childPID);
            write(1, childPIDstrPtr, 50);
            waitpid(childPID, &statusCode, WNOHANG);
        }

        else
        {
            waitpid(childPID, &statusCode, 0);
            if (WIFEXITED(statusCode)) // enter if exited normally
                *exitStatus = WEXITSTATUS(statusCode);
        }

        break;
    }
    return;
}