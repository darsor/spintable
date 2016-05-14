#ifndef COSMOS_QUEUE_H
#define COSMOS_QUEUE_H

#include "cosmos.h"
#include <mutex>

std::mutex push_mutex;
std::mutex pop_mutex;

template <typename T>
class CosmosQueue {
    public:
        CosmosQueue(const unsigned int size);
        ~CosmosQueue();
        void push(const T& item);
        bool pop();
        unsigned int size();
        void connect();
        void disconnect();
        bool isConnected() { return connected; }
    private:
        Cosmos cosmos;
        bool connected;
        unsigned int wrap(unsigned int& value);
        unsigned int currentSize;
        unsigned int capacity;
        unsigned int front;
        unsigned int back;
        char timeBuffer[TIME_PKT_SIZE];
        char sensorBuffer[SENSOR_PKT_SIZE];
        char cameraBuffer[CAM_PKT_SIZE];
        char encoderBuffer[ENC_PKT_SIZE];
        T* array;
};

template <typename T>
CosmosQueue<T>::CosmosQueue(const unsigned int size):
    cosmos(8321) {
        connected = false;
        array = new T[size];
        for (unsigned int i=0; i<size; i++) {
            array[i] = NULL;
        }
        currentSize = 0;
        capacity = size;
        front = back = 0;
    }

template <typename T>
CosmosQueue<T>::~CosmosQueue() {
    for (unsigned int i=0; i<capacity; i++) {
        if (array[i] != NULL) {
            delete array[i];
            array[i] = NULL;
        }
    }
    delete[] array;
    array = NULL;
    currentSize = 0;
    capacity = 0;
    front = back = 0;
}

template <typename T>
unsigned int CosmosQueue<T>::wrap(unsigned int& value) {
    if (value < 0) value += capacity;
    else if (value >= capacity) value -= capacity;
    return value;
}

// TODO: test this
template <typename T>
void CosmosQueue<T>::push(const T& item) {
    push_mutex.lock();
    if (front == back && currentSize > 0) {
        delete array[back];
        array[back] = NULL;
        wrap(++front);
    } else {
        currentSize++;
    }
    array[back] = item;
    wrap(++back);
    push_mutex.unlock();
}

// TODO: test this
template <typename T>
bool CosmosQueue<T>::pop() {
    static char* buffer = NULL;
    static unsigned int bufferSize = 0;
    pop_mutex.lock();
    if (currentSize == 0 || array[front] == NULL) {
        pop_mutex.unlock();
        return false;
    }
    switch (array[front]->id) {
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
    array[front]->convert(buffer);
    push_mutex.lock();
    delete array[front];
    array[front] = NULL;
    wrap(++front);
    currentSize--;
    push_mutex.unlock();
    pop_mutex.unlock();
    if (cosmos.sendPacket(buffer, bufferSize) != 0) {
        connected = false;
        return false;
    }
    return true;
}

template <typename T>
unsigned int CosmosQueue<T>::size() {
    return currentSize;
}

template <typename T>
void CosmosQueue<T>::connect() {
    cosmos.acceptConnection();
    connected = true;
}

template <typename T>
void CosmosQueue<T>::disconnect() {
    cosmos.cosmosDisconnect();
}

#endif
