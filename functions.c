#include "functions.h"

// function documentation is in the functions.h header file

char *insertPID(char *commandLine, size_t *commLen)
{
    // get processID to check for "$$'s"
    pid_t PIDval = getpid();
    char PIDstr[10] = {0};
    sprintf(PIDstr, "%d", PIDval);
    size_t PIDlen = strlen(PIDstr);
    // parse the entire commandLine and check if there is any instance of "$$".
    for (int i = 0; i < *commLen; i++)
    {
        if (commandLine[i] == '$' && commandLine[i + 1] == '$')
        {
            // create a new command line to hold the new commandLine information with the PID inserted
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
    // create initial struct to hold the fields of the input command line information
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
    //      *do this because tokens are all just pointers to parts of parseString
    char *command = calloc(strlen(token) + 1, sizeof(char));
    strcpy(command, token);

    parsedCommandLine->command = command;

    // parse the rest of parseString
    int storedArguments = 0;
    while (token = strtok_r(NULL, " \n", &saveptr))
    {
        // check if it was & and the & was the last parameter in the command line input
        if (strcmp(token, "&") == 0 && *saveptr == 0) // bc saveptr points just after delimiter position, it should be 0 if it's at end
        {
            char *foregroundName = "FOREMODE";
            char *foregroundMode = getenv(foregroundName);
            if (strcmp(foregroundMode, "0") == 0)
                parsedCommandLine->background = true;
            free(parseString);
            return parsedCommandLine;
        }

        // check if it started with > for output file
        if (strcmp(token, ">") == 0)
        {
            token = strtok_r(NULL, " \n", &saveptr);

            // create a new heap memory item so that can free parseString before returning
            char *outFile = calloc(strlen(token) + 1, sizeof(char));
            strcpy(outFile, token);
            parsedCommandLine->outputFile = outFile;

            // return if the delimiter was \n or continue loop if not.
            if (*saveptr == 0)
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
            char *inFile = calloc(strlen(token) + 1, sizeof(char));
            strcpy(inFile, token);
            parsedCommandLine->inputFile = inFile;

            // return if the delimiter was \n or continue loop if not.
            if (*saveptr == 0)
            {
                free(parseString);
                return parsedCommandLine;
            }
            else
                continue;
        }

        // otherwise, it is an argument
        // create a new heap memory item so that can free parseString before returning
        char *argument = calloc(strlen(token) + 1, sizeof(char));
        strcpy(argument, token);

        // append the argument into the struct's array of arguments. It will persit, because all of the struct data exists on
        //  the heap, so it can go out of scope and be returned.
        parsedCommandLine->arguments[storedArguments++] = argument;
    }
    free(parseString);
    return parsedCommandLine;
}

void builtInCommands(struct commLineInput *parsedCommandData, int exitStatus, bool ranProgram, bool *ranBuiltCommand, bool sigTerminated)
{
    // exit functionality
    if (strcmp(parsedCommandData->command, "exit") == 0)
    {
        *ranBuiltCommand = true;
        // terminate any running child processes in the same process family as the parent process
        int killRet = kill(0, SIGTERM);
        if (!killRet)
            printf("There was an error with killing all child processes\n");

        // terminate self
        // freeing all of the parsedCommandData fields and the parsedCommandData item itself
        free(parsedCommandData->command);
        for (int i = 0; i < 512; i++)
            free(parsedCommandData->arguments[i]);
        if (parsedCommandData->inputFile != NULL)
            free(parsedCommandData->inputFile);
        if (parsedCommandData->outputFile != NULL)
            free(parsedCommandData->outputFile);
        free(parsedCommandData); // don't need to free the background bool because it is in parsedCommandData struct on the heap

        exit(EXIT_SUCCESS);
    }

    // cd functionality
    if (strcmp(parsedCommandData->command, "cd") == 0)
    {
        *ranBuiltCommand = true;
        char *path;
        if (parsedCommandData->arguments[0] != 0) // if an argument was passed with command set path to first argument
            path = parsedCommandData->arguments[0];
        else // if no argument was passed, get the home directory absolute path
        {
            path = getenv("HOME");
            if (!path)
                printf("FAILED TO GET HOME PATH\n");
        }
        int ret = chdir(path); // works with both absolute and relative paths.
        if (ret)
            printf("FAILED dir change\n");
    }

    // status functionality
    if (strcmp(parsedCommandData->command, "status") == 0)
    {
        *ranBuiltCommand = true;
        if (!ranProgram) // enter if have not ran a process before
            printf("exit value %d\n", 0);
        else // if have ran a process, then print either the termination message or the exit message
        {
            if (sigTerminated)
                printf("Terminated by signal %d\n", exitStatus);
            else
                printf("exit value %d\n", exitStatus);
        }
    }
    return;
}

void execCommand(struct commLineInput *parsedCommandData, int *exitStatus, int argNum, bool *sigTerminated)
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
        // set up a new sigtstp action to ignore sigtstp. Because it is SIG_IGN, it carries over to the exec() call later
        struct sigaction SIGTSTP_action = {0};
        SIGTSTP_action.sa_handler = SIG_IGN;
        sigfillset(&SIGTSTP_action.sa_mask);
        sigaction(SIGTSTP, &SIGTSTP_action, NULL);

        // reset the SIG INT handler to the default if not a background process
        if (parsedCommandData->background == false)
        {
            struct sigaction SIGINT_action = {0};
            SIGINT_action.sa_handler = SIG_DFL;
            sigfillset(&SIGINT_action.sa_mask);
            sigaction(SIGINT, &SIGINT_action, NULL);
        }

        // check if need to open input file
        int inFD;
        if (parsedCommandData->inputFile != NULL)
        {
            // open input file
            inFD = open(parsedCommandData->inputFile, O_RDONLY, 0777);
            if (inFD < 0)
            {
                printf("Failed to open %s for input\n", parsedCommandData->inputFile);
                exit(1);
            }

            // make stdin point to that input file
            int dupRet = dup2(inFD, STDIN_FILENO);
            if (dupRet < 0)
                printf("failed to duplicate the fd into STDIN\n");
        }
        else // deal with scenario where background is true and input file in not given
        {
            if (parsedCommandData->background == true)
            {
                // make the input open /dev/null
                inFD = open("/dev/null", O_RDONLY, 0777);
                if (inFD < 0)
                {
                    printf("Failed to open the output file\n");
                    exit(1);
                }

                // make stdin take input from /dev/null
                int dupRet = dup2(inFD, STDOUT_FILENO);
                if (dupRet < 0)
                    printf("failed to duplicate the fd into STDOUT\n");
            }
        }

        // check if need to open output file
        int outFD;
        if (parsedCommandData->outputFile != NULL)
        {
            // open output file
            outFD = open(parsedCommandData->outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0777);
            if (outFD < 0)
            {
                printf("Failed to open %s for output\n", parsedCommandData->outputFile);
                exit(1);
            }

            // make stdout point to that output file
            int dupRet = dup2(outFD, STDOUT_FILENO);
            if (dupRet < 0)
                printf("failed to duplicate the fd into STDOUT\n");
        }
        else
        {
            // deal with scenario where background is true and output file in not given
            if (parsedCommandData->background == true)
            {
                // make the output use /dev/null
                outFD = open("/dev/null", O_WRONLY | O_CREAT | O_TRUNC, 0777);
                if (outFD < 0)
                {
                    printf("Failed to open the output file\n");
                    exit(1);
                }

                // make stdout output to /dev/null
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
        int ret = execvp(passArgV[0], passArgV);
        if (ret)
        {
            printf("%s: no such file or directory\n", passArgV[0]);
            exit(1);
        }
    }
    break;
    default: // if parent, deal with child existence
        if (parsedCommandData->background == true)
        {
            // create the formatted string for writing to console
            char childPIDstr[50] = {0};
            char *childPIDstrPtr = childPIDstr;
            sprintf(childPIDstr, "The background process' PID is %d\n", childPID);
            write(1, childPIDstrPtr, 50);
            waitpid(childPID, &statusCode, WNOHANG); // move on from waiting for the child
        }
        else // wait for child to exit
        {
            while (waitpid(childPID, &statusCode, 0) == -1) // deal with the child process sending kill -SIGTSTP to parent
            {
                printf("\n");
                continue;
            }

            if (WIFEXITED(statusCode)) // enter if exited normally
                *exitStatus = WEXITSTATUS(statusCode);
            else // enter if exited via signal.
            {
                *exitStatus = WTERMSIG(statusCode);
                printf("Terminated by signal %d\n", *exitStatus);
                *sigTerminated = true;
            }
        }

        break;
    }
    return;
}