#ifndef CIRCULAR_BUFFER_TEMPLATE_HPP
#define CIRCULAR_BUFFER_TEMPLATE_HPP

#include <algorithm>
#include <cstddef>
#include <cassert>
#include <stdexcept>
#include <iostream>

/**Linear data buffer

A simplified version of FIFO.
**/
template <typename T, size_t CAP>
class DataBuffer{
public:
    DataBuffer() {};
    ~DataBuffer() {};

    /**Diagnostics**/
    size_t size() const { return ( _write-_read ); }
    size_t capacity() const { return _capacity; }
    bool is_full(){ return ( (_write - _read)<_capacity ); }
    bool is_empty(){ return (_write ==_read); }

    /**Operators**/
    void zero(){ memset(m_cont, 0, sizeof(m_cont)); }
    T& operator[](size_t index);                                  // Array subscript operator
    T& container(){  return (_cont; }                             // Direct container reference

    /**Element access**/
    const T& pop_front();       //Destructive read
    const T& get_front();       //Non-destructive read
    void push_back(T item);     //Write new element to buffer

    /**Guards**/
    std::mutex g_mutex;
private:
    T m_cont[CAP];
    const size_t m_capacity = CAP;
    size_t ptr_write = 0;
    size_t ptr_read = 0;
};

/** Array subscript operator
    Throws 'std::length_error' if out of range.
**/
template<typename T>
T& DataBuffer<T>::operator[](size_t idx){
    if(idx > m_capacity){
        std::string msg = "Buffer index '" + std::to_string(idx) + "' is out of range with capacity '" + std::to_sting(m_capacity) + "'" + std::endl;
        throw std::out_of_range(msg);
    }

    return m_buffer[idx];
}

template<typename T>
T& DataBuffer<T>::container(){
    return m_buffer;
}

/*********************************************************************/

/** Destructive read (i.e. progress the read pointer) **/
template<typename T>
const T& DataBuffer<T>::pop_front(){
    std::lock_guard<std::mutex> g_guard;
    ptr_read++;
    return _buffer[ptr_read-1];
}

/**Push a new element to the ringbuffer (do not progress read pointer)**/
template<typename T>
const T& DataBuffer<T>::peek_front(){
    return m_buffer[ptr_read];
}


/**Push a new element to the ringbuffer**/
template<typename T>
void DataBuffer<T>::push_back(T item){
    std::lock_guard<std::mutex> g_guard;
    if(ptr_write==m_capacity-1){
        std::string msg = "Buffer with '" + std::to_sting(m_capacity) + "' capacity is full" + std::endl;
        throw std::out_of_range(msg);
    }
    m_buffer[ptr_write] = item;
    ptr_write++;
}

#endif // CIRCULAR_BUFFER_TEMPLATE_HPP
