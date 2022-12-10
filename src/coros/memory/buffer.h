#ifndef COROS_MEMORY_BUFFER_H
#define COROS_MEMORY_BUFFER_H

#include <cstddef>

namespace coros::base {
    struct IOChunk {
        std::byte* data;
        int size;
    };

    class ByteBuffer {
        private:
            std::byte* data;
            int read_p;
            int write_p;
            int max_capacity;
            int get_index(int p);
            bool has_wrap_around();
        public:
            ByteBuffer(int max_capacity);
            ~ByteBuffer();
            IOChunk get_read_chunk();
            IOChunk get_write_chunk();
            void increment_read_pointer(int size);
            void increment_write_pointer(int size);
            void read(std::byte* dest, int size);
            std::byte read_b();
            void write(std::byte* src, int size);
            void write_b(std::byte b);
            int get_total_capacity();
            int get_total_remaining();
    };
}

#endif
