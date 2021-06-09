#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "command.h"

void ReadArgs(char *in, char **argv, int size) //Used to populate a subcommand
{
	int i = 0;
	char *token;
	if (size <= 0)
	{
		printf("Size passed to ReadArgs not valid.\n");
		return;
	}
	token = strtok(in, " ");
	while (i < size && token != NULL)
	{
		argv[i] = strdup(token);
		token = strtok(NULL, " ");
		i++;
	}
    if (i == size) argv[i-1] = NULL;
    else argv[i] = NULL;
}

void PrintArgs(char **argv) //Likely won't be necessary, but included anyway
{
	int i = 0;
       	while (argv[i] != NULL)
	{
		printf("argv[%d] = '%s'\n", i, argv[i]);
		i++;
	}
}

void ReadCommand(char *line, struct Command *command) //Used to populate command
{
	int i = 0;
	char *token;
	token = strtok(line, "|");
	while (i < MAX_SUB_COMMANDS && token != NULL)
	{
		command->sub_commands[i].line = strdup(token);
		token = strtok(NULL, "|");
		i++;
	}
	command->num_sub_commands = i;
	for (i = 0; i < command->num_sub_commands; i++)
	{
		ReadArgs(command->sub_commands[i].line, command->sub_commands[i].argv, MAX_ARGS);
	}
}

void ReadRedirectsAndBackground(struct Command *command) //Populates redirects & background field of command. Errors regarding output will be handled in main. 
{
	int i = 0;
    while ( i < MAX_ARGS && command->sub_commands[command->num_sub_commands - 1].argv[i] != NULL) i++;
	i--;
	command->background = 0;
	command->stdin_redirect = NULL;
	command->stdout_redirect = NULL;
	while ( i >= 0 )
	{
		char *current = command->sub_commands[command->num_sub_commands - 1].argv[i];
		if (strcmp(current, "&") == 0) // Set background field accordingly and adjust argv of last subcommand to avoid errors. 
		{
			command->background = 1;
			command->sub_commands[command->num_sub_commands - 1].argv[i] = NULL;
		}
		else if (strcmp(current, "<") == 0)
		{
			//if (i+1 >= MAX_ARGS || command->sub_commands[command->num_sub_commands - 1].argv[i+1] == NULL) printf("Input redirect fault.");
            command->stdin_redirect = command->sub_commands[command->num_sub_commands - 1].argv[i+1];
			command->sub_commands[command->num_sub_commands - 1].argv[i] = NULL;
		}
		else if (strcmp(current, ">") == 0)
		{
			if (i+1 >= MAX_ARGS) //Bounds check for output redirect. 
                printf("Output redirect fault: Maximum subcommand count exceeded.\n");
            else if (command->sub_commands[command->num_sub_commands - 1].argv[i+1] == NULL)
                printf("Output redirect fault: Invalid output redirect.\n");
			else
			{
				command->stdout_redirect = command->sub_commands[command->num_sub_commands - 1].argv[i+1];
				command->sub_commands[command->num_sub_commands - 1].argv[i] = NULL;
			}
		}
		i--;
	}
}

void PrintCommand(struct Command *command) //Likely won't be necessary, but included anyway
{
	int i;
	for (i = 0; i < command->num_sub_commands; i++)
	{
		printf("Command %d:\n", i);
		PrintArgs(command->sub_commands[i].argv);
		printf("\n");
	}
	if (command->stdin_redirect != NULL) printf("Redirect stdin: %s\n", command->stdin_redirect);
	if (command->stdout_redirect != NULL) printf("Redirect stdout: %s\n", command->stdout_redirect);
	if (command->background == 1)
	{
		printf("Background: yes\n");
	}
	else printf("Background: no\n");
}