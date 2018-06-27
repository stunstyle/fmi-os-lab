#include <ctype.h>
#include <err.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/wait.h>

#define MAX_INPUT_LENGTH 8192

static int proc_count = 0;

void append(char *s, char c) {
    int len = strlen(s);
    s[len] = c;
    s[len + 1] = '\0';
}

void trim(char *str) {
    int i;
    int begin = 0;
    int end = strlen(str) - 1;

    while (isspace((unsigned char)str[begin]))
        begin++;

    while ((end >= begin) && isspace((unsigned char)str[end]))
        end--;

    for (i = begin; i <= end; i++)
        str[i - begin] = str[i];

    str[i - begin] = '\0';
}

bool is_pipe_task(char *task) {
    for (unsigned i = 0; i < strlen(task); ++i) {
        if (task[i] == '|') {
            return 1;
        }
    }
    return 0;
}

int count_tokens(char *s) {
    int i = 1;
    int token_count = 0;
    while (s[i] != '\0') {
        if ((s[i] == ' ' && s[i - 1] != ' ') || s[i + 1] == '\0') {
            token_count++;
        }
        i++;
    }
    return token_count;
}

int count_tasks_with_delim(char *s, char delim) {
    int i = 0, count = 0;
    while (s[i] != '\0') {
        if (s[i] == delim) {
            count++;
        }
        i++;
    }

    return count + 1;
}

void split_piped_tasks(char *s, char *task1, char *task2) {
    int i = 0, j = 2;
    while (true) {
        if (s[i] == '|') {
            j--;
        }
        if (j == 0) {
            i++;
            break;
        }
        append(task1, s[i]);
        i++;
    }
    while (s[i] != '\0' && s[i] != ';') {
        append(task2, s[i]);
        i++;
    }
}

void exec_task(char *task) {
    int arg_count = count_tokens(task);
    char **cmd_with_args = (char **)malloc((arg_count + 1) * sizeof(char *));

    for (int i = 0; i < arg_count; ++i) {
        cmd_with_args[i] = (char *)malloc(512);  // ARBITRARY MAX CMD OR ARG LENGTH = 512
    }
    cmd_with_args[arg_count] = NULL;

    char *token = strtok(task, " ");
    int i = 0;
    while (token != NULL) {
        strcpy(cmd_with_args[i], token);
        i++;
        token = strtok(NULL, " ");
    }

    execvp(cmd_with_args[0], cmd_with_args);
    err(1, "Unable to execute task!");
}

void exec_line(char *s);  // declaring exec_line before calling it in exec_pipe

void exec_pipe(char *task) {
    char *task1 = strtok(task, "|");
    char *task2 = strtok(NULL, "|");

    trim(task1);
    trim(task2);
    int pf[2];

    pipe(pf);

    pid_t pid = fork();
    if (pid == -1) {
            err(1, "unable to fork");
    }
    if (pid == 0) {
        pid_t pid1 = fork();
        if (pid1 == -1) {
            err(1, "unable to fork");
    }
        if (pid1 > 0) {
            close(pf[0]);
            close(1);
            dup(pf[1]);
            exec_task(task1);
        }

        else if (pid1 == 0) {
            close(pf[1]);
            close(0);
            dup(pf[0]);
            exec_line(task2);
        }
    } else {
        waitpid(pid, NULL, WUNTRACED);
    }
}

void exec_piped_tasks(char *task1, char *task2) {
    trim(task1);
    trim(task2);
    int pf[2];

    pipe(pf);

    pid_t pid = fork();
    if (pid == -1) {
            err(1, "unable to fork");
        }
    if (pid == 0) {
        pid_t pid1 = fork();
        if (pid1 == -1) {
            err(1, "unable to fork");
    }
        if (pid1 > 0) {
            close(pf[0]);
            close(1);
            dup(pf[1]);
            if (is_pipe_task(task1)) {
                char *new_task1 = strtok(task1, "|");
                char *new_task2 = strtok(NULL, "\0");
                exec_piped_tasks(new_task1, new_task2);
            } else {
                exec_task(task1);
            }
        }

        else if (pid1 == 0) {
            close(pf[1]);
            close(0);
            dup(pf[0]);
            if (is_pipe_task(task2)) {
                char *new_task1 = strtok(task2, "|");
                char *new_task2 = strtok(NULL, "\0");
                exec_piped_tasks(new_task1, new_task2);
            } else {
                exec_task(task2);
            }
        }
    } else {
        waitpid(pid, NULL, WUNTRACED);
    }
}
void exec_line(char *line) {
    char *token = strtok(line, ";");
    while (token != NULL) {
        pid_t pid;
        pid = fork();
        if (pid == -1) {
            err(1, "unable to fork");
        }
        if (pid == 0) {
            if (is_pipe_task(token)) {
                char *task1 = strtok(token, "|");
                char *task2 = strtok(NULL, "\0");

                exec_piped_tasks(task1, task2);
            } else {
                exec_task(token);
            }
        } else {
            waitpid(pid, NULL, 0);
        }
        token = strtok(NULL, ";");
    }
    exit(0);
}

void read_tasks_into_buffer(char *buf) {
    char c;
    int readStatus = 0;
    while ((readStatus = read(0, &c, 1) > 0)) {
        append(buf, c);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        errx(1, "incorrect usage!");
    }
    int N = atoi(argv[1]);

    char cmd[MAX_INPUT_LENGTH] = "";

    read_tasks_into_buffer(cmd);

    const char *delim = "\n";
    char *token = strtok(cmd, delim);

    while (token != NULL) {
        while (proc_count >= N) {
            wait(NULL);
            proc_count--;
        }
        pid_t pid = fork();
        if (pid == -1) {
            err(1, "unable to fork");
        }
        if (pid == 0) {
            exec_line(token);
        } else {
            proc_count++;
            token = strtok(NULL, delim);
        }
    }
    while (wait(NULL) != -1) {
        // waiting for all tasks to finish
    }

    exit(0);
}