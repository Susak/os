#include<unistd.h>
#include<stdlib.h>
#include<vector>
#include<sstream>
#include<string>
#include<sys/types.h>
#include<memory.h>
#include<sys/socket.h>
#include<netdb.h>
#include<poll.h>
#include<stdio.h>
#include<iostream>
#include<map>
#include<iterator>
#include<algorithm>
#include<arpa/inet.h>
#include<netinet/in.h>

struct multiheadqueue {
    std::vector<char> buffer;
    std::vector<int> pos;
    std::vector<int> ref;
    int size;
    multiheadqueue() {
        pos.push_back(0);
        int size = 0;
    }
    void add(std::string &a) {
        for (int i = 0; i < a.size(); i++) {
            buffer.push_back(a[i]);
            ref.push_back(pos.size());
        }
        size += a.size();
    }

    void update() {
        int min_i = -1;
        int min = -1;
        for (int i = 0; i < pos.size(); i++) {
            if (min > pos[i]) {
                min = pos[i];
                min_i = i;
            }
        }
        for (int i = 1; i < pos.size(); i++) {
            pos[i] -= min;
        }
        std::vector<char> tmp(buffer.size());
        tmp.insert(tmp.begin(), buffer.begin() + min_i,  buffer.end());
        tmp.swap(buffer);
    }
};
std::string get_str(int a) {
    std::string res;
    std::ostringstream convert;
    convert << a;
    return convert.str();
}
std::string parse_sr(std::vector<char> &a) {
    auto it = std::find(a.begin(), a.end(), ' ');
    it++;
    int num = atoi(std::string(a.begin(), it).data());
    std::cerr << num << "\n";
    std::vector<char> tmp(a.size());
    std::string res(it, it + num);
    res.push_back('\0');
    tmp.insert(tmp.begin(), it + num,  a.end());
    a.swap(tmp);
    return res;
}
std::vector<std::string > parse_buffer(std::vector<char> a,
        char delim, int sz) {
    a[sz - 1] = ' ';
    auto it = std::find(a.begin(), a.end(), delim);
    std::vector<std::string > res;
    while (it != a.end()) {
        res.push_back(std::string(a.begin(), it));
        it++;
        a.erase(a.begin(), it);
        it = std::find(a.begin(), a.end(), delim);
    }
    return res;
}
int main (int argc, char *argv[]) {
    if (argc < 2)
        _exit(1);
    if (fork() == 0) {
        setsid();
        int sz = 4096;
        std::vector<std::vector<char>> buffer(1);
        multiheadqueue queue;
        struct addrinfo hints, *res;
        struct sockaddr_storage sock_stor;
        memset(&hints, 0, sizeof(struct addrinfo));
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_PASSIVE;
        if (getaddrinfo(NULL, argv[1], &hints, &res)) {
        perror("error getaddrinfo");
        }
        int sfd = socket(res->ai_family, res->ai_socktype, 
                res->ai_protocol);
        if (sfd == -1) {
        perror("error socket");
        }
        int optval = 1;
        if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optval,
                    sizeof(int)) == -1)    {
            perror("error setsockopt");
        }

        if (bind(sfd, res->ai_addr, res->ai_addrlen) == -1) {
            perror("error bind");
        }
        if (listen(sfd, 5) == -1) {
            perror("error listen");
        }
        std::vector<struct pollfd> psfd(1);
        psfd[0].fd = sfd;
        psfd[0].events = POLLIN;
        std::vector<std::string> info(1);
        while (true) {
            int pol_val = poll(psfd.data(), psfd.size(), -1);
            if (pol_val > 0) {
                if (psfd[0].revents & POLLIN) {
                    int sockfd;
                    sockaddr_in client;
                    client.sin_family = AF_INET;
                    socklen_t addr_sz = sizeof(sock_stor);
                    info.push_back(get_str(client.sin_addr.s_addr));
                    info.back() += " " + get_str(client.sin_port) + " ";
                    sockfd = accept(sfd,
                            (struct sockaddr *) &client, &addr_sz);
                    struct pollfd temp_poll;
                    temp_poll.events = POLLIN | POLLOUT;
                    temp_poll.fd = sockfd;
                    psfd.push_back(temp_poll);
                    queue.pos.push_back(0);
                    buffer.push_back(std::vector<char>(sz));
                    std::cerr << "Connected " << sockfd << std::endl;
                }

                for (int i = 1; i < psfd.size(); i++) {
                    if (psfd[i].revents & POLLIN) {
                        int cur_read = 0;
                        int read_c = 1;
                        std::cerr << "write\n";
                            read_c= read(psfd[i].fd,buffer[i].data() +
                                    cur_read, sz - cur_read);
                            if (read_c < 0)
                                _exit(2);
                            cur_read += read_c;
                            std::string cur_mes = info[i] + 
                                parse_sr(buffer[i]);
                            std::cerr << cur_mes << std::endl;
                            queue.add(cur_mes);
                    }
                }

                for (int i = 1; i < psfd.size(); i++) {
                    if ((psfd[i].revents & POLLOUT) && 
                            queue.pos[i] < queue.buffer.size()) {
                        int cur_len = sz;
                        if (queue.buffer.size() - queue.pos.size() < sz) {
                            cur_len = queue.buffer.size()
                                - queue.pos[i];
                        }
                        int wr = write(psfd[i].fd, queue.buffer.data() +
                                queue.pos[i], cur_len);
                        write(psfd[i].fd, "\n", 1);
                        queue.pos[i] += wr;
                    }
                }
            }
        }
    }
    return 0;
}
