#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<utility>

int ok = 0;

int find_delim(char *buf, char delim, size_t sz)
{
    int i = 0;
    for (; i < sz; i++) {
        if (buf[i] == delim)
            return i;
    }
    return -1;
}

void _write(int fd, char *buf, size_t sz) {
    int _is_ok = write(fd, buf, sz);
    if (_is_ok < 0)
            printf("problem with writing");
    printf("\n");
}
void run_command(char * buf[], char *string, size_t sz) {
    int fork_val = fork();
    if (fork_val == 0) {
        int fd;
        dup2(0, fd);
        _write(fd, string, sz);
        close(fd);
        execvp(buf[0], buf);
        exit(1); 
    }
}

std::pair<size_t, char*>  build_str(char *buf, size_t sz) {
    int i = 0;
    int j = 0;
    char * buffer_to_write = (char *)malloc(sz / 2 + 1);
    while (i <= sz) {
        if (i % 2 == 0) {
            if (!ok)
                buffer_to_write[j] = buf[i];
            else
                buffer_to_write[j] = buf[i + 1];
            j++;
        }
        i++;
    }
    ok = 1;
    return std::make_pair(j, buffer_to_write);
}
int main(int argc, char *argv[]) {
    size_t buf_size = 10;
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
        ret_pair = build_str(buf, delim_pos + 1);
        //_write(1, ret_pair.second, ret_pair.first);
        command_buffer[argc - 1] = ret_pair.second;
        run_command(command_buffer, ret_pair.second, ret_pair.first);
        memmove(buf, buf + delim_pos + 1, count_of_write);
    }
}
