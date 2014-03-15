#include <sys/types.h>
#include <wait.h>
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
int pid;
void h(int signal)
{
    if (pid) {
        kill(pid, SIGINT);
    }
}

struct client {
    vector<char> to_read;
    vector<char> to_write;
    int shift_r;
    int shift_w;
    struct pollfd poll;

    client(int fd) : to_read(sz), to_write(sz), shift_r(0), shift_w(0) {
        poll.fd = fd;
        poll.events = POLLIN | POLLRDHUP; 
    }
};

int main(int argc, char *argv[])
{
    pid = fork();
    if (pid)
    {
        signal(SIGINT, h);
        wait(NULL);
        return 0;
    }
    setsid();
    vector<vector<char> > to_read(1, vector<char>(sz)); 
    vector<vector<char> > to_write(1, vector<char>(sz));
    vector<int> shift_r(1, 0);
    vector<int> shift_w(1, 0);
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
    std::vector<struct pollfd> pfd(1);
    pfd[0].fd = sfd;
    pfd[0].events = POLLIN;

    while (true)
    {
        int pol_val = poll(pfd.data(), pfd.size(), -1);
        if (pol_val > 0)
        {
            if (pfd[0].revents & POLLIN)
            {
                int sockfd;
                sockaddr_in client;

                client.sin_family = AF_INET;
                socklen_t addr_sz = sizeof(sock_stor);
                sockfd = accept(sfd,
                        (struct sockaddr *) &client, &addr_sz);
                if (sockfd == -1)
                    panic("error accept");
                struct pollfd temp_poll;
                temp_poll.events = POLLIN | POLLRDHUP;
                temp_poll.fd = sockfd;            
                pfd.push_back(temp_poll);
                to_read.push_back(vector<char>(sz));
                to_write.push_back(vector<char>(sz));
                shift_w.push_back(0);
                shift_r.push_back(0);

                std::cout << "Client accepted!" << std::endl;
            }
            for (int i = 1; i < pfd.size(); i++)
            {
                if ((pfd[i].revents & POLLIN) ||
                        (pfd[i].revents & POLLRDHUP))
                {
                    if (pfd[i].revents & POLLRDHUP) {
                        shift_w.erase( shift_w.begin() + i);
                        pfd.erase(pfd.begin() + i);
                        shift_r.erase(shift_r.begin() + i);
                        to_read.erase(to_read.begin() + i);
                        to_write.erase(to_write.begin() + i);
                    }
                    cout << "Client " << i << "something write\n";
                    int read_count = read(pfd[i].fd,
                            to_read[i].data() + shift_r[i],
                            to_read[i].size() - shift_r[i]);
                    if (read_count < 0)
                    {
                        shift_w.erase( shift_w.begin() + i);
                        pfd.erase(pfd.begin() + i);
                        shift_r.erase(shift_r.begin() + i);
                        to_read.erase(to_read.begin() + i);
                        to_write.erase(to_write.begin() + i);
                        continue;
                    }
                    shift_r[i] += read_count;
                }
                if (shift_w[i] > 0) {
                    pfd[i].events |= POLLOUT;
                }
                if (pfd[i].revents & POLLOUT)
                {
                    int write_count = write(pfd[i].fd,
                            to_write[i].data(), shift_w[i]);

                    if (write_count < 0)
                    {
                        pfd.erase(pfd.begin() + i);
                        shift_r.erase(shift_r.begin() + i);
                        shift_w.erase(shift_w.begin() + i);

                        to_read.erase(to_read.begin() + i);
                        to_write.erase(to_write.begin() + i);
                        continue;
                    }

                    shift_w[i] -= write_count;
                    to_write[i].erase(to_write[i].begin(), to_write[i].begin() + write_count);
                    to_write[i].resize(sz);
                }

                if (shift_w[i] == 0) {
                    pfd[i].events |= ~POLLOUT;
                }
                copy(to_read[i].begin(), to_read[i].begin() + shift_r[i],
                        back_inserter(to_write[i]));
                to_read[i].erase(to_read[i].begin(), to_read[i].begin() + 
                        shift_r[i]);
                to_read[i].resize(sz);
                
                shift_w[i] += shift_r[i];
                /*int pos = parse(to_read[i]);
                if (pos != -1)
                {
                    vector<char> tmp;
                    shift_r[i] -= pos;
                    copy(to_read[i].begin(), to_read[i].begin() + pos,
                            back_inserter(tmp));
                    to_read[i].erase(to_read[i].begin(),
                            to_read[i].begin() + pos  + 1);
                    to_read[i].resize(sz);
                    system(tmp.data());
                    vector<char> temp(sz);
                    int count = 0;
                    while (true) {
                        int read_count = read(1, temp.data() + count,
                                 tmp.size() - count);
                        count += read_count;
                        if (read_count == 0) break;
                        if (read_count < 0) {
                            panic("read system result fail");
                        }
                    }
                    copy(temp.begin(), temp.begin() + count,
                            back_inserter(to_write[i]));
                    to_write[i].push_back('0');
                    shift_w[i] += count + 1;
                }*/
            }
        }
    }
}
