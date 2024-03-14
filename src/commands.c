#include "commands.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int execute_command(const char *command, char *output, size_t output_size)
{
    int     pipefd[2];
    pid_t   pid;
    int     status;
    ssize_t bytes_read;

    // Create pipe
    if(pipe2(pipefd, O_CLOEXEC) == -1)
    {
        perror("pipe");
        return EXIT_FAILURE;
    }

    // Fork a child process
    pid = fork();
    if(pid == -1)
    {
        perror("fork");
    }
    else if(pid == 0)
    {    // Child process
        // Close read end of the pipe
        close(pipefd[0]);

        // Redirect stdout to write end of the pipe
        dup2(pipefd[1], STDOUT_FILENO);

        // Close unused write end of the pipe
        close(pipefd[1]);

        // Execute command
        if(execl("/bin/sh", "sh", "-c", command, NULL) == -1)
        {
            perror("execl");
            // Exit child process with failure
            exit(EXIT_FAILURE);
        }
    }
    else
    {    // Parent process
        // Close write end of the pipe
        close(pipefd[1]);

        // Read output from the read end of the pipe
        bytes_read = read(pipefd[0], output, output_size);
        if((int)bytes_read == -1)
        {
            perror("read");
            return EXIT_FAILURE;
        }

        // Null-terminate output string
        if(bytes_read < (ssize_t)output_size)
        {
            output[bytes_read] = '\0';
        }
        else
        {
            output[output_size - 1] = '\0';
        }

        // Wait for child process to terminate
        if(waitpid(pid, &status, 0) == -1)
        {
            perror("waitpid");
            return EXIT_FAILURE;    // Return -1 to indicate error
        }

        // Check if child process exited normally
        if(!WIFEXITED(status) || WEXITSTATUS(status) != 0)
        {
            fprintf(stderr, "Child process failed to execute command\n");
            return EXIT_FAILURE;    // Return -1 to indicate error
        }
    }

    return EXIT_SUCCESS;
}
