#include "Buffer.h"

Buffer::Buffer() {}

Buffer::~Buffer() {}

void Buffer::append(const char* _str, int _size) {
    const char* p = _str;
    while (*p != '\0') {
        buf.push_back(*p);
        p++;
    }
}

ssize_t Buffer::size() { return buf.size(); }

const char* Buffer::c_str() { return buf.c_str(); }

void Buffer::clear() { buf.clear(); }
