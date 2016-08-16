#include "cosmosQueue.h"
#include <string.h> // for memcpy()
#include <unistd.h>
#include <thread>
#include <mutex>

std::mutex tlm_mutex;
std::mutex cmd_mutex;
std::mutex connection_mutex;

CosmosQueue::CosmosQueue(unsigned int port, unsigned int sendSize, unsigned int recvSize):
    cosmos(port), tlmQueue(), cmdQueue() {
        connected.store(false);
        tlmCapacity = sendSize;
        cmdCapacity = recvSize;
        if (tlmCapacity > 0) {
            std::thread th1(&CosmosQueue::tlm_thread, this);
            th1.detach();
        }
        if (cmdCapacity > 0) {
            std::thread th2(&CosmosQueue::cmd_thread, this);
            th2.detach();
        }
    }

CosmosQueue::~CosmosQueue() {
    while (!tlmQueue.empty()) {
        deleteFrontTlm();
    }
    while (!cmdQueue.empty()) {
        deleteFrontCmd();
    }
    tlmCapacity = 0;
    cmdCapacity = 0;
}

void CosmosQueue::deleteFrontTlm() {
    if (tlmQueue.empty()) return;
    delete tlmQueue.front();
    tlmQueue.pop();
}

void CosmosQueue::deleteFrontCmd() {
    if (cmdQueue.empty()) return;
    delete cmdQueue.front();
    cmdQueue.pop();
}

void CosmosQueue::push_tlm(Packet* item) {
    if (tlmCapacity == 0) {
        delete item;
        return;
    }
    tlm_mutex.lock();
    if (tlmQueue.size() >= tlmCapacity) {
        //printf("deleting packet from full queue\n");
        deleteFrontTlm();
    }
    //printf("pushing packet with id %d\n", item->id);
    tlmQueue.push(item);
    tlm_mutex.unlock();
}

bool CosmosQueue::pop_tlm() {
    tlm_mutex.lock();
    //printf("trying to pop from queue of size %d\n", tlmQueue.size());
    if (tlmCapacity == 0 || tlmQueue.empty() || !connected.load()) {
        tlm_mutex.unlock();
        return false;
    }
    Packet* temp = tlmQueue.front();
    tlmQueue.pop();
    tlm_mutex.unlock();
    temp->convert();
    if (cosmos.sendPacket(temp->buffer, temp->length) <= 0) {
        connected.store(false);
        delete temp;
        return false;
    }
    delete temp;
    return true;
}

void CosmosQueue::push_cmd(Packet* cmd) {
    if (cmdCapacity == 0) return;
    cmd_mutex.lock();
    if (cmdQueue.size() >= cmdCapacity) {
        deleteFrontCmd();
    }
    cmdQueue.push(cmd);
    cmd_mutex.unlock();
}

bool CosmosQueue::pop_cmd(Packet* &cmd) {
    cmd_mutex.lock();
    if (cmdCapacity == 0 || cmdQueue.empty()) {
        cmd_mutex.unlock();
        return false;
    }
    cmd = cmdQueue.front();
    cmdQueue.pop();
    cmd_mutex.unlock();
    return true;
}

unsigned int CosmosQueue::tlmSize() {
    return tlmQueue.size();
}

unsigned int CosmosQueue::cmdSize() {
    return cmdQueue.size();
}

uint16_t CosmosQueue::cmd_front_id() {
    uint16_t temp;
    cmd_mutex.lock();
    if (cmdSize() > 0) temp = cmdQueue.front()->id;
    else temp = -1;
    cmd_mutex.unlock();
    return temp;
}

void CosmosQueue::connect() {
    cosmos.acceptConnection();
    connected.store(true);
}

void CosmosQueue::disconnect() {
    cosmos.cosmosDisconnect();
}

void CosmosQueue::tlm_thread() {
    while (true) {
        connection_mutex.lock();
        if (!connected.load()) connect();
        connection_mutex.unlock();
        while (connected.load()) {
            while (pop_tlm());
            //printf("emptied queue\n");
            usleep(10000);
        }
        printf("telemetry connection with COSMOS lost\n");
    }
}

void CosmosQueue::cmd_thread() {
    unsigned char buffer[128];
    uint32_t length;
    uint8_t id;
    Packet* cmd;
    while (true) {
        connection_mutex.lock();
        if (!connected.load()) connect();
        connection_mutex.unlock();
        while (connected.load()) {
            // receive the first four bytes (the length of the packet);
            if (cosmos.recvPacket(buffer, 4) < 4) {
                printf("unable to get 4 bytes\n");
                connected.store(false);
                break;
            } else {
                memcpy(&length, buffer, sizeof(length));
                printf("received packet of length %u\n", length);
                if (length < 5 || length > 100) {
                    printf("packet is too small. Disconnecting...\n");
                    connected.store(false);
                    break;
                }
                // receive the rest of the packet
                if (cosmos.recvPacket(buffer+4, length-4) < (int)(length-4)) {
                    printf("failed to fetch entire packet. Disconnecting...\n");
                    connected.store(false);
                    break;
                }
                // get the id to know what command it is
                memcpy(&id, buffer+4, sizeof(id));
                printf("id is %d\n", id);
                cmd = new Packet(length, id, true);
                memcpy(cmd->buffer, buffer, length);
                push_cmd(cmd);
            }
        }
        printf("command connection with COSMOS lost\n");
    }
}
