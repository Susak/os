#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int get_size(char *c)
{
	int answ = 0;
	int i = 0;
	while (c[i] != 0)
	{
		answ = answ * 10 + c[i] - '0';
		i++;
	}
	return answ;
}

void parse_string(size_t size)
{
	int is_new_string = 1;
	size_t counter = 0;
	size_t count_of_write = 0;
	size_t count_to_read = 0;
	char *buffer = malloc(size);
	char *temp_buf = malloc(size);
	size_t i = 0;
	size_t read_counter = 1;
	while (read_counter > 0)
	{
		read_counter = read(0, buffer + count_to_read, size - count_to_read);
		counter = read_counter + count_to_read;
		count_of_write = 0;
		i = 0;
		while ( i < read_counter)
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
					is_new_string = 1;
				}
			}
		i++;
		}
		if (count_of_write != 0)
		{
			count_to_read = counter - count_of_write;
			memcpy(temp_buf, buffer + count_of_write, count_to_read);
			buffer = malloc(size);
			memcpy(buffer, temp_buf, count_to_read);
			temp_buf = malloc(size);
		} else {
			buffer = malloc(size);
			is_new_string = 0;
		}
	}
}
int main(int argc, char *argv[]) {
	size_t size = get_size(argv[1]);
	parse_string(size + 1);
	return 0;
}
