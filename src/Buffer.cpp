#include "Buffer.h"

Buffer::Buffer() {}

Buffer::~Buffer() {}

void Buffer::append(const char* _str, int _size) {
    const char* p = _str;
    int i = 0;
    while (*p != '\0' && i < _size) {
        buf.push_back(*p);
        p++; i++;
    }
}

ssize_t Buffer::size() { return buf.size(); }

const char* Buffer::c_str() { return buf.c_str(); }

void Buffer::clear() { buf.clear(); }
