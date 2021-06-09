# Shell
Shell project for CSCI3362 Operating Systems, coded in C. 
This program reads commands from the user, splitting the inputted string into sub-commands separated by the `|` character. The program creates child threads via the `fork()` system call, then those children execute their assigned commands via the `execvp()` system call, redirecting their inputs and outputs through pipes or created files as specified by the user.
# Piping and Redirects
The user can use the `<` operator to redirect the standard input of the first sub-command. This will only work if the operator is included in the last sub-command and if the sub-command supports input redirection, as shown in the example below:
```
/home/user/shell$ wc < main.c
202 744 7125
```
Similarly, the user can use the `>` operator to redirect the standard output of the last sub-command, as shown in the example below:
```
/home/user/shell$ ls -l > ls.txt
/home/user/shell$ cat ls.txt
total 44
-rwxrwxrwx 1 user user   110 May 10 16:12 Makefile
-rwxrwxrwx 1 user user  3390 May 10 17:51 command.c
-rwxrwxrwx 1 user user   493 May  8 17:08 command.h
-rwxrwxrwx 1 user user     0 May 14 10:20 hello.txt
-rwxrwxrwx 1 user user     0 Jun  9 19:22 ls.txt
-rwxrwxrwx 1 user user  7125 Jun  8 22:58 main.c
-rwxrwxrwx 1 user user 29416 Jun  8 22:58 shell
/home/user/shell$
```
The user can also launch a command in the background using the `&` character as a suffix to the last sub-command. The shell will print the *pid* of the last process launched in the command, corresponding to the last sub-command. The shell then accepts new commands, and upon pressing the `Enter` key, the program will check to see if the background process finished, and will inform the user as such before performing the specified command. Example:
```
/home/user/shell$ sleep 30 &
[114]
/home/user/shell$
[114] finished
/home/user/shell$
```
# Exiting the Shell:
**Important:** to kill the program, enter "exit" or "quit" as a command. **Do not** use Ctrl+C&mdash;any zombie processes not yet killed by the program will lose their parent and will waste CPU quantum until the user's computer is reset. 
