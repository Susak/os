#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define true 1
#define false 0
typedef int bool;

void parse_string(size_t size)
{
    char *buf_new_line = malloc(1);
    if (buf_new_line == NULL) 
        _exit(2);
    buf_new_line[0] = '\n';
    bool is_new_string = true;
    size_t counter = 0;
    size_t count_of_write = 0;
    size_t count_to_read = 0;
    char *buffer = malloc(size);
    if (buffer == NULL)
        _exit(2);
    char *temp_buf = malloc(size);
    if (temp_buf == NULL)
        _exit(2);
    size_t i = 0;
    size_t read_counter = 1;
    while (read_counter > 0)
    {
        read_counter = read(0, buffer + count_to_read, size - count_to_read);
        counter = read_counter + count_to_read;
        count_of_write = 0;
        i = 0;
        while ( i < counter)
        {
            if (buffer[i] == '\n')
            {
                if (is_new_string)
                {
                    write(1, buffer + count_of_write, i - count_of_write + 1);
                    write(1, buffer + count_of_write, i - count_of_write + 1);
                    count_of_write = i + 1;
                } else {
                    count_of_write = i + 1;
                    is_new_string = true;
                }
            }
            i++;
        }
        if (count_of_write != 0)
        {
            count_to_read = counter - count_of_write;
            memcpy(temp_buf, buffer + count_of_write, count_to_read);
            buffer = malloc(size);
            if (buffer == NULL)
                _exit(2);
            memcpy(buffer, temp_buf, count_to_read);
            temp_buf = malloc(size);
            if (temp_buf == NULL)
                _exit(2);
        } else if(counter == size) {
            buffer = malloc(size);
            if (buffer == NULL)
                _exit(2);
            is_new_string = false;
        } else {
            count_to_read = counter;
        }
    }
    if (is_new_string && count_to_read < size)
    {
        write(1, buffer, count_to_read);
        write(1, buf_new_line, 1);
        write(1, buffer, count_to_read);
        write(1, buf_new_line, 1);
    }
}
int main(int argc, char *argv[])
{
    if (argc < 2)
        return 1;
    size_t size = atoi(argv[1]) + 1;
    parse_string(size);
    return 0;
}
