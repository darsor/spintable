#ifndef COSMOS_QUEUE_H
#define COSMOS_QUEUE_H

#include "cosmos.h"
#include "packets.h"
#include <atomic>
#include <queue>

class CosmosQueue {
    public:
        CosmosQueue(unsigned int port, unsigned int sendSize, unsigned int recvSize = 0);
        ~CosmosQueue();
        void push_tlm(Packet* tlm);
        bool  pop_cmd(Packet* &cmd);
        void deleteFrontTlm();
        void deleteFrontCmd();
        unsigned int tlmSize();
        unsigned int cmdSize();

        void connect();
        void disconnect();
        bool isConnected() { return connected; }
    private:
        Cosmos cosmos;
        std::atomic<bool> connected;

        std::queue<Packet*> tlmQueue;
        std::queue<Packet*> cmdQueue;
        unsigned int tlmCapacity;
        unsigned int cmdCapacity;

        bool pop_tlm();
        void push_cmd(Packet* cmd);

        void tlm_thread();
        void cmd_thread();
};
#endif
