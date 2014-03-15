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

int main (int argc, char *argv[]) {
   struct addrinfo hints;
   struct addrinfo *result;
   int sfd, s;

   memset(&hints, 0, sizeof(struct addrinfo));
   hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
   hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
   hints.ai_flags = 0;
   hints.ai_protocol = 0;          /* Any protocol */

   s = getaddrinfo(NULL, argv[1], &hints, &result);
   if (s != 0) {
       fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
       exit(EXIT_FAILURE);
   }

   sfd = socket(result->ai_family, result->ai_socktype,
                result->ai_protocol);
   if (sfd == -1) {
        perror("error socket");
   }
   if (connect(sfd, result->ai_addr, result->ai_addrlen) == -1) {
       perror("error connect");
   }
   write(sfd, "hello", 5);

   freeaddrinfo(result); 
   return 0;
}
