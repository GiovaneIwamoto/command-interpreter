# COMMAND INTERPRETER

### **INTRODUCTION**

This shell is a command-line interpreter developed in C specifically for the Unix environment. It offers a comprehensive set of features and functionalities to enhance the command-line experience for users.

### **FEATURES AND BUGS**

- FOREGROUND AND BACKGROUND

Commands can be executed both in the foreground and in the background. To execute in the background, simply append the '&' character at the end of the input. When a process is running in the foreground, the shell does not accept new inputs until the respective process completes or is interrupted by a signal. Conversely, when we have a process running in the background, it allows interactions with the command line while a process is running in the background.

- REDIRECTING STDOUT OR STDERR

The redirection of standard output (STDOUT) is implemented correctly, behaving as expected and redirecting output accordingly. However, when it comes to error output, there are some undesired behaviors related to issues such as not finding the output file.

- REDIRECTING STDIN

Redirecting from standard input (STDIN) to an input file is a functionality that has been added to the command interpreter, and it works correctly using a similar structural logic as the implementations of standard output and error redirection.

- MYPATH

The programs to be executed are searched in the directories listed in the MYPATH environment variable. The initial content of the variable is correctly imported from the PATH environment variable. This custom environment variable contains a list of directories separated by colons (":") where the operating system looks for commands to be executed. The responsible function searches for the commands by traversing the directories until the "found" flag is updated. If a command is not listed in MYPATH, an error message is displayed.

- EXPORT

The export command allows the definition and incrementation of environment variables. Initially, it contains the content of PATH and concatenates the new directory passed by the export command to the set of directories in MYPATH. This enables the addition of new environment variables that the user can access and utilize. Similarly to the previous functionality implementation, all directories are standardized to be separated by colons (":") and slashes ("/") to ensure that the search for commands and the update of the "found" flag continue to work correctly

- CD PWD

Changing directories can be done using the "cd" command followed by the desired directory. When this functionality is performed, the "pwd" variable is correctly updated to reflect the targeted directory. Upon starting the shell, the "pwd" variable obtains the current value of the current directory.

An improvement has been made to the visual aspect of the shell. Similar to other command interpreters, the prompt now displays the current directory to provide the user with information about their current location. If an attempt is made to change to a directory that does not exist, an error notification is displayed.

- HISTORY

The "history" command is functioning correctly, displaying the last 50 commands entered by the user. Each time a command is entered, the ".history" file is opened and the input is recorded. However, there is currently an issue with the implementation related to directory changes, which affects the history and command counter. This error causes commands executed in other directories to not be listed in the history. Additionally, the command counter increments incorrectly, resulting in inconsistencies in the count and display of commands in the listing. This issue should be addressed to ensure accurate tracking and display of command history.

- MYPS1

The command interpreter always starts with the prompt set as "tecii$". However, this can be redefined using the environment variable MYPS1. By executing the command "export MYPS1=newprompt", the prompt changes to "newprompt$:/dir> ". This new prompt will remain in effect until there is another modification to the MYPS1 variable.

- EXIT

The shell can be terminated correctly using either the "exit" command or by typing Ctrl+D. Both methods allow for a graceful termination of the shell, ensuring that any necessary cleanup or finalization steps are performed before exiting the program. This provides a convenient and flexible way for users to exit the shell based on their preference or workflow.

- SIGINT

The signal handling for SIGINT prevents the interpreter from terminating when the Ctrl+C keys are pressed, allowing the shell to continue running normally. By intercepting the SIGINT signal, the shell can take appropriate actions to handle or ignore the interrupt signal as desired. This ensures that the shell remains responsive and allows users to continue their interactions without prematurely terminating the shell process.

- SIGTSTP

The SIGTSTP signal triggers a handler that appropriately handles the foreground process by pausing it, freeing the shell for new inputs, and storing the PID of the paused process in the Jobs list. When the Ctrl+Z keys are pressed on the keyboard, the process running in the foreground can be paused by sending the signal to the handler, where it will be handled accordingly. By pausing the foreground process, the shell allows users to temporarily halt its execution, giving them the ability to interact with other commands or perform additional tasks without terminating the process entirely. This feature provides flexibility and control over process management within the shell.

- KILL

The "kill" command is used to send signals to processes and follows the syntax: `kill <pid> [signal]`. Both the PID and signal are integers. If no signal value is specified, the default value of SIGTERM is used.

When the "kill" command is invoked with incorrect syntax, it displays an example usage of the command. In some specific cases, the command may trigger the default notification of the "/bin/kill" program instead of the expected behavior. This can result in unintended notifications being displayed upon invocation. However, the functionality is still executed based on the implemented source code. To avoid any confusion, it is recommended to track processes using both the Jobs list and the "ps -ef" command.

By closely monitoring the Jobs list and utilizing the "ps -ef" command, users can ensure a more accurate understanding of how the "kill" command is behaving and how it affects processes.

- JOBS

The "jobs" command lists the processes that have been stopped by the SIGTSTP signal with the status "STOPPED" and the processes running in the background with the status "RUNNING". The listing is displayed in the format `[(jobs_counter)] (status) (input) >> PID: (PID)`.

If a background process is terminated during the execution of the shell, the user is notified about the termination and the corresponding signal/status. The status in the Jobs list is automatically changed to "DONE" when a process finishes. Similarly, if a process is terminated using the kill command with the termination signal, the status is also appropriately updated in the Jobs list.

This provides users with visibility and information about the status of background and stopped processes, allowing them to manage and monitor their execution effectively within the shell.

- SIGCHLD

The signal handling for SIGCHLD and the handling of zombie processes are functionalities in which the handler effectively manages in the interpreter. This handler takes the responsibility of updating the Jobs list with the appropriate statuses and notifies the shell interface with the PID of the respective process and the termination reason.

When a child process terminates, the SIGCHLD signal is generated, and the handler takes action to handle the termination properly. It updates the Jobs list, marking the corresponding process as "DONE" and providing the necessary information about the termination, such as the PID and the reason for termination.

By managing zombie processes effectively, the shell ensures that resources are properly released and that the user is informed about the termination of background processes. This enhances the overall functionality and user experience of the shell.

- ECHO

The "echo" command is used to print strings, and if the command is followed by the "$" symbol and the name of an environment variable, it prints the entire content of that variable. If the variable does not exist or is invalid, an error message informs the user about the nonexistence of the parameter.

By using the "echo" command with the "$" symbol and an environment variable name, users can conveniently print the value of a specific variable. This feature allows for dynamic printing of variable contents, providing flexibility and customization within the shell. If an invalid or non-existent variable is used, the user is promptly notified to avoid any confusion or unexpected behavior.

- SET

The "set" command has been implemented to display all environment variables. When executed, the "set" command lists all the current environment variables, providing users with a comprehensive view of the existing variables and their respective values.

### **AUTHOR**

- Giovane Hashinokuti Iwamoto - Computer Science student at UFMS - Brazil - MS

I am always open to receiving constructive criticism and suggestions for improvement in my developed code. I believe that feedback is an essential part of the learning and growth process, and I am eager to learn from others and make my code the best it can be. Whether it's a minor tweak or a major overhaul, I am willing to consider all suggestions and implement the changes that will benefit my code and its users.
