#ifndef COSMOS_H
#define COSMOS_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "packets.h"

class Cosmos {

    public:
        Cosmos(int port);
        ~Cosmos();
        void cosmosConnect();
        void cosmosDisconnect();
        void acceptConnection();
        int sendPacket(unsigned char* buffer, int size);
        int recvPacket(unsigned char* buffer, int size);

    private:
        int connectionSocket;
        int bindSocket;
        int port;
        unsigned int clilen;
        sockaddr_in serv_addr;
        sockaddr_in cli_addr;
};

#endif
