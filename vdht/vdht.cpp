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
        char delim) {
    a.push_back(' ');
    auto it = std::find(a.begin(), a.end(), delim);
    std::vector<std::string > res;
    while (it != a.end()) {
        it++;
        res.push_back(std::string(a.begin(), it));
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
    std::vector<char> buffer(4096);
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

   struct addrinfo hints;
   struct addrinfo *result, *rp;
   int sfd, s;
   struct sockaddr_storage peer_addr;
   socklen_t peer_addr_len;
   ssize_t nread;

   memset(&hints, 0, sizeof(struct addrinfo));
   hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
   hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
   hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
   hints.ai_protocol = 0;          /* Any protocol */
   hints.ai_canonname = NULL;
   hints.ai_addr = NULL;
   hints.ai_next = NULL;

   s = getaddrinfo(NULL, argv[1], &hints, &result);
   if (s != 0) {
       fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
       exit(EXIT_FAILURE);
   }

        sfd = socket(result->ai_family, result->ai_socktype,
                result->ai_protocol);
        if (sfd == -1) {
            _exit(2);
        }
          
    std::vector<struct pollfd> pfd(my_clients.size() + 2);
    int poll_init = POLLIN | POLLERR;
    pfd[0].fd = sfd;
    pfd[0].events = poll_init;
    pfd[1].events = poll_init;
    pfd[1].fd = 0;
    if (rp == NULL) {
        std::cerr << "Could not bind" << std::endl;
        exit(EXIT_FAILURE);
    }
    int opt = 1;
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt,
                sizeof(int)) != 0) {
        std::cerr << "Could not set socket optins" << std::endl;
        exit(EXIT_FAILURE);
    }
    if (bind(sfd, result->ai_addr, result->ai_addrlen) == -1) {
        std::cout << "couldnt bind";
        _exit(6);
    }
    /*if (listen(sfd, 10) == -1) {
        std::cerr << "Could not listen " << sfd << std::endl;
        exit(EXIT_FAILURE);
    }*/
    
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
                read_c = read(0, buffer.data() + cur_read,
                            buffer.size() - cur_read);
                cur_read += read_c;
                std::vector<std::string> command = parse_buffer(buffer, ' ');
                std::cout << command[0] << "FFFFFF ";

                if (command[0] == "print") {
                    int num = atoi(command[1].data());
                    for (int i = 0; i < history[num].size(); i++) {
                        write(1, history[num][i].data(),
                                history[num][i].size());
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
                            write(1, "collision", 9);
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
                    while (read_c > 0) {
                        read_c = read(pfd[i].fd, buffer.data() + cur_read,
                                buffer.size() - cur_read);
                        cur_read += read_c;
                        if (read_c < 0) {
                            _exit(7);
                        }
                    }
                    std::vector<std::string> command = parse_buffer(buffer, ' ');
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
                                write(1, "collision", 9);
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
