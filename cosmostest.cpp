#include <stdio.h>
#include <csignal>
#include <cerrno>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

struct data {
    uint16_t length;
    uint16_t id;
    uint32_t test_data;
}test;

void htonData(struct data d, char buffer[sizeof(struct data)]) {
    uint16_t u16;
    uint32_t u32;
    u16 = htons(d.length);
    memcpy(buffer+0, &u16, 2);
    u16 = htons(d.id);
    memcpy(buffer+2, &u16, 2);
    u32 = htonl(d.test_data);
    memcpy(buffer+4, &u32, 4);
}

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main()
{
     int sockfd, newsockfd, portno;
     socklen_t clilen;
     char buffer[sizeof(struct data)];
     struct sockaddr_in serv_addr, cli_addr;
     int n;

     // create a socket
     // socket(int domain, int type, int protocol)
     sockfd =  socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");

     // clear address structure
     bzero((char *) &serv_addr, sizeof(serv_addr));

     portno = 8321;

     /* setup the host_addr structure for use in bind call */
     // server byte order
     serv_addr.sin_family = AF_INET;  

     // automatically be filled with current host's IP address
     serv_addr.sin_addr.s_addr = INADDR_ANY;  

     // convert short integer value for port must be converted into network byte order
     serv_addr.sin_port = htons(portno);

     // bind(int fd, struct sockaddr *local_addr, socklen_t addr_length)
     // bind() passes file descriptor, the address structure, 
     // and the length of the address structure
     // This bind() call will bind  the socket to the current IP address on port, portno
     printf("binding socket\n");
     if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
              error("ERROR on binding");

     // This listen() call tells the socket to listen to the incoming connections.
     // The listen() function places all incoming connection into a backlog queue
     // until accept() call accepts the connection.
     // Here, we set the maximum size for the backlog queue to 5.
     printf("listening on socket\n");
     listen(sockfd,2);

     // The accept() call actually accepts an incoming connection
     clilen = sizeof(cli_addr);

     // This accept() function will write the connecting client's address info 
     // into the the address structure and the size of that structure is clilen.
     // The accept() returns a new socket file descriptor for the accepted connection.
     // So, the original socket file descriptor can continue to be used 
     // for accepting new connections while the new socker file descriptor is used for
     // communicating with the connected client.
     newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
     if (newsockfd < 0) 
          error("ERROR on accept");

     printf("server: got connection from %s port %d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port));

     bzero(buffer,sizeof(buffer));
     struct data test;
     test.length = sizeof(buffer);
     test.id = 3;
     signal(SIGPIPE, SIG_IGN); // ignore sigpipe signal - don't stop program when writing to closed socket

     printf("size of buffer: %d\n", sizeof(buffer));
     while(true) {
         test.test_data = (uint32_t) (rand() % 99 + 1);
         printf("sending data: %d\n", test.test_data);
         htonData(test, buffer); // format the data into a buffer

         // This send() function sends the bytes of the packet to the new socket
         if (send(newsockfd, buffer, sizeof(buffer), 0) < 0) {
             perror("The following error occured");
             break;
         }

         bzero(buffer,sizeof(buffer));
         usleep(20000);
     }

     printf("closing program\n");
     close(newsockfd);
     close(sockfd);
     return 0; 
}
