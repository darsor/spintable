#include "cosmosQueue.h"
#include <mutex>

std::mutex push_mutex;
std::mutex pop_mutex;

CosmosQueue::CosmosQueue(const unsigned int size):
    cosmos(8321), queue() {
        connected = false;
        capacity = size;
    }

CosmosQueue::~CosmosQueue() {
    while (!queue.empty()) {
        deleteFront();
    }
    capacity = 0;
}

void CosmosQueue::deleteFront() {
    delete queue.front();
    queue.pop();
}

void CosmosQueue::push(Packet* item) {
    push_mutex.lock();
    if (queue.size() >= capacity) {
        deleteFront();
    }
    queue.push(item);
    push_mutex.unlock();
}

bool CosmosQueue::pop() {
    pop_mutex.lock();
    static char* buffer = NULL;
    static unsigned int bufferSize = 0;
    if (queue.empty()) {
        pop_mutex.unlock();
        return false;
    }
    switch (queue.front()->id) {
        case 1:
            buffer = timeBuffer;
            bufferSize = TIME_PKT_SIZE;
            break;
        case 2:
            buffer = sensorBuffer;
            bufferSize = SENSOR_PKT_SIZE;
            break;
        case 3:
            buffer = cameraBuffer;
            bufferSize = CAM_PKT_SIZE;
            break;
        case 4:
            buffer = encoderBuffer;
            bufferSize = ENC_PKT_SIZE;
            break;
    }
    queue.front()->convert(buffer);
    push_mutex.lock();
    deleteFront();
    push_mutex.unlock();
    pop_mutex.unlock();
    if (cosmos.sendPacket(buffer, bufferSize) != 0) {
        connected = false;
        return false;
    }
    return true;
}

unsigned int CosmosQueue::size() {
    return queue.size();
}

void CosmosQueue::connect() {
    cosmos.acceptConnection();
    connected = true;
}

void CosmosQueue::disconnect() {
    cosmos.cosmosDisconnect();
}

int CosmosQueue::recv(char* buffer, int size) {
    if (cosmos.recvPacket(buffer, size) < 0) {
        connected = false;
        return -1;
    } else return 0;
}
