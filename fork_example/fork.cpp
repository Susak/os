#include <sys/types.h>
#include <wait.h>
#include <boost/optional.hpp>
#include <iterator>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <vector>
#include <signal.h>
#include <algorithm>
#include <poll.h>
#include <iostream>

#define BUF_SIZE 500

using namespace std;

const int sz = 4096;

void panic(const char * msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

void safe_write(int fd, vector<char> &buf, int read_cout)
{
    int written = 0;
    while(written < read_cout)
    {
        int write_count = write(fd, buf.data(), read_cout - written);
        if (write_count < 0)
            panic("Error in write");
        written += write_count;
    } 
}

int parse(vector<char> & buf) 
{
    for (int i = 0; i < buf.size(); i++)
    {
        if (buf[i] == '\n')
            return i;
    }
    return -1;
}
vector<int> sockets;
int pid;
void h(int signal)
{
    if (pid) {
        kill(pid, SIGINT);
    } else {
        for (int i = 0; i < sockets.size(); i++) 
        {
            kill(sockets[i], SIGKILL);
        }
        kill(pid, SIGINT);
    }
        _exit(EXIT_SUCCESS);
}

void h2(int signal)
{
    exit(EXIT_SUCCESS);
}
struct client {
    vector<char> to_read;
    vector<char> to_write;
    int shift_r;
    int shift_w;
    bool end;

    client() : to_read(sz), to_write(0), shift_r(0), shift_w(0), end(false) {}

    void resize() {
        to_read.resize(sz);
    }

    bool client_read(int fd)
    {
        int read_count = read(fd,
                to_read.data() + shift_r,
                to_read.size() - shift_r);
        if (read_count < 0)
        {
            return false;
        }

        if (read_count == 0)
        {
            end = true;
        }

        shift_r += read_count;
        return true;
    }

    bool client_write(int fd)
    {
        int write_count = write(fd, to_write.data(),
                shift_w);

        if (write_count < 0)
        {
            return false;
        }

        shift_w -= write_count;
        to_write.erase(to_write.begin(),
                to_write.begin() + write_count);
        resize();

        return true;

    }

    bool is_end()
    {
        return end && shift_w == 0;
    }

};


boost::optional<vector<char> > try_system(vector<char> &tmp, int& shift_r)
{
    int pos = parse(tmp);
    if (pos != -1)
    {
        vector<char> temp_buf(tmp.begin(),
                tmp.begin() + pos);
        temp_buf.push_back(0);
        tmp.erase(tmp.begin(),
                tmp.begin() + pos + 1);
        shift_r -= pos + 1;
        tmp.resize(sz);
        return boost::optional<vector<char> >(temp_buf);
    }
    return boost::optional<vector<char> >();

}

int main(int argc, char *argv[])
{
    pid = fork();
    if (pid == -1)
    {
        panic("Error on fork");
    }
    if (pid)
    {
        if (signal(SIGINT, h) == SIG_ERR)
        {
            panic("Signal");
        }
        wait(NULL);
        return 0;
    }
    setsid();
    if (signal(SIGINT, h) == SIG_ERR)
    {
        panic("Signal");
    }
    struct addrinfo hints, *res;
    struct sockaddr_storage sock_stor;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, argv[1], &hints, &res)) {//argv[1] - port
        panic("error getaddrinfo");
    }

    int sfd = socket(res->ai_family, res->ai_socktype, 
            res->ai_protocol);

    if (sfd == -1) {
        panic("error socket");
    }

    int optval = 1;

    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optval,
                sizeof(int)) == -1)    {
        panic("error setsockopt");
    }

    if (bind(sfd, res->ai_addr, res->ai_addrlen) == -1) {
        panic("error bind");
    }

    if (listen(sfd, 5) == -1) {
        panic("error listen");
    }
    freeaddrinfo(res);

    vector<client> clients(1, client());
    vector<struct pollfd> pfd(1);
    pfd[0].fd = sfd;
    pfd[0].events = POLLIN;

    while (true)
    {
        int sockfd;
        sockaddr_in clientc;

        clientc.sin_family = AF_INET;
        socklen_t addr_sz = sizeof(sock_stor);
        sockfd = accept(sfd,
                (struct sockaddr *) &clientc, &addr_sz);
        if (sockfd == -1)
            panic("error accept");
        int fork_val = fork();
        if (fork_val == 0)
        {
            vector<char> buffer(sz);
            int shift = 0;
            dup2(sockfd, 1);
            dup2(sockfd, 2);
            while (true) 
            {
                int read_count = read(sockfd, buffer.data() + shift, sz - shift);
                if (read_count < 0)
                {
                    panic("read");
                    return 0;
                }
                shift += read_count;
                if (read_count == 0)
                    return 0;
                boost::optional<vector<char> > tmp_vec =
                    try_system(buffer, shift);
                if (tmp_vec.is_initialized())
                {
                    if (system(tmp_vec.get().data()) == -1)
                    {
                        panic("system");
                    }
                }
            } 
        }   else 
            {
                sockets.push_back(fork_val);
            }
    }
}

