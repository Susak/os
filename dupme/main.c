#include <unistd.h>

int get_size(char *c)
{
	int answ = 0;
	int i = 0;
	while (c[i] != 0)
	{
		answ = answ * 10 + c[i] - '0';
		i++;
	}
}

int main(int argc, char *argv[]) {
	return 0;
}
