#include<sys/socket.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<netdb.h>
#include<unistd.h>
#include<iostream>
int main () {
    struct addrinfo hints;
    struct addrinfo *result;
    int sfd, s;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
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
    sockaddr saddr;
    socklen_t addrlen = sizeof(saddr);
    sockfd = accept(sfd, &saddr, &addrlen);
    if (sockfd == -1) {
        std::cerr << "Could not accept" << std::endl;
        exit(EXIT_FAILURE);
    }
    pid_t fork_val = fork();
    if (fork_val) {// parent
        close(sockfd);
    } else {// child
        close(sockfd);
        dup2(sockfd, 0);
        dup2(sockfd, 1);
        dup2(sockfd, 2);
        std::cout << "OK" << std::endl;
    }
    return 0; 
}
