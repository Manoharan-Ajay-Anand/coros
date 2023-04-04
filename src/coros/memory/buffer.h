#ifndef COROS_MEMORY_BUFFER_H
#define COROS_MEMORY_BUFFER_H

#include <cstddef>

namespace coros::base {
    struct IOChunk {
        std::byte* data;
        long long size;
    };

    class ByteBuffer {
        private:
            std::byte* data;
            long long read_p;
            long long write_p;
            long long max_capacity;
            long long get_index(long long p);
            bool has_wrap_around();
        public:
            ByteBuffer(long long max_capacity);
            ~ByteBuffer();
            IOChunk get_read_chunk();
            IOChunk get_write_chunk();
            void increment_read_pointer(long long size);
            void increment_write_pointer(long long size);
            void read(std::byte* dest, long long size);
            void write(const std::byte* src, long long size);
            long long get_total_capacity();
            long long get_total_remaining();
    };
}

#endif
