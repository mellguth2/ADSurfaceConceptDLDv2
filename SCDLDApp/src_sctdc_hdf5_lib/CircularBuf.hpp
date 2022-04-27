/*
 * Copyright 2020 Surface Concept GmbH
 */
/* 
 * Thread-safe circular buffer (for FIFO with 1 producer and 1 consumer).
 * The producer calls the put function, and the consumer calls the consume
 * function. For consumption, data copy is avoided by passing contigous memory 
 * regions directly from the circular buffer to a consumer-provided function, 
 * while the circular buffer is locked for access by others.
 * Size is restricted to powers of 2, such that internally, wrapping of the 
 * index can be done via a bitwise AND operation instead of an expensive modulo 
 * operation (modulo involves division!).
 * Supports adding and consuming multiple elements at once.
 */

#ifndef CIRCULARBUF_HPP
#define CIRCULARBUF_HPP

#include <mutex>
#include <memory>
#include <functional>
//#include <iostream>

template <class T>
class CircularBuf {
public:
    /**
     * CircularBuf constructor.
     * @param size the size of the circular buffer
     * the specified size is accepted if it is a power of 2, otherwise it is
     * increased to the next power of 2
     **/
    CircularBuf(std::size_t size)
    {
      // restrict size to 2 .. 2^40
      if (size > (1ull << 40))
        size = (1ull << 40) - 1;
      if (size < 2ull)
        size = 2ull;
      // find power of 2 that is greater or equal to specified size
      std::size_t adapted_size = 1ull;
      while ( adapted_size < size )
        adapted_size = adapted_size << 1;
      size_ = adapted_size;
      idxmask_ = size_ - 1ull;
      buf_.reset(new T[size_]);
    }

    /**
     * @brief adds data to the circular buffer 
     * does not put any data, if the item_count does not fit into the available
     * space
     * @param items, an array of items
     * @param item_count the length of the array
     * @return true on success; false, if not all data fits */
    bool put(const T* items, std::size_t item_count) {
        std::lock_guard<std::mutex> lock(mutex_);
        return _put_impl(items, item_count);
    }
    
    /**
     * consume multiple items from the ring buffer by calling a function that 
     * receives a data pointer and size argument and shall return the number of 
     * elements that it processed.
     * No check is done that the function f returns a valid number of processed
     * elements! If the reported number of processed elements is larger than 
     * the available elements, the ring buffer will subsequently yield lots of
     * invalid/undefined data.
     * The function f may be called twice if the available data wraps around the
     * end of the ring buffer. This will only happen if the function f reported
     * the same number of elements as consumed that were passed to it.
     * @param f function that will get called
     */
    void consume(std::function<size_t(T*, std::size_t)> f) {
        std::lock_guard<std::mutex> lock(mutex_);
        _consume_impl(f);
    }

    /** consume items only if at least *thresh* number of items is available */
    void consume_at_least(std::size_t thresh,
                          std::function<size_t(T*, std::size_t)> f) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (_avail_read_impl() >= thresh)
          _consume_impl(f);
    }
    
    /**
     * @brief puts the circular buffer into a state where no data is available
     **/
    inline void reset(void)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        head_ = tail_;
    }

    /**
     * @brief check whether the buffer is empty
     * @return true if the buffer is empty
     **/
    inline bool empty(void) {
        std::lock_guard<std::mutex> lock(mutex_);
        return _empty_impl();
    }
    
    /**
     * @brief check whether the buffer is full
     * @return true if the buffer is full
     */
    inline bool full(void) {
        std::lock_guard<std::mutex> lock(mutex_);
        return ((head_ + 1) & idxmask_) == tail_;
    }
    
    /**
     * @brief the number of elements that can be put, it the
     * circular buffer is empty. The internal implementation is such that
     * one cell has to remain empty. Therefore, the size is reduced by 1. */
    inline size_t size(void) {
        return size_ - 1;
    }

    inline std::size_t avail_read() {
        std::lock_guard<std::mutex> lock(mutex_);
        return _avail_read_impl();
    }
    
private:
    inline bool _put_impl(const T* items, std::size_t item_count) {
      //std::cout << _avail_write_impl() << std::endl;
      if (item_count > _avail_write_impl())
        //return false;
        throw 0xff;
      for (std::size_t i = 0; i < item_count; i++) {
        buf_[head_] = items[i];
        head_ = (head_ + 1) & idxmask_;
        if (head_ == tail_)
          tail_ = (tail_ + 1) & idxmask_;
      }
      return true;
    }
    
    inline bool _empty_impl() {
      return head_ == tail_;
    }

    inline void _consume_impl(std::function<size_t(T*, std::size_t)> f) {
      if (_empty_impl()) return;
      else if (head_ > tail_) {
          size_t processed = f(&buf_[tail_], head_ - tail_);
          tail_ = (tail_ + processed) & idxmask_;
      }
      else if (head_ < tail_) {
          size_t processed = f(&buf_[tail_], size_ - tail_);
          tail_ = (tail_ + processed) & idxmask_;
          // tail_ may wrap around to the start of the ring buffer and more
          // data might be available, just pass this to f in a second call
          if (head_ > tail_) {
            size_t processed = f(&buf_[tail_], head_ - tail_);
            tail_ = (tail_ + processed) & idxmask_;
          }
      }
    }

    // return bytes available for reading
    inline std::size_t _avail_read_impl(void) {
      if (head_ == tail_) return 0;
      else if (head_ > tail_) return head_ - tail_;
      else return size_ + head_ - tail_;
    }
    
    inline std::size_t _avail_write_impl(void) {
      return size_ - _avail_read_impl();
    }
  
    std::mutex mutex_;
    std::unique_ptr<T[]> buf_;
    size_t head_ = 0;
    size_t tail_ = 0;
    size_t size_;
    size_t idxmask_;
};

#endif /* CIRCULARBUF_HPP */

