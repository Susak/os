#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

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
	char *buffer = malloc(size);
	size_t i = 0;
	size_t read_counter = 1;
	while (read_counter > 0)
	{
		read_counter = read(0, buffer, size);
		i = 0;
		while ( i < read_counter)
		{
			if (buffer[i] == '\n')
			{
				write(1, buffer, i + 1);
				write(1, buffer, i + 1);	
			}
		i++;
		}
	}
}
int main(int argc, char *argv[]) {
	size_t size = get_size(argv[1]);
	printf("%zu\n", size);
	parse_string(size + 1);
	return 0;
}
