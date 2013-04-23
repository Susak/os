#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<utility>
#include <sys/wait.h>

int find_delim(char *buf, char delim, size_t sz)
{
    int i = 0;
    for (; i <= sz; i++) {
        if (buf[i] == delim)
            return i + 1;
    }
    return -1;
}

void _write(int fd, char *buf, size_t sz) {
    int print_len = 0;
    while (print_len < sz) {
        int add_len = write(fd, buf + print_len, sz - print_len);
        if (add_len < 0)
            printf("problem with writing");
        print_len += add_len;
    }
    printf("\n");
}
void run_command(char * buf[], char *string, size_t sz) {
    int fork_val = fork();
    int child_status;
    if (fork_val == 0) {
        execvp(buf[0], buf);
        exit(0); 
    } else {
       pid_t tpid;
      do {
        tpid = wait(&child_status);
      } while (tpid != fork_val); 
    }
}

std::pair<size_t, char*>  build_str(char *buf, size_t sz) {
    int i = 0;
    int j = 0;
    char * buffer_to_write = (char *)malloc(sz / 2 + 1);
    while (i < sz) {
        if (i % 2 == 1) {
            buffer_to_write[j] = buf[i];
            j++;
        }
        i++;
    }
    return std::make_pair(j, buffer_to_write);
}
int main(int argc, char *argv[]) {
    size_t buf_size = 4096;
    char *buf = (char *)malloc(buf_size + 1);
    int  delim_pos = 0;
    size_t count_of_write = 0;
    size_t read_count = 0;
    std::pair<size_t, char*> ret_pair;
    char *command_buffer[argc + 1];
    int k = 1;
    for (; k < argc; k++) {
       command_buffer[k - 1] = argv[k]; 
    }
    command_buffer[argc] = NULL;
    char *buffer_to_write = (char *)malloc (buf_size / 2 + 1);
    while (true) {
        int count_of_read = read(0, buf + count_of_write,
                    buf_size - count_of_write);
        read_count = count_of_write + count_of_read;
        if (count_of_read == 0) {
            free(buf);
            free(command_buffer);
            free(buffer_to_write);
            return 0;
        }
        if (count_of_read < 0) {
            free(buf);
            free(command_buffer);
            free(buffer_to_write);
            printf("Problem with reading\n");
            return 0;
        }
        delim_pos = find_delim(buf,
                     '\n', read_count);
        count_of_write = read_count - delim_pos; 
        if (delim_pos == -1) {
            printf("There is no delimetr\n");
            return 0;
        }
        ret_pair = build_str(buf, delim_pos);
        command_buffer[argc - 1] = ret_pair.second;
        run_command(command_buffer, ret_pair.second, ret_pair.first);
        memmove(buf, buf + delim_pos, count_of_write);
    }
}
