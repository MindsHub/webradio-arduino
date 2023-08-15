#include "circular_buffer.hpp"

CircularBuffer::CircularBuffer(char *buffer_, size_t buffer_size_, size_t chunk_size_)
    : buffer{buffer_}, size{buffer_size_}, chunk_size{chunk_size_} {
    if (buffer_size_ % chunk_size_ != 0) {
        Serial.print("Invalid chunk size ");
        Serial.print(chunk_size_);
        Serial.print(" does not divide buffer size ");
        Serial.println(buffer_size_);
        assert(false);
    }
}


char *CircularBuffer::writeAddress() {
    return buffer + ((start + count) % size);
}

size_t CircularBuffer::writeMaxCount() {
    if (size - count - start >= 0) {
        // start is before end, so the remaining space is split in two by the array boundary
        return size - count - start;
    } else {
        // start is after end, we can fill up the whole remaining space at once
        return size - count;
    }
}

void CircularBuffer::advanceWritten(size_t writeActualCount) {
    count += writeActualCount;
    if (count > size) {
        Serial.print("Count ");
        Serial.print(count);
        Serial.print(" > size after writeActualCount=");
        Serial.println(writeActualCount);
        assert(false);
    }
}


bool CircularBuffer::chunkAvailable() {
    return count >= chunk_size;
}

char* CircularBuffer::advanceChunk() {
    if (count < chunk_size) {
        Serial.print("Count ");
        Serial.print(count);
        Serial.println(" < chunk_size in advanceChunk");
        assert(false);
    }

    // chunk_size divides buf_size, so we don't need to check for circular buffer bounds
    const char* result = buffer + start;
    count -= chunk_size;
    start += chunk_size;
    start %= size;
}

size_t CircularBuffer::chunkSize() {
    return chunk_size;
}


void CircularBuffer::printInfo() {
    Serial.print("Circular buffer usage: ");
    Serial.print(count);
    Serial.print("/");
    Serial.println(size);
}
