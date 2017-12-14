#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>

#define PORT "3490" // the port client will be connecting to
#define MAXDATASIZE 1464 // max number of bytes we can get at once


const char clr[] = { 27, '[', '2', 'J', '\0' };
const char topLeft[] = { 27, '[', '1', ';', '1', 'H','\0' };


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa){
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
    int sockfd, numbytes;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    // get information of the server
    if ((rv = getaddrinfo("lab01", PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo); // all done with this structure

    //
    // while(1){
    //   send(sockfd, "Hello, world! from client", 100, 0);
    // }
    // CHARA begin


    for(int i=0; i<100; i++){
                char *data;
                data = (char *)malloc(MAXDATASIZE * sizeof(char));
                memset( data, '*', MAXDATASIZE * sizeof(char) );

                send(sockfd, data, MAXDATASIZE, 0);
                if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
                    perror("recv");
                    exit(1);
                }

                printf("%ld\n",strlen(buf));
              	/* Clear screen and move to top left */
              	// printf("%s%s", clr, topLeft);
                // printf("\nTCP Pingpong Client ====================================");
                // printf("\nStatistics for port  ------------------------------"
                //      "\nPackets send: %ld"
            		// 	   "\nPackets received: %ld"
                //      "\n %s"
                //      ,strlen(data)
                //      ,strlen(buf)
            		// 	   ,buf);
                // printf("\n====================================================\n");
    }

    close(sockfd);
    return 0;
}
