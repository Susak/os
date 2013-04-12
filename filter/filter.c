#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define true 1
#define false 0
typedef int bool;

void write_(size_t size, char *buffer)
{
    size_t k;
    for (k = 0; k < size;)
    {
        size_t count_of_write = write(1,
                buffer + k, size - k);
        k += count_of_write;
    }
}

int find_pattern(char *buffer, size_t size, char pattern)
{
    int i = 0;
    while (i < size && buffer[i] != pattern)
        i++;
    if (i != size)
        return i;
    return -1;
}

int main (int argc, char *argv[]) { 
    char pattern = '\n';
    int rez = 0;
    size_t position = 0;
    size_t size = 4096;
    while ((rez = getopt(argc, argv, "nzb:")) != -1)
    {
        switch (rez) {
            case 'n' : pattern = '\n'; break;
            case 'z' : pattern = 0; break;
            case 'b' : size = atoi(optarg); break; 
        }
    } 
    char * buffer = malloc(size + 1);
    char ** strings = malloc(argc + 1 - optind);
    int k = optind;
    for (; k < argc; k++)
    {
        strings[k - optind] = argv[k];
        if (strcmp(argv[k], "{}") == 0)
            position = k - optind;
    }
    strings[argc - optind] = 0;
    int count_r = 0;
    int sz = 0;
    int pos = 0; 
    bool eof = false;
    while (true)
    {
        if (eof)
            break;
        if (sz == size)
            return 1;
        count_r = read(0, buffer + sz, size - sz);
        
        if (count_r == 0)
        {
            eof = true;
            buffer[sz] = pattern;
            count_r = 1;
        }

        pos = find_pattern(buffer, count_r, pattern);
        pos += sz;
        sz += count_r;
        while (true)
        {
            if (pos == -1)
                break;
            buffer[pos] = 0;
            int child_st;
            int fork_val = fork();
            if (fork_val == 0)
            {
                strings[position] = buffer;
                execvp(strings[0], strings);
                exit(1);
            }
            waitpid(fork_val, &child_st, 0);
            if (WIFEXITED(child_st) &&
                    (WEXITSTATUS(child_st) == 0))
            {
                buffer[sz] = '\n';
                write_(sz + 1, buffer);
            }
            memcpy(buffer, buffer + pos + 1, sz - pos - 1);
            sz -= pos + 1;
            pos = find_pattern(buffer, sz, pattern);
        }
    }
    free(buffer);
    free(strings);
    return 0;
}
