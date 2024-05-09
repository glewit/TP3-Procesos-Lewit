#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#define MAX_COMMANDS 200
#define MAX_ARGS 128

void sacar_comillas(char *str) {
    for (int i = 0; str[i] != '\0'; i++) {

        if (str[i] == '"') {
            memmove(&str[i], &str[i + 1], strlen(str) - i);
            i--;
            
        }
    }
}

int main() {

    char command[256];
    char *commands[MAX_COMMANDS];
    int command_count;

    while (1) 
    {
        printf("Shell> ");
        
        /*Reads a line of input from the user from the standard input (stdin) and stores it in the variable command */
        fgets(command, sizeof(command), stdin);
        
        /* Removes the newline character (\n) from the end of the string stored in command, if present. 
           This is done by replacing the newline character with the null character ('\0').
           The strcspn() function returns the length of the initial segment of command that consists of 
           characters not in the string specified in the second argument ("\n" in this case). */
        command[strcspn(command, "\n")] = '\0';

        /* Tokenizes the command string using the pipe character (|) as a delimiter using the strtok() function. 
           Each resulting token is stored in the commands[] array. 
           The strtok() function breaks the command string into tokens (substrings) separated by the pipe character |. 
           In each iteration of the while loop, strtok() returns the next token found in command. 
           The tokens are stored in the commands[] array, and command_count is incremented to keep track of the number of tokens found. */
        command_count = 0;
        char *token = strtok(command, "|");
        while (token != NULL) 
        {
            commands[command_count++] = token;
            token = strtok(NULL, "|");
        }

        int pids[command_count];
        int pipes[command_count-1][2];
        
        for (int i = 0; i < command_count-1; i++)
        {
            if (pipe(pipes[i]) == -1) {
                perror("fallo creando pipes");
                exit(EXIT_FAILURE);
            }
        }

        /* You should start programming from here... */
        for (int i = 0; i < command_count; i++) 
        {      
            char *args[MAX_ARGS];
            int index = 0;
            char *token_ = strtok(commands[i], " ");

            while (token_ != NULL && index < MAX_ARGS - 1) {
                sacar_comillas(token_);
                args[index++] = token_;
                token_ = strtok(NULL, " ");
            }

            args[index] = NULL;

            // printf("Command %d arguments:\n", i);
            for (int j = 0; j < index; j++) {
                printf(" %s\n", args[j]);
            }
            printf("Command %d: %s\n", i, commands[i]);

            pids[i] = fork();
            if (pids[i] == -1){
                perror("fallo haciendo fork");
                exit(EXIT_FAILURE);
            }

            if(pids[i] == 0) {
                if (i == 0) {
                    close(pipes[i][0]);

                    dup2(pipes[i][1], STDOUT_FILENO);
                    close(pipes[i][1]);

                } else if (i != command_count-1) {
                    dup2(pipes[i-1][0], STDIN_FILENO);
                    close(pipes[i-1][0]);

                    dup2(pipes[i][1], STDOUT_FILENO);
                    close(pipes[i][1]);

                } else {
                    close(pipes[i-1][1]);

                    dup2(pipes[i-1][0], STDIN_FILENO);
                    close(pipes[i-1][0]);

                }

                for (int j = 0; j < command_count - 1; j++) {
                    if (j != i - 1) close(pipes[j][0]);
                    if (j != i) close(pipes[j][1]);
                }

                execvp(args[0], args);
                perror("Execution failed");
                exit(EXIT_FAILURE);

            }
        }

        for (int i = 0; i < command_count - 1; i++) {
            close(pipes[i][0]);
            close(pipes[i][1]);
        }

        for (int i = 0; i < command_count; i++) {
            waitpid(pids[i], NULL, 0);
        }
    }

    return 0;
}
