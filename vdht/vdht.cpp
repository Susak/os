#include<unistd.h>
#include<stdlib.h>
#include<vector>
#include<string>
#include<sys/types.h>
#include<memory.h>
#include<sys/socket.h>
#include<netdb.h>
#include<poll.h>
#include<stdio.h>
#include<iostream>
#include<map>
#include<algorithm>

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
    if (argc < 3)
        _exit(1);
    std::map<int, std::vector<std::string> > history;
    std::vector<int> my_clients;
    for (int i = 2; i < argc; i++) {
           struct addrinfo hints;
           struct addrinfo *result, *rp;
           int sfd, s, j;
           size_t len;
           ssize_t nread;
       
           memset(&hints, 0, sizeof(struct addrinfo));
           hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
           hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
           hints.ai_flags = 0;
           hints.ai_protocol = 0;          /* Any protocol */

           s = getaddrinfo(NULL, argv[i], &hints, &result);
           if (s != 0) {
               fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
               exit(EXIT_FAILURE);
           }

           for (rp = result; rp != NULL; rp = rp->ai_next) {
               sfd = socket(rp->ai_family, rp->ai_socktype,
                            rp->ai_protocol);
               if (sfd == -1)
                   continue;
              if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1) {
                  my_clients.push_back(sfd);
              }

           }


           freeaddrinfo(result); 

    }

    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if (getaddrinfo(NULL, argv[1], &hints, &res)) {
        perror("error getaddrinfo");
    }
    int sfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sfd == -1) {
        perror("error socket");
    }
    int optval = 1;
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) == -1)    {
        perror("error setsockopt");
    }

    if (bind(sfd, res->ai_addr, res->ai_addrlen) == -1) {
        perror("error bind");
    }
    if (listen(sfd, 5) == -1) {
        perror("error listen");
    }
    std::vector<struct pollfd> pfd(my_clients.size() + 2);
    int poll_init = POLLIN | POLLERR;
    pfd[0].fd = sfd;
    pfd[0].events = poll_init;
    pfd[1].events = poll_init;
    pfd[1].fd = 0;
    for (int i = 2; i < pfd.size(); i++) {
        pfd[i].fd = my_clients[i];
        pfd[i].events = poll_init | POLLOUT;
    }
 
    while (true) {
        int poll_val = poll(pfd.data(), pfd.size(), -1);
        if (poll_val > 0) {
            if (pfd[0].revents & POLLERR) {
                _exit(2);
            }
            if (pfd[0].revents & POLLIN) {
                int sockfd;
                sockfd = accept(sfd, NULL, NULL);
                struct pollfd temp_poll;
                temp_poll.fd = sockfd;
                temp_poll.events = poll_init | POLLOUT;
                pfd.push_back(temp_poll);
                my_clients.push_back(sockfd);
            }
            if (pfd[1].revents & POLLERR) {
                _exit(2);
            }
            if (pfd[1].revents & POLLIN) {
                int read_c = 1;
                int cur_read = 0;
                std::vector<char> buffer(4096);
                read_c = read(0, buffer.data() + cur_read,
                            buffer.size() - cur_read);
                cur_read += read_c;
                std::vector<std::string> command = 
                    parse_buffer(buffer, ' ', cur_read);
                if (command[0] == "print") {
                    int num = atoi(command[1].data());
                    for (int i = 0; i < history[num].size(); i++) {
                        write(2, history[num][i].data(),
                                history[num][i].size());
                        std::cerr << std::endl;
                    }
                }
                if (command[0] == "change") {
                    int num = atoi(command[1].data());
                    if (history[num].back()
                            != command[3]) {
                        if (history[num].back()
                                == command[2]) {
                            history[num].push_back(command[3]);
                            for (int i = 2; i < pfd.size(); i++) {
                                if (pfd[i].revents & POLLOUT) {
                                    write(pfd[i].fd, buffer.data(),
                                            buffer.size());
                                }
                            }
                        } else {
                            write(2, "collision", 9);
                            std::cerr << std::endl;
                        }
                    }
                }

                if (command[0] == "add") {
                   if (history.count(atoi(command[1].data())) == 0) {
                       std::vector<std::string> tmp;
                       tmp.push_back(command[2]);
                        history.insert(std::make_pair(atoi(command[1].data()),
                                    tmp));
                        for (int i = 2; i < pfd.size(); i++) {
                            if (pfd[i].revents & POLLOUT) {
                                write(pfd[i].fd, buffer.data(),
                                        buffer.size());
                            }
                        }
                    } 
                }
            }

            for (int i = 2; i < pfd.size(); i++) {
                if (pfd[i].revents & POLLERR) {
                    _exit(2);
                }
                if (pfd[i].revents & POLLIN) {
                    int read_c = 1;
                    int cur_read = 0;
                    std::vector<char> buffer(4096);
                    while (read_c > 0) {
                        read_c = read(pfd[i].fd, buffer.data() + cur_read,
                                buffer.size() - cur_read);
                        cur_read += read_c;
                        if (read_c < 0) {
                            _exit(7);
                        }
                    }
                    std::vector<std::string> command = 
                        parse_buffer(buffer, ' ', cur_read);
                    if (command[0] == "change") {
                        int num = atoi(command[1].data());
                        if (history[num][history[num].size() - 1]
                                != command[3]) {
                            if (history[num].back()
                                    == command[2]) {
                                history[num].push_back(command[3]);
                                for (int i = 2; i < pfd.size(); i++) {
                                    if (pfd[i].revents & POLLOUT) {
                                        write(pfd[i].fd, buffer.data(),
                                                buffer.size());
                                    }
                                }
                            } else {
                                write(2, "collision", 9);
                                std::cerr << std::endl;
                            }
                        }
                    }

                    if (command[0] == "add") {
                       if (history.count(atoi(command[1].data())) == 0) {
                           std::vector<std::string> tmp;
                           tmp.push_back(command[2]);
                            history.insert(std::make_pair(atoi(command[1].data()),
                                        tmp));
                            for (int i = 2; i < pfd.size(); i++) {
                                if (pfd[i].revents & POLLOUT) {
                                    write(pfd[i].fd, buffer.data(),
                                            buffer.size());
                                }
                            }
                        } 
                    }
                }
            }
        }
    }
    return 0;
}
