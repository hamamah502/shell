#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "command.h"

int executecommand(struct Command * command)
{
    int fds[command->num_sub_commands - 1][2], child[command->num_sub_commands], i = 0, j = 0;
    int error;
    for (i = 0; i < (command->num_sub_commands - 1); i++)
    {
        error = pipe(fds[i]);
        if (error == -1)
        {
            perror("Error creating pipe");
            return -1;
        }
    }
    child[0] = fork();
    if (child[0] < 0)
    {
        perror("First fork failed");
        return 0;
    }
    else if (child[0] == 0)
    {
        for (j = 1; j < (command->num_sub_commands - 1); j++) // Close every pipe except for the first one. 
        {
            close(fds[j][0]);
            close(fds[j][1]);
        }
        close(fds[0][0]);
        if (command->num_sub_commands != 1) // Only time we want to keep first pipe open. 
        {
            dup2(fds[0][1], 1);
            //close(fds[0][1]);
            //close(1);
            //dup(fds[i]);
        }
        else if (command->stdout_redirect) // If this is the only subcommand, check for output redirect. Open file for redirection. 
        {
            close(fds[0][1]);
            FILE *fout;
            fout = freopen(command->stdout_redirect, "w+", stdout);
            if (!fout)
                perror("Failed output redirect in only subcommand");
        }
        else
            close(fds[0][1]);
        if (command->stdin_redirect) // Always check for input redirect, then open & redirect input if file exists. 
        {
            FILE *fin;
            fin = freopen(command->stdin_redirect, "r", stdin);
            if (!fin)
                perror("Failed input redirect in first subcommand");
        }
        execvp(command->sub_commands[0].argv[0], command->sub_commands[0].argv);
        perror("First subcommand failed");
        exit(0); 
    }
    for (i = 1; i < (command->num_sub_commands - 1); i++) // Fork for all subcommands except first & last; should skip if num_subcommands == 1
    {
        child[i] = fork();
        if (child[i] < 0)
        {
            printf("Fork %d failed", i);
            return 0;
        }
        else if (child[i] == 0)
        {
            for (j = 0; j < (i - 1); j++)
            {
                close(fds[j][0]); // Close all pipes except pipe i-1 (read), i (write)
                close(fds[j][1]);
            }
            for (j = (i + 1); j < (command->num_sub_commands - 1); j++)
            {
                close(fds[j][0]); 
                close(fds[j][1]);
            }
            close(fds[i - 1][1]);
            close(fds[i][0]);
            dup2(fds[i - 1][0], 0); // Read from pipe i - 1
            //close(fds[i - 1][0]); // Close to avoid undesired blocking
            dup2(fds[i][1], 1); // Write to pipe i
            //close(fds[i][0]); // Close to avoid undesired blocking
            /*close(0);
            dup(fds[i - 1]);
            close(1);
            dup(fds[i]);*/
            execvp(command->sub_commands[i].argv[0], command->sub_commands[i].argv);
            printf("Subcommand %d failed", i); // Again, should never reach here but error check just in case
            perror;
            exit(0); // If exec() fails, we need to free the allocated memory. 
        }
    }
    if (command->num_sub_commands == 1) // Following section is for the final subcommand, so don't go there if there's only one
        return child[0];
    child[i] = fork();
    if (child[i] < 0)
    {
        perror("Final fork failed");
        return 0;
    }
    else if (child[i] == 0)
    {
        for (j = 0; j < (command->num_sub_commands - 2); j++)
        {
            close(fds[j][0]);
            close(fds[j][1]);
        }
        close(fds[i - 1][1]);
        dup2(fds[i - 1][0], 0); // Read from pipe
        //close(fds[i - 1][0]);
        /*close(0);
        dup(fds[i-1]);*/
        if (command->stdout_redirect) // Check for output redirect. Open file for redirection. 
        {
            FILE *fout;
            fout = freopen(command->stdout_redirect, "w+", stdout);
            if (!fout)
                perror("Failed output redirect in only subcommand");
        }
        execvp(command->sub_commands[i].argv[0], command->sub_commands[i].argv);
        perror("Final subcommand failed"); //Should never reach here, but error checking just in case
        exit(0); // If exec() fails, we need to free the allocated memory. 
    }
    for (j = 0; j < (command->num_sub_commands - 1); j++) // Close every pipe because we don't need them. 
    {
        close(fds[j][0]);
        close(fds[j][1]);
    }
    return child[i];
}
int checkexit(char *s)
{
    if (!strcasecmp(s, "exit") || !strcasecmp(s, "quit"))
    {
        printf("Thank you so much for using my shell!\n");
        exit(0);
    }
}
int main()
{
    struct Command *command;
    int lastpid = 0;
    char s[200], cwd[1024];
    while (1)
    {
        command = (struct Command *) calloc(1, sizeof (struct Command));
        getcwd(cwd, sizeof(cwd));   
        // Get command
        printf("%s$ ", cwd);
        fgets(s, sizeof(s), stdin);
        if (s[0] == '\n')
            continue;
        s[strlen(s) - 1] = '\0';
        // Populate command & redirects
        ReadCommand(s, command);
        // Check exit cond:
        checkexit(command->sub_commands[0].line);
        // Check to see if previous process finished executing:
        if (waitpid(lastpid, NULL, WNOHANG) != 0)
        {
            printf("[%d] finished\n", lastpid);
            lastpid = 0;
        }
        ReadRedirectsAndBackground(command);
        // Check for change directory command:
        if (!strcmp(command->sub_commands[0].argv[0], "cd"))
        {
            if (command->num_sub_commands > 1)
            {
                command->sub_commands[0].line = NULL;
                command->sub_commands[0].argv[0] = NULL;
            }
            else 
            {
                int cderror = chdir(strtok(command->sub_commands[0].argv[1], " "));
                if (cderror < 0)
                {
                    perror("Command cd failed");
                    continue;
                }
                continue;
            }
        }
        // Execute command, fetch pid of last subcommand
        lastpid = executecommand(command);
        if (command->background) // If we want to run this in the background, print the pid of the last subcommand (which was returned by executecommand)
            printf("[%d]\n", lastpid);
        else // Wait for the final process to finish before accepting the next command. 
        {
            waitpid(lastpid, NULL, 0);
        }
        free(command);
    }
    return 0; // Should never reach here, only ought to exit via conditions in "checkexit" function. 
}