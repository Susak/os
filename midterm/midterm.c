#include<unistd.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#define true 1
#define false 0
typedef int bool;

int ok = 0;

int find_delim(char *buf, char delim, size_t sz)
{
    int i = 0;
    for (; i < sz; i++) {
        if (buf[i] == '\n')
            return i;
    }
    return -1;
}

size_t  build_str(char *buf, size_t sz, char *buffer_to_write) {
    int i = 0;
    int j = 0;
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
    return j;
}
void _write(char *buf, size_t sz) {
    int _is_ok = write(1, buf, sz);
    if (_is_ok < 0)
            printf("problem with writing");
    printf("\n");
}
int main(int argc, char *argv[]) {
    size_t buf_size = 10;
    char *buf = malloc(buf_size + 1);
    int  delim_pos = 0;
    size_t count_of_write = 0;
    size_t read_count = 0;
    char *buffer_to_write = malloc (buf_size / 2 + 1);
    while (true) {
        int count_of_read = read(0, buf + count_of_write,
                    buf_size - count_of_write);
        read_count = count_of_write + count_of_read;
        if (count_of_read == 0)
            return 0;
        if (count_of_read < 0) {
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
        size_t size_to_write = build_str(buf, delim_pos + 1,
                buffer_to_write);
        _write(buffer_to_write, size_to_write);
        memmove(buf, buf + delim_pos + 1, count_of_write);
    }
}
