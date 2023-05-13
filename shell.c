#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#define MAX_ARGS 1024
#define MAX_INPUT 1024
#define PATH_MAX 1024
#define MAX_JOBS 10

extern char **environ;

pid_t process_pid;
int process_running = 0; // (1) status running (0) status not running or get stopped
int jobs_controller = 0; // Quantity of process paused and back-grounded
char input[MAX_INPUT];

char full_input[MAX_INPUT];
char input_file[MAX_INPUT];
char output_file[MAX_INPUT];
char error_file[MAX_INPUT];
int redirect_input = 0;
int redirect_output = 0;
int redirect_error = 0;

// Job struct
typedef struct
{
    int pid;
    int jobs_counter;
    int running;
    char input[MAX_INPUT];
} job;

job job_list[MAX_JOBS];
// Jobs command printer
void jobs()
{
    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (job_list[i].pid > 0)
        {
            printf("[%d] ", job_list[i].jobs_counter);
            if (job_list[i].running == 1)
            {
                printf("RUNNING ");
            }
            else if (job_list[i].running == 0)
            {
                printf("STOPPED ");
            }
            else if (job_list[i].running == 2)
            {
                printf("DONE ");
            }
            printf("%s >> PID: %d\n", job_list[i].input, job_list[i].pid);
        }
    }
}
// Add process to job list
int add_job(pid_t pid, char *input, int running)
{
    for (int i = 0; i < MAX_JOBS; i++)
    {
        if (job_list[i].pid == 0) // Find an empty space at list
        {
            job_list[i].pid = pid;
            job_list[i].jobs_counter = jobs_controller;
            strcpy(job_list[i].input, input);
            job_list[i].running = running;
            return 0;
        }
    }
}
// SIGTSTP handler CTRL+Z
void sigtstp_handler(int sig)
{
    if (process_pid > 0 && process_running == 1)
    {
        printf(" [%d] STOPPED %s >> PID: %d\n", jobs_controller, full_input, process_pid);
        for (int i = 0; i < MAX_JOBS; i++) // Add stopped process to job list
        {
            if (job_list[i].pid == 0)
            {
                job_list[i].pid = process_pid;
                job_list[i].jobs_counter = jobs_controller;
                job_list[i].running = 0;
                strcpy(job_list[i].input, full_input);
                jobs_controller++;
                break;
            }
        }
        fflush(stderr);
        kill(process_pid, SIGSTOP); // Stop process
        process_running = 0;
    }
}

// SIGCHLD handler
void sigchld_handler(int sig)
{
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        for (int i = 0; i < MAX_JOBS; i++)
        {
            if (job_list[i].pid == pid)
            {
                if (WIFEXITED(status))
                {
                    // Child process ended
                    int exit_status = WEXITSTATUS(status);
                    printf("PID process %d ended with status %d\n", pid, exit_status);
                    job_list[i].running = 2; // Update jobs status to DONE
                }
                else if (WIFSIGNALED(status))
                {
                    // Signal interfering
                    int terminating_signal = WTERMSIG(status);
                    printf("PID process %d ended by signal %d\n", pid, terminating_signal);
                    job_list[i].running = 2; // Update jobs status to DONE
                }
                break;
            }
        }
    }
}

int main()
{
    char *args[MAX_ARGS];
    char *token;

    // Background flag
    int background = 0;

    // Output file name
    char *outfile = NULL;

    // Counter command history
    int last_command = 0;

    // Prompt
    char *prompt = getenv("MYPS1");

    if (prompt == NULL)
    {
        prompt = "tecii";
    }

    // Reset history when reopening
    FILE *history_file = fopen(".history", "w");
    fclose(history_file);

    // SIGTSTP CTRL + Z
    signal(SIGTSTP, sigtstp_handler);

    // SIGINT CTRL + C
    signal(SIGINT, SIG_IGN);

    // SIGCHLD
    signal(SIGCHLD, sigchld_handler);

    printf("|==================|\n");
    printf("|                  |\n");
    printf("|    LINUX SHELL   |\n");
    printf("|                  |\n");
    printf("|==================|\n");

    while (1)
    {
        // ---------- STARTING SHELL ----------

        char cwd[PATH_MAX];
        getcwd(cwd, sizeof(cwd));
        printf("\n%s$:%s> ", prompt, cwd);

        // Store fgets content at input or ctrl+D
        if (fgets(input, MAX_INPUT, stdin) == NULL)
        {
            printf("\nExiting...\n");
            sleep(1);
            exit(0);
        }

        // Remove '\n' from string
        input[strcspn(input, "\n")] = 0;

        redirect_input = (strstr(input, "<") != NULL);
        redirect_output = (strstr(input, ">") != NULL);
        redirect_error = (strstr(input, "2>") != NULL);

        // Var to store original input
        strcpy(full_input, input);

        // Open .history write at end mode
        FILE *history_file = fopen(".history", "a");
        fprintf(history_file, "[%d] >> %s\n", ++last_command, input);
        fclose(history_file);

        // Strtok part input string in tokens and store at args array.
        token = strtok(input, " ");

        int i = 0;
        while (token != NULL && i < MAX_ARGS)
        {
            args[i++] = token;
            token = strtok(NULL, " ");
        }

        // ---------- KILL ----------

        if (strcmp(args[0], "kill") == 0)
        {
            if (args[1] != NULL)
            {
                int pid = atoi(args[1]);
                int signal_num = SIGTERM;
                if (args[2] != NULL)
                {
                    signal_num = atoi(args[2]);
                }
                kill(pid, signal_num);
            }

            if (args[1] == NULL && strcmp(args[0], "kill") == 0)
            {
                printf("Usage: kill <pid> [signal]\n");
                printf("SIGTERM (15) || SIGKILL (9)  || SIGINT (2)\n");
                printf("SIGSTOP (17) || SIGTSTP (20) || SIGCONT (18)\n");

                continue;
            }
            else
            {
                printf("Usage: kill <pid> [signal]\n");
            }
        }

        // ---------- HISTORY ----------

        if (strcmp(args[0], "history") == 0)
        {
            // Reading mode opening
            FILE *history_file = fopen(".history", "r");
            char line[MAX_INPUT];
            if (last_command <= 50)
            {
                while (fgets(line, MAX_INPUT, history_file))
                {
                    printf("%s", line);
                }
            }
            else
            {
                int num_printed = 0;
                while (fgets(line, MAX_INPUT, history_file))
                {
                    if (num_printed >= last_command - 50)
                    {
                        printf("%s", line);
                    }
                    num_printed++;
                }
            }
            // Close the .history file
            fclose(history_file);
            // Skip the rest of the loop
            continue;
        }

        // ---------- JOBS ----------

        if (strcmp(args[0], "jobs") == 0)
        {
            jobs();
            continue;
        }

        // ---------- FOREGROUND AND BACKGROUND ----------

        if (i > 0 && strcmp(args[i - 1], "&") == 0) // Check if last argument is "&"
        {
            args[i - 1] = NULL; // Remove "&" from args
            background = 1;     // Set background flag
        }
        else
        {
            args[i] = NULL;
            background = 0;
        }

        // ---------- EXIT COMMAND ----------

        if (strcmp(args[0], "exit") == 0)
        {
            printf("Exiting...\n");
            sleep(1);
            exit(0);
        }
        // ---------- CHANGE DIRECTORY ----------

        if (strcmp(args[0], "cd") == 0 && args[1] != NULL)
        {
            if (chdir(args[1]) == -1)
            {
                // Try to change dir
                printf("cd: %s: No such file or directory\n", args[1]);
            }
            else
            {
                if (getcwd(cwd, sizeof(cwd)) != NULL)
                {
                    setenv("PWD", cwd, 1);
                }
                else
                {
                    perror("getcwd() error");
                }
            }
            continue;
        }
        // ---------- EXPORT MYPS1 ----------

        if (strcmp(args[0], "export") == 0 && args[1] != NULL && strncmp(args[1], "MYPS1=", 6) == 0)
        {
            char *new_prompt = &input[13];
            setenv("MYPS1", new_prompt, 1);
            prompt = getenv("MYPS1");
            continue;
        }

        // ---------- SET ----------

        if (strcmp(args[0], "set") == 0 && args[1] == NULL)
        {
            char **env = environ;
            while (*env != NULL)
            {
                printf("%s\n", *env);
                env++;
            }
            continue;
        }

        // ---------- EXPORT PATH ----------

        char *mypath = getenv("MYPATH");

        if (mypath == NULL)
        {
            char *path = getenv("PATH");
            if (path != NULL)
            {
                setenv("MYPATH", path, 1);
                mypath = getenv("MYPATH");
            }
        }

        if (strcmp(args[0], "export") == 0 && args[1] != NULL)
        {
            char *path_start = strstr(args[1], ":/");
            if (path_start != NULL)
            {
                path_start += 2;
                // Copy the rest of the string after ":/" to new_path
                char new_path[strlen(path_start) + 1];
                strcpy(new_path, path_start);
                char updated_path[strlen(mypath) + strlen(new_path) + 1];
                sprintf(updated_path, "%s:/%s", mypath, new_path);
                setenv("MYPATH", updated_path, 1);
                mypath = getenv("MYPATH");
                printf("$MYPATH>> %s\n", mypath);
                continue;
            }
        }

        // ---------- ECHO ----------

        if (strcmp(args[0], "echo") == 0 && args[1] != NULL)
        {
            if (args[1][0] == '$') // Environment variable case
            {
                char *var_name = &args[1][1];       // Get var name
                char *var_value = getenv(var_name); // Value
                if (var_value != NULL)
                {
                    printf("%s\n", var_value);
                }
                else
                {
                    printf("Environment variable not found: %s\n", var_name);
                }
                continue;
            }
            else // Characters print case
            {
                for (int i = 1; args[i] != NULL; i++)
                {
                    printf("%s \n", args[i]);
                }
                continue;
            }
        }

        //---------- MYPATH ----------

        if (mypath != NULL && strcmp(mypath, "") != 0)
        {
            // Tokenize the path variable and store in array of directories
            char *path_copy = strdup(mypath);
            char *dir = strtok(path_copy, ":");
            char *dirs[MAX_ARGS]; // directories array
            int path_num = 0;
            while (dir != NULL && i < MAX_ARGS)
            {
                dirs[path_num++] = dir;
                dir = strtok(NULL, ":"); // Separete mypath
            }
            dirs[path_num] = NULL;
            int found = 0;
            char command[MAX_INPUT];
            // Find command through directories
            for (int k = 0; dirs[k] != NULL && !found; k++)
            {
                sprintf(command, "%s/%s", dirs[k], args[0]);
                if (access(command, X_OK) == 0)
                {
                    args[0] = strdup(command);
                    found = 1;
                    continue;
                }
            }
            if (!found)
            {
                printf("Command not found in MYPATH\n");
                continue;
            }
            free(path_copy);
        }

        // ---------- REDIRECT STDIN STDOUT STDERR ----------

        input_file[0] = '\0';
        output_file[0] = '\0';
        error_file[0] = '\0';

        if (redirect_input)
        {
            int j = 0;
            while (args[j] != NULL)
            {
                if (strcmp(args[j], "<") == 0)
                {
                    if (args[j + 1] != NULL)
                    {
                        strcpy(input_file, args[j + 1]);
                        args[j] = NULL;
                        args[j + 1] = NULL;
                        break;
                    }
                }
                j++;
            }
        }

        if (redirect_error)
        {
            int j = 0;
            while (args[j] != NULL)
            {
                if (strcmp(args[j], "2>") == 0)
                {
                    if (args[j + 1] != NULL)
                    {
                        strcpy(error_file, args[j + 1]);
                        args[j] = NULL;
                        args[j + 1] = NULL;
                        break;
                    }
                }
                j++;
            }
        }

        if (redirect_output)
        {
            int j = 0;
            while (args[j] != NULL)
            {
                if (strcmp(args[j], ">") == 0)
                {
                    if (args[j + 1] != NULL)
                    {
                        strcpy(output_file, args[j + 1]);
                        args[j] = NULL;
                        args[j + 1] = NULL;
                        break;
                    }
                }
                j++;
            }
        }

        // ---------- FORKING ----------

        process_pid = fork(); // Process Identifier

        if (process_pid < 0) // Error creating child process
        {
            printf("Error creating process\n");
            exit(1);
        }

        // -------- CHILD PROCESS --------

        else if (process_pid == 0)
        {

            // ------- REDIRECTING STDIN STDOUT AND STDERR -------
            if (redirect_input)
            {
                int fd_inputfile = open(input_file, O_RDONLY);
                if (fd_inputfile == -1)
                {
                    perror("Error opening input file");
                    exit(EXIT_FAILURE);
                }
                dup2(fd_inputfile, STDIN_FILENO);
                close(fd_inputfile);
            }

            if (redirect_output)
            {
                int fd_outputfile = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                if (fd_outputfile == -1)
                {
                    perror("Error opening output file");
                    exit(EXIT_FAILURE);
                }
                dup2(fd_outputfile, STDOUT_FILENO);
                close(fd_outputfile);
            }

            if (redirect_error)
            {
                int fd_errorfile = open(error_file, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
                if (fd_errorfile == -1)
                {
                    perror("Error opening error file");
                    exit(EXIT_FAILURE);
                }
                dup2(fd_errorfile, STDERR_FILENO);
                close(fd_errorfile);
            }

            if (execvp(args[0], args) < 0)
            {
                printf("Error executing command\n");
                exit(1);
            }
        }

        // -------- FATHER PROCESS --------

        else // process_pid > 0
        {
            // Foreground
            if (!background)
            {
                process_running = 1;
                waitpid(process_pid, NULL, WUNTRACED);
                process_running = 0;
            }
            // Add back ground process to jobs list
            else
            {
                int running = 1;
                add_job(process_pid, full_input, running);

                printf("[%d] RUNNING %s >> PID: %d\n", jobs_controller, full_input, process_pid);
                jobs_controller++;
            }
        }
    }
    return 0;
}