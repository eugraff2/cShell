#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>

#define BUFFSIZE 1024

int main(int argc, char * argv[]) {

    char * homeDir = getenv("HOME");
    chdir(homeDir); // start off in the user's home directory

    while(1) {

        // printing out the current working directory and replacing HOME with ~
        char cwd[BUFFSIZE];
        getcwd(cwd, BUFFSIZE);
        char * currentDir = strstr(cwd, homeDir);
        int length = strlen(homeDir);
        if (strcmp(currentDir, cwd) == 0) {
            printf("1730sh:~%s$ ", currentDir + length);
        } else {
            printf("1730sh:%s$ ", cwd);
        } // if
        fflush(stdout);

        char commandInput[BUFFSIZE];
        int n = read(0, commandInput, BUFFSIZE); // reading command input
        if (n == -1) perror("read");
        char command[n+1];
        for (int i = 0; i < n; i++) {
            command[i] = commandInput[i]; // storing command input in an "n" sized array
        } // for i
        command[n] = 0; // adding terminating character for c string


        char * argv[n]; // the array of arguments with the original command at index 0
        char * delim = " \n";
        char * token = strtok(command, delim);
        argv[0] = token;
        int j = 1;
        while (token != NULL) {
            token = strtok(NULL, delim);
            argv[j] = token;
            j++;
        } // while parsing command arguments

        char * cmd = argv[0];

        if (strcmp(cmd, "exit") == 0) {
            exit(EXIT_FAILURE);
        } else if (strcmp(cmd, "cd") == 0) {
            int ch = chdir(argv[1]);
            if (ch == -1) perror("cd");
        } else { // any other command


//*** fork code ******************************************

            pid_t pid;
            if ((pid = fork()) < 0) perror("fork");
            else if (pid == 0) { // child process

                for (int k = 0; k < (j-1); k++) {
                    char * input = argv[k+1];
                    if (strcmp(argv[k], "<") == 0) { // redirect in

                        argv[k] = NULL; // to avoid execvp reading < as an argument
                        strcpy(input, argv[k+1]);

                        int fdi = open(input, O_RDONLY);
                        if (fdi == -1) perror("open");
                        dup2(fdi, STDIN_FILENO);
                        close(fdi);

                    } else if (strcmp(argv[k], ">") == 0) { // redirect out

                        argv[k] = NULL; // to avoid execp reading > as an argument
                        strcpy(input, argv[k+1]);

                        int fdo = creat(input, 0644);
                        if (fdo == -1) perror("open");
                        dup2(fdo, STDOUT_FILENO);
                        close(fdo);

                    } // checking for i/o redirection
                } // for k

                if (execvp(cmd, argv) == -1) perror(cmd); // executing the command

            } else { // parent process
                int status;
                while (!(wait(&status) == pid)); // avoids race conditions
            } // if

//********************************************************

        } // else

    } // while (infinite)

} // main
