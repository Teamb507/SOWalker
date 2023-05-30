#ifndef _GRAPH_BUFFER_H_
#define _GRAPH_BUFFER_H_

#include <assert.h>
#include <cstddef>
/** This file defines the buffer data structure used in graph processing */

template<typename T>
class base_buffer {
protected:
    size_t bsize;  // buffer current size
    size_t capacity;
    T *array;
};

template<typename T>
class graph_buffer : public base_buffer<T> {
public:
    graph_buffer() { this->bsize = this->capacity = 0, this->array = NULL; }
    graph_buffer(size_t size) { 
        alloc(size);
    }
    ~graph_buffer() { this->destroy(); }

    void alloc(size_t size) { 
        this->capacity = size;
        this->bsize = 0;
        this->array = (T*)malloc(size * sizeof(T));
    }

    void realloc(size_t size) {
        this->array = (T*)malloc(this->array, size * sizeof(T));
        this->capacity = size;
        this->bsize = 0;
    }

    void destroy() { 
        if(this->array) free(this->array);
        this->array = NULL;
        this->bsize = 0;
        this->capacity = 0;
    }

    T& operator[](size_t off) {
        assert(off < this->bsize);
        return this->array[off];
    }

    T* &buffer_begin() { return this->array; }
    size_t size() const { return this->bsize; } 

    bool push_back(T val) {
        if(this->bsize < this->capacity) { 
            this->array[this->bsize++] = val;
            return true;
        }
        return false;
    }

    bool empty() { return this->bsize == 0; }
    bool full() { return this->bsize == this->capacity; }
    
    /** test if add num elements whether will overflow the maximum capacity or not. */
    bool test_overflow(size_t num) {
        return this->bsize + num > this->capacity;
    }

    void clear() {
        this->bsize = 0;
    }

    void set_size(size_t _size) {
        this->bsize = _size;
    }
};

#endif