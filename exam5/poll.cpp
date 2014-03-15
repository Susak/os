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

const int sz = 50;

void panic(const char * msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

/*void safe_write(int fd, vector<float> &buf, int read_cout)
{
    int written = 0;

    //cout << "here" << buf.size() << endl;
    while(written < read_cout)
    {
        int write_count = write(fd, buf.data() + written, read_cout - written);
        if (write_count < 0)
            panic("Error in write");
        written += write_count;
    } 
}*/
void safe_write(int fd, vector<float> &buf, int read_cout)
{
    int written = 0;

    for (int i = 0; i < buf.size(); i++)
    {
        cout << buf[i];
    }
    cout << endl;
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
       if (kill(pid, SIGINT) == -1)
       {
            panic("kill");
       }
    }
}
struct client {
    vector<float> to_read;
    int shift_r;

    client() : to_read(sz, 0.), shift_r(0) {}

    void resize() {
        to_read.resize(sz);
    }

    bool client_read(int fd)
    {
        int read_count = read(fd,
                to_read.data() + shift_r,
                (to_read.size() - shift_r) * 4);
        if (read_count < 0)
        {
            return false;
        }

        if (read_count == 0)
        {
            return false;
        }

        shift_r += read_count / 4;
        return true;
    }

    void push() 
    {
        for (int i = shift_r; i < sz; i++)
        {
            to_read[i] = 0.;
        }
        shift_r = sz;
    }

    void clr()
    {
        to_read.assign(sz, 0.);
        shift_r = 0;
    }

};

vector<float> add(vector<client> & clients) 
{
    for (int i = 0; i < clients.size(); i++)
        clients[i].push();
    vector<float> res(50, 0.0);
    for (int i = 0; i < sz; i++)
    {
        for (int j = 0; j <clients.size(); j++)
        {
            res[i] += clients[j].to_read[i];
        }
    }
    for (int i = 0; i < clients.size(); i++)
    {
        clients[i].clr();
    }
    res.push_back(0);
    return res;
}
bool check(vector<float> & v)
{
    for (int i = 0; i < v.size(); i++)
    {
        if (v[i] != 0)
            return true;
    }
    return false;
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
            panic("signal");
        }
        if (wait(NULL) == -1)
        {
            panic("wait");
        }
        return 0;
    }
    if (setsid() == -1) 
    {
        panic("setsid");
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

    vector<client> clients(2, client());
    vector<struct pollfd> pfd(2);
    pfd[0].fd = sfd;
    pfd[0].events = POLLIN | POLLRDHUP;
    pfd[1].fd = 1;
    pfd[1].events = POLLOUT | POLLRDHUP;

    while (true)
    {
        int pol_val = poll(pfd.data(), pfd.size(), 5000);
        if (pol_val == -1)
        {
            panic("Error in poll");
        }
        if (pol_val > 0 || pol_val == 0)
        {
            if (pfd[0].revents & POLLIN || pfd[0].revents && POLLRDHUP)
            {
                if (pfd[0].revents & POLLRDHUP)
                {
                    panic("pollrdhup");
                }
                int sockfd;
                sockaddr_in clientc;

                clientc.sin_family = AF_INET;
                socklen_t addr_sz = sizeof(sock_stor);
                sockfd = accept(sfd,
                        (struct sockaddr *) &clientc, &addr_sz);
                if (sockfd == -1)
                    panic("error accept");
                struct pollfd temp_poll;
                temp_poll.events = POLLIN | POLLRDHUP | POLLOUT;
                temp_poll.fd = sockfd;            
                pfd.push_back(temp_poll);
                clients.push_back(client());
                std::cout << "Client accepted!" << std::endl;
            }

            if (pfd[1].revents & POLLOUT || pfd[1].revents & POLLRDHUP)
            {
                if (pfd[1].revents & POLLRDHUP) {
                    panic("pollrdhup");
                }
                vector<float> res = add(clients);
                /*if (!check(res)) {
                    string r(50, '0');
                    cout << endl << r.data();
                    sleep(3);
                } else {
                    safe_write(1, res, sz * 4);
                }*/
                safe_write(1, res, sz * 50);
                sleep(3);

            }
            for (int i = 2; i < pfd.size(); i++)
            {
                client &tmp = clients[i];
                if ((pfd[i].revents & POLLIN) ||
                        (pfd[i].revents & POLLRDHUP))
                {
                    if (pfd[i].revents & POLLRDHUP)
                    {
                        pfd.erase(pfd.begin() + i);
                        clients.erase(clients.begin() + i);
                        continue;
                    }
                    cout << "Client " << i << "something write\n";
                    
                    if (!tmp.client_read(pfd[i].fd))
                    {
                        pfd.erase(pfd.begin() + i);
                        clients.erase(clients.begin() + i);
                        continue;
                    }
                }
            }
        }
    }
}
