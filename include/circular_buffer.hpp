#pragma once
#ifndef _CIRCULAR_BUFFER_HPP_
#define _CIRCULAR_BUFFER_HPP_

#include <Arduino.h>

class CircularBuffer {
    char* buffer;
    size_t size;
    size_t chunk_size;
    size_t start = 0;
    size_t count = 0;

public:
    CircularBuffer(char* buffer, size_t buffer_size, size_t chunk_size);

    char* writeAddress();
    size_t writeMaxCount();
    void advanceWritten(size_t writeActualCount);

    bool chunkAvailable();
    char* advanceChunk();
    size_t chunkSize();

    void printInfo();
};

#endif // _CIRCULAR_BUFFER_HPP_