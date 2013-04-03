#include <unistd.h>

int main(int argc, char *argv[]) {
	write(1, argv[1], 3);
	return 0;
}
