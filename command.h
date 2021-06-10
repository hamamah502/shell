#ifndef COMMAND_H
#define COMMAND_H

#define MAX_SUB_COMMANDS 5
#define MAX_ARGS 10

struct SubCommand
{
	char *line; // Sub-command stored as string
	char *argv[MAX_ARGS]; // Sub-command broken into strings.
};

struct Command
{
	struct SubCommand sub_commands[MAX_SUB_COMMANDS];
	int num_sub_commands;
	char *stdin_redirect; // Initially NULL; indicates redirection of standard input
	char *stdout_redirect; // Initially NULL; indicates redirection of standard output
	int background; // Binary field; indicates whether or not command should run in background.
};

void ReadCommand(char *line, struct Command *command);
void ReadRedirectsAndBackground(struct Command *command);
void PrintCommand(struct Command *command); // Test function---not used in actual shell.

#endif
