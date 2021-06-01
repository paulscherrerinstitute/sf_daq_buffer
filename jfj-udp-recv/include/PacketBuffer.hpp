#ifndef CIRCULAR_BUFFER_TEMPLATE_HPP
#define CIRCULAR_BUFFER_TEMPLATE_HPP

#include <cstddef>
#include <stdexcept>
#include <iostream>
#include <mutex>
#include <sys/socket.h>
#include <netinet/in.h>


/** Linear data buffer (NOT FIFO)

	Simplified data buffer that provides pop and push operations and
	bundles the actual container with metadata required by <sockets.h>.
	It stores the actual data in an accessible C-style array. **/
template <typename T, size_t CAPACITY>
class PacketBuffer{
public:
    PacketBuffer() {
        for (int i = 0; i < CAPACITY; i++) {
            m_recv_buff_ptr[i].iov_base = (void*) &(m_container[i]);
            m_recv_buff_ptr[i].iov_len = sizeof(T);

            // C-structure as expected by <sockets.h>
            m_msgs[i].msg_hdr.msg_iov = &m_recv_buff_ptr[i];
            m_msgs[i].msg_hdr.msg_iovlen = 1;
            m_msgs[i].msg_hdr.msg_name = &m_sock_from[i];
            m_msgs[i].msg_hdr.msg_namelen = sizeof(sockaddr_in);
        }
     };
    // ~PacketBuffer() {};

    /**Diagnostics**/
    size_t size() const { return ( idx_write-idx_read ); }
    size_t capacity() const { return m_capacity; }
    bool is_full() const { return bool(idx_write >= m_capacity); }
    bool is_empty() const { return bool(idx_write <= idx_read); }

    /**Operators**/
    void reset(){ idx_write = 0; idx_read = 0; };		// Reset the buffer
    T& container(){ return m_container; };              // Direct container reference
    mmsghdr& msgs(){ return m_msgs; };

    /**Element access**/
    T& pop_front();             //Destructive read
    const T& peek_front();      //Non-destructive read
    void push_back(T item);     //Write new element to buffer

    /**Fill from UDP receiver**/
    template <typename TY>
    void fill_from(TY& recv){
        std::lock_guard<std::mutex> g_guard(m_mutex);
        this->idx_write = recv.receive_many(m_msgs, this->capacity());
        this->idx_read = 0;
    }

private:
    // Main container
    T m_container[CAPACITY];
    const size_t m_capacity = CAPACITY;
    /**Guards**/
    std::mutex m_mutex;
    /**Read and write index**/
    size_t idx_write = 0;
    size_t idx_read = 0;

    // C-structures as expected by <sockets.h>
    mmsghdr m_msgs[CAPACITY];
    iovec m_recv_buff_ptr[CAPACITY];
    sockaddr_in m_sock_from[CAPACITY];
};


/*********************************************************************/
/*********************************************************************/
/*********************************************************************/

/** Destructive read
	Standard read access to queues (i.e. progress the read pointer).
	Throws 'std::length_error' if container is empty. **/
template <typename T, size_t CAPACITY>
T& PacketBuffer<T, CAPACITY>::pop_front(){
    std::lock_guard<std::mutex> g_guard(m_mutex);
    if(this->is_empty()){ throw std::out_of_range("Attempted to read empty queue!"); }
    idx_read++;
    return m_container[idx_read-1];
}

/** Non-destructive read
	Standard, non-destructive read access (does not progress the read pointer).
	Throws 'std::length_error' if container is empty. **/
template <typename T, size_t CAPACITY>
const T& PacketBuffer<T, CAPACITY>::peek_front(){
    std::lock_guard<std::mutex> g_guard(m_mutex);
	if(this->is_empty()){ throw std::out_of_range("Attempted to read empty queue!"); }
    return m_container[idx_read];
}


/** Push an element into the end of the buffer**/
template <typename T, size_t CAPACITY>
void PacketBuffer<T, CAPACITY>::push_back(T item){
    std::lock_guard<std::mutex> g_guard(m_mutex);
    if(this->is_full()){ throw std::out_of_range("Attempted to write a full buffer!"); }
    m_container[idx_write] = item;
    idx_write++;
}

#endif // CIRCULAR_BUFFER_TEMPLATE_HPP
