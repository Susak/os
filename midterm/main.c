#include <unistd.h>
#include <string.h>
#include <stdlib.h>

int main (int argc, char *argv[]) { 
    char pattern;
    char *strings[argc + 1];
    int rez = 0;
    size_t size = 4097;
    while ((rez = getopt(argc, argv, "nzb:")) != -1)
    {
        switch (rez) {
            case 'n' : pattern = '\n'; break;
            case 'z' : pattern = '\0'; break;
            case 'b' : size = atoi(optarg) + 1; break; 
        }
    }
    int k = optind;
    for (; k < argc; k++)
    {
  //    strcpy( strings[k - optind], argv[k]);
    }
    strings[argc] = 0; 
    char *new_line = malloc(1);
    new_line[0] = pattern; 
    char *buffer = malloc(size);
    size_t count_of_read = 1;
    size_t counter = 0;
    size_t count_of_write = 0;
    char *temp_buf = malloc(size);
    while (count_of_read > 0)
    {   
        int temp_read = count_of_read;
        int ok = 0; 
        count_of_read = read(0, buffer, size);
        int i = 0;
        count_of_write = 0;
        for (i = 0; i < count_of_read; i++)
        {
            if (buffer[i] == pattern)
            {
                int pid = fork();
                if (pid == 0) {

                    ok = 1;
                    strncpy(strings[argc], buffer + count_of_write,
                       i - count_of_write + 1);
                     count_of_write = i + 1; 
                    execvp(argv[optind], strings);
                    write (1, temp_buf, count_of_write);
            }
            }
        }
        if (count_of_read == 0 && temp_read - count_of_write < size - 1)
        {
            memcpy(buffer, new_line, 1);
            memcpy(temp_buf, buffer + count_of_write,
                    temp_read - count_of_write + 2);
           // execvp(argv[command_id], temp_buf
            
        }
        if (!ok) {
            return 0;
        }
    }
    return 0;
}
