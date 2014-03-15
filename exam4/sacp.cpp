#include <iostream>
#include <signal.h>
#include <fcntl.h>
#include <vector>
#include <unistd.h>
#include <string>
#include <algorithm>
#include <sstream>
#include <stdio.h>
#include <utility>

using namespace std;
sig_atomic_t sret = 0;

void h2(int signal)
{
    sret = 1;
}
void panic(const char* message)
{
     perror(message);
     exit(EXIT_FAILURE);
}

void safe_write(int fd, vector<char> &buf, int read_count)
{
    size_t written = 0;
    while (written < read_count) 
    {
        int write_count = write(fd, buf.data(), read_count);

        if (write_count < 0)
        {
            panic("error on writing");
        }
        written += write_count;
    }
}
int main(int argc, char ** argv) {
    if (argc < 3)
    {
        panic("wrong arguments");
    }
    vector<int> fd_read(argc - 2);
    for (int i = 1; i < argc - 1; i++)
    {
        int fd = open(argv[i], O_RDONLY, S_IRWXU);
        if (fd < 0)
        {
            panic("error on opening file to read");
        }
        fd_read[i - 1] = fd;
    }

    int fd_wr = open(argv[argc - 1], O_WRONLY | O_CREAT | O_EXCL, S_IRWXU);

    if (fd_wr < 0)
    {
        panic("error on creating file to write or file already exist");
    }

    if (signal(SIGINT, h2) == SIG_ERR) panic("signal SIGINT");

    int buf_size = 1;

    vector<char> buffer(buf_size);
    for (int i = 0; i < fd_read.size(); i++) {
        while(true) {
            int read_count = read(fd_read[i], buffer.data(), buf_size);
            if (read_count < 0)
                panic("error on reading");

            if (read_count == 0)
                break;

            safe_write(fd_wr, buffer, read_count);

            if (sret) {
                if (unlink(argv[argc - 1]) == -1)
                        panic("error when unlink file");
                exit(EXIT_SUCCESS);
            }

        }
    }
    exit(EXIT_SUCCESS);
}
