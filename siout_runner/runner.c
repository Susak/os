#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<unistd.h>
#include<utility>

#define STDIN 0
#define STDOUT 1

int find_delim(char *buf, char delim, size_t sz) {
    int i = 0;
    for (; i <= sz; i++) {
        if (buf[i] == 0)
            return i + 1;
    }
    return -1;
}

std::pair<int, int> find_end(char* buf, size_t sz) {
    int i = 1;
    int end_of_str = -1;
    for (; i <= sz; i++) {
        if (buf[i - 1] == 0 && buf[i] == 0) {
            end_of_str = i;
            break;
        }
    }
    i -= 2;
    for (;i >= 0; i--) {
        if (buf[i] == 0)
            return std::make_pair(i + 1, end_of_str);
    }
}

int run_command(char* buf[], char* in, char* out) {
    int pid = fork();
    if (pid == 0) { //child
        int fd_in = open(in, O_RDONLY);
        if (fd_in == -1)
            return -1;
        int fd_out = open(out, O_WRONLY | O_CREAT, 0644);
        dup2(fd_in, STDIN);
        close(fd_in);
        dup2(fd_out, STDOUT);
        close(fd_out);
        execvp(buf[0], buf);
    }
}

int main(int argc, char ** argv) {
    size_t buf_size = 4096;
    char* buffer = (char*) malloc(buf_size * sizeof(char));
    int delim_pos = 0;
    int fd = open(argv[1], O_RDONLY);
    int count_of_read = 0;
    std::pair<int, int> position;
    while (true) {
        count_of_read = read(fd, buffer, buf_size);
        if (count_of_read <= 0)
            break;
        int end_of_str = 0;
        while (end_of_str <= count_of_read) {
            int first_del = find_delim(buffer + end_of_str,
                    '\0', count_of_read);
            position = find_end(buffer + end_of_str, count_of_read);
            int str_end = position.second;
            int sec_delim = position.first;
            if (sec_delim == -1) {
               return 0;
            }
            char* input = buffer + end_of_str;
            char* output = buffer + sec_delim + end_of_str;
            char* command_buf[str_end / 2 + 1];
            command_buf[0] = buffer + first_del + end_of_str;
            int j = first_del;
            int i = 1;
            int k = first_del;
            while (true) {
                j = find_delim(buffer + j + end_of_str, '\0',
                        count_of_read - sec_delim);
                k += j;
                if (k >= sec_delim)
                    break;
                command_buf[i] = buffer + k + end_of_str;
                i++;
            }
            command_buf[i] = 0;
            if (run_command(command_buf, input, output) == -1)
                return 0;
            end_of_str += str_end;
        }
    }
    free(buffer);
    return 0;
}
