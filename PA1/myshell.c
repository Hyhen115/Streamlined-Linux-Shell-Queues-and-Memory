/*
    COMP3511 Fall 2024
    PA1: Simplified Linux Shell (MyShell)

    Your name: Wong Hon Yin
    Your ITSC email: hywongdc@connect.ust.hk

    Declaration:

    I declare that I am not involved in plagiarism
    I understand that both parties (i.e., students providing the codes and students copying the codes) will receive 0 marks.

*/

/*
    Header files for MyShell
    Necessary header files are included.
    Do not include extra header files
*/
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h> // For constants that are required in open/read/write/close syscalls
#include <sys/wait.h> // For wait() - suppress warning messages
#include <fcntl.h>    // For open/read/write/close syscalls
#include <signal.h>   // For signal handling

// Define template strings so that they can be easily used in printf
//
// Usage: assume pid is the process ID
//
//  printf(TEMPLATE_MYSHELL_START, pid);
//
#define TEMPLATE_MYSHELL_START "Myshell (pid=%d) starts\n"
#define TEMPLATE_MYSHELL_END "Myshell (pid=%d) ends\n"
#define TEMPLATE_MYSHELL_TERMINATE "Myshell (pid=%d) terminates by Ctrl-C\n"

// Assume that each command line has at most 256 characters (including NULL)
#define MAX_CMDLINE_LENGTH 256

// Assume that we have at most 8 arguments
#define MAX_ARGUMENTS 8

// Assume that we only need to support 2 types of space characters:
// " " (space) and "\t" (tab)
#define SPACE_CHARS " \t"

// The pipe character
#define PIPE_CHAR "|"

// Assume that we only have at most 8 pipe segements,
// and each segment has at most 256 characters
#define MAX_PIPE_SEGMENTS 8

// Assume that we have at most 8 arguments for each segment
// We also need to add an extra NULL item to be used in execvp
// Thus: 8 + 1 = 9
//
// Example:
//   echo a1 a2 a3 a4 a5 a6 a7
//
// execvp system call needs to store an extra NULL to represent the end of the parameter list
//
//   char *arguments[MAX_ARGUMENTS_PER_SEGMENT];
//
//   strings stored in the array: echo a1 a2 a3 a4 a5 a6 a7 NULL
//
#define MAX_ARGUMENTS_PER_SEGMENT 9

// Define the standard file descriptor IDs here
#define STDIN_FILENO 0  // Standard input
#define STDOUT_FILENO 1 // Standard output

// This function will be invoked by main()
// This function is given
int get_cmd_line(char *command_line)
{
    int i, n;
    if (!fgets(command_line, MAX_CMDLINE_LENGTH, stdin))
        return -1;
    // Ignore the newline character
    n = strlen(command_line);
    command_line[--n] = '\0';
    i = 0;
    while (i < n && command_line[i] == ' ')
    {
        ++i;
    }
    if (i == n)
    {
        // Empty command
        return -1;
    }
    return 0;
}

// read_tokens function is given
// This function helps you parse the command line
//
// Suppose the following variables are defined:
//
// char *pipe_segments[MAX_PIPE_SEGMENTS]; // character array buffer to store the pipe segements
// int num_pipe_segments; // an output integer to store the number of pipe segment parsed by this function
// char command_line[MAX_CMDLINE_LENGTH]; // The input command line
//
// Sample usage:
//
//  read_tokens(pipe_segments, command_line, &num_pipe_segments, "|");
//
void read_tokens(char **argv, char *line, int *numTokens, char *delimiter)
{
    int argc = 0;
    char *token = strtok(line, delimiter);
    while (token != NULL)
    {
        argv[argc++] = token;
        token = strtok(NULL, delimiter);
    }
    *numTokens = argc;
}

void process_cmd(char *command_line)
{
    // Uncomment this line to check the cmdline content
    // Please remember to remove this line before the submission
    // printf("Debug: The command line is [%s]\n", command_line);

    //TODO feature 3: File Redirection
    //Phrase the command line
    char *pipeSegments[MAX_PIPE_SEGMENTS]; // character array buffer to store the pipe segements
    char *arguments[MAX_ARGUMENTS_PER_SEGMENT]; // character array buffer to store the arguments
    int argumentsCount;
    int pipeSegmentsCount;
    int redirectionsCount;

    //phrase the command line

    //split the command line by pipe
    read_tokens(pipeSegments, command_line, &pipeSegmentsCount, PIPE_CHAR);
    pipeSegments[pipeSegmentsCount] = NULL;

    //for each pipe segment split with space
    for(int i = 0; i < pipeSegmentsCount; ++i)
    {
        //split the pipe segment with space -> get the arguments
        read_tokens(arguments, pipeSegments[i], &argumentsCount, SPACE_CHARS);
        arguments[argumentsCount] = NULL;

        //search for the redirections
        for(int j = 0; j < argumentsCount; ++j)
        {
            //input redirection
            if(strcmp(arguments[j], "<") == 0)
            {
                //read the file
                int fdInput = open(arguments[j + 1], O_RDONLY);

                //check successfull or not
                if (fdInput == -1)
                {
                    printf("Error: Cannot open the file %s\n", arguments[j + 1]);
                    exit(1);
                }

                //redirect the file to the standard input
                dup2(fdInput, STDIN_FILENO);
                close(fdInput);
                //! no fin yet ==================

                //remove '<'
                arguments[j] = NULL;
            }
            
            //output redirection
            else if(strcmp(arguments[j], ">") == 0)
            {
                //write the file
                int fdOutput = open(arguments[j + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                
                //check successfull or not
                if(fdOutput == -1)
                {
                    printf("Error: Cannot open the file %s\n", arguments[j + 1]);
                    exit(1);
                }

                //redirect the file to the standard output
                dup2(fdOutput, STDOUT_FILENO);
                close(fdOutput);
                //! no fin yet ==================

                //remove '>'
                arguments[j] = NULL;

            }
        }
        
        //execute the command
        //since (</>) has been become null
        //argv argv ... NULL... file names ... NULL... filenames
        //the execvp only detect the argvs and end before the file names
        execvp(arguments[0], arguments);
        
        //execvp fails
        printf("execvp fails\n");
        exit(1);
    }



}

// TODO feature 2: Ctrl-C (SIGINT) handling
void sigint_handler(int sig)
{
    // print the terminate message
    printf(TEMPLATE_MYSHELL_TERMINATE, getpid());
    //terminate the program
    exit(0);
}

/* The main function implementation */
int main()
{
    // TODO: replace the shell prompt with your ITSC account name
    // For example, if you ITSC account is cspeter@connect.ust.hk
    // You should replace ITSC with cspeter
    char *prompt = "hywongdc";
    char command_line[MAX_CMDLINE_LENGTH];

    // TODO:
    // The main function needs to be modified
    // For example, you need to handle the exit command inside the main function

    printf(TEMPLATE_MYSHELL_START, getpid());

    // The main event loop
    while (1)
    {
        // TODO feature 2: Ctrl-C (SIGINT) handling
        signal(SIGINT, sigint_handler);

        printf("%s> ", prompt);
        if (get_cmd_line(command_line) == -1)
            continue; /* empty line handling */

        // TODO feature 1: exit command
        //  check if the command is exit
        if (strcmp(command_line, "exit") == 0)
        {
            // Exit the loop (exit the program)
            printf(TEMPLATE_MYSHELL_END, getpid());
            return 0; // exit the program
        }

        pid_t pid = fork();
        if (pid == 0)
        {
            // the child process handles the command
            process_cmd(command_line);

            // ensure the child process terminates
            exit(0);
        }
        else
        {
            // the parent process simply wait for the child and do nothing
            wait(0);
        }
    }

    return 0;
}