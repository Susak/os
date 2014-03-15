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
sigatomic_t sbreak = 0;
sigatomic_t sprint = 0;

void handler2(int signum)
{
    sprint = 1;
}

void handler(int signum)
{
   sbreak = 1;
}
vector<string> parse(const char * st, char delim)
{
    vector<string> result;
    stringstream ss(st);
    string temp;
    while (getline(ss, temp, delim))
    {
        result.push_back(temp);
    }
    return result;
}
int main(int argc, char ** argv) {
    if (argc != 4)
    {
        return 1;
    }
    int buf_size = atoi(argv[1]);
    int in_fd = open(argv[2], O_RDWR, S_IRWXU);
    out_fd = open(argv[3], O_RDWR, S_IRWXU);
    vector<char> buf(buf_size);
    int count = 0;
    signal(SIGINT, handler);
    signal(SIGUSR1, handler2);
    while(true) {
        if (sprint == 1)
            cout << write_count;

        if (sbreak == 1)
            unlink(argv[3]);
        int read_count = read(in_fd, buf.data() + count, buf_size - count);
        count += read_count;
        if (read_count == 0 && write_count == count)
            return 0;
        int write_count += write(out_fd, buf.data() + write_count,
               count); 
        if (count == buf_size && write_count == count)
            return 0;
    }
    return 0;
}

