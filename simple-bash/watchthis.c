#include<unistd.h>
#include<string.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<stdlib.h>
#include<stdio.h>
#include<fcntl.h>

#define true 1
#define false 0

void change_name(const char * first, const char *second) {
    int fork_val = fork();
    if (fork_val == 0)
        execlp("mv", "mv", first, second, NULL);
    int st;
    waitpid(fork_val, &st, 0);
}
void show (const char * buf) {
    int fork_v = fork();
    if (fork_v == 0) {
        execlp("cat", "cat", buf, NULL);
    }
    int st;
    waitpid(fork_v, &st, 0);
}
void run(char * com, char ** argv, const char* out) {
    int fork_val = fork();
    if (fork_val == 0) {
        int fd = creat(out, 0655);
        if (fd < 0)
            perror("creat failed");
        dup2(fd, 1);
        execvp(com, argv);
    }
    int st;
    waitpid(fork_val, &st, 0);
}

const char * old_v = "/tmp/old_v";
const char * new_v = "/tmp/new_v";


int main(int argc, char ** argv) {
    if (argc < 3) {
        return 1;
    }
    int sleep_time = atoi(argv[1]);
    run(argv[2], argv + 2, old_v);
    show(old_v);
    while (true) {
        sleep(sleep_time);
        run(argv[2], argv + 2, new_v);
        show(new_v);
        int fork_val = fork();
        if (fork_val == 0)
            execlp("diff", "diff", "-u", old_v, new_v, NULL);
        int st;
        waitpid(fork_val, &st, 0);
        change_name(new_v, old_v);
    }
    return 0;
}
