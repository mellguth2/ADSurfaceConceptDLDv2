#ifndef GUI2_HDF5EVENTBUF_HPP
#define GUI2_HDF5EVENTBUF_HPP

/*
 * Copyright (C) 2020 Surface Concept GmbH
*/

// the written HDF5 has 1D arrays of numbers
// for writing these, events must be cast into such arrays in memory

#include <vector>
#include <string>
#include <mutex>
#include <atomic>
#include "HDF5Config.hpp"
#include "HDF5Utilities.h"
#include <scTDC_types.h>
//#include <iostream>

struct HDF5EventBufConfig {
  EventDataFieldSelection datasel;
  unsigned long long size;
  HDF5EventBufConfig() : size(10) { }
  HDF5EventBufConfig(
    const EventDataFieldSelection& datasel_arg,
    unsigned long long size_arg)
    : datasel(datasel_arg), size(size_arg) {}
};

struct HDF5EventBufFull : public std::exception
{
  const char * what () const throw () {
    return "HDF5 event buffer is full";
  }
};

/**
 * @brief The HDF5EventBuf class inter-thread event data exchange for HDF5
 * writing. Accomplishes two things: (1) double buffers (two "buffer pages") for
 * letting the consumer read the data without mutex locking most of the time
 * (except for getting the final remainder of data at the end). (2) casting the
 * incoming events into 1D arrays (sc_DldEvent* is an array of structs, for
 * the HDF5 we want a struct of arrays --- calling the HDF5 function for
 * appending data has considerable overhead, so one cannot append single
 * events).
 */
class HDF5EventBuf
{
public:
  // constants filling a contigous range starting from zero, must be
  // synchronized with arrays 'mask_from_bufid' and 'element_sizes'
  // and who knows what else
  static const unsigned BSTARTCTR = 0;
  static const unsigned BTIMETAG  = 1;
  static const unsigned BSUBDEV   = 2;
  static const unsigned BCHANNEL  = 3;
  static const unsigned BSUM      = 4;
  static const unsigned BDIF1     = 5;
  static const unsigned BDIF2     = 6;
  static const unsigned BMRC      = 7;
  static const unsigned BADC      = 8;
  static const unsigned BSIGBIT   = 9;
  static const unsigned NR_OF_BUFS = 10; // one larger than the highest index
  // -----------------------

  HDF5EventBuf(const HDF5EventBufConfig& c)
    : cfg_(c), buf_el_idx_(0), buf0full_(false), buf1full_(false),
      activebuf_(0)
  {
    initbufs();
    fill_h5types();
    fill_names();
  }

  ~HDF5EventBuf()
  {
    deletebufs();
  }

  /**
   * @brief push add DLD event data to the event buffer. The return value is
   * useful to decide whether to wake up a consumer thread. Locks a mutex which
   * should be almost always unlocked (negligible performance cost).
   * This mutex will only be taken by a consumer, if the consumer tries
   * to get the latest data from a partially filled buffer page.
   * @param e DLD event array
   * @param len length of the DLD event array
   * @return true if a buffer page became full
   */
  bool push(const sc_DldEvent *const e, std::size_t len)
  {
    std::unique_lock<std::mutex> lock(mutex_);
    bool bufpage_became_full = false;
    for (std::size_t eidx = 0; eidx < len; eidx++) {
      const sc_DldEvent& ev = e[eidx];
      push_val(BSTARTCTR, ev.start_counter);
      push_val(BTIMETAG, ev.time_tag);
      push_val(BSUBDEV, ev.subdevice);
      push_val(BCHANNEL, ev.channel);
      push_val(BSUM, ev.sum);
      push_val(BDIF1, ev.dif1);
      push_val(BDIF2, ev.dif2);
      push_val(BMRC, ev.master_rst_counter);
      push_val(BADC, ev.adc);
      push_val(BSIGBIT, ev.signal1bit);
      // ---  once after each event --------------------------
      activebuf_len_ += 1;
      if (activebuf_len_ == cfg_.size) {
        switchbuf();
        bufpage_became_full = true;
      }
    }
    return bufpage_became_full;
  }

  /**
   * @brief get_buf returns full buf page for the specified data field.
   * Used by consumer.
   * Only returns a buffer if there is a full buffer page. No mutex locking
   * is needed since the returned buffer is not used by the producer.
   * The buffer page has a length as returned by size(). After getting all
   * data field buffers, release_page() must be called to mark the consumed
   * buffer page as free for subsequent writing accesses.
   * For getting latest data from the partially filled buffer page, use
   * get_partial_buf, instead.
   * @param buf_id must be one of BSTARTCTR, BTIMETAG, ..., BSIGBIT
   * @return buffer pointer
   */
  void* get_buf(unsigned buf_id) const {
    if (buf_id >= NR_OF_BUFS)
      return nullptr;
    if (buf0full_.load())
      return buffers_[buf_id];
    else if (buf1full_.load())
      return buffers_[buf_id+NR_OF_BUFS];
    else
      return nullptr;
  }

  /**
   * @brief get_partial_buf returns partial buf page for specified data field.
   * Used by consumer. Requires that mutex() be locked (which will block the
   * producer). During the lock, the consumer will want to call this function
   * for all desired data fields and then call release_partial_page().
   * @param buf_id must be one of BSTARTCTR, BTIMETAG, ..., BSIGBIT
   * @param b (*b) takes the pointer to the partial buffer
   * @param len (*len) takes the number of valid entries in partial buffer
   */
  void get_partial_buf(unsigned buf_id, void** b, size_t* len) const {
    if (buf_id >= NR_OF_BUFS ||
        ((cfg_.datasel.value & mask_from_bufid[buf_id]) == 0))
    {
      *b = nullptr;
      *len = 0;
    }
    *len = activebuf_len_;
    *b = buffers_[buf_id+(activebuf_ * NR_OF_BUFS)];
  }

  bool has_data_page() const {
    return (buf0full_.load() || buf1full_.load());
  }

  hid_t get_h5_type(unsigned buf_id) const {
    if (buf_id >= NR_OF_BUFS)
      return -1;
    return element_h5types[buf_id];
  }

  EventDataFieldSelection::type maskFromBufId(unsigned buf_id) const {
    if (buf_id >= NR_OF_BUFS)
      return 0;
    else
      return mask_from_bufid[buf_id];
  }

  /**
   * @brief release_page release a consumed buffer page.
   * Used by the consumer to indicate that it has finished processing the
   * currently filled buffer page.
   */
  void release_page() {
    if (buf0full_.load())
      buf0full_.store(false);
    else if (buf1full_.load())
      buf1full_.store(false);
  }

  /**
   * @brief release_partial_page release a consumed partial buffer page.
   * Used by the consumer to indicate that it has finished processing the
   * latest data from the partially filled buffer page. Requires the mutex()
   * to be locked by the consumer. Use this in conjunction with get_partial_buf.
   */
  void release_partial_page();

  std::size_t size() const {
    return cfg_.size;
  }

  std::mutex& mutex() { return mutex_; }

  const char* get_name(unsigned buf_id) const {
    if (buf_id >= NR_OF_BUFS)
      return emptyname_.c_str();
    return names_[buf_id].c_str();
  }

private: // methods
  void switchbuf(bool init=false); // switch between buffer pages #0 and #1
  void initbufs(); // allocate buffers and set unused element sizes to zero
  void deletebufs(); // free buffers
  void fill_h5types();

  template <typename T>
  void push_val(unsigned buf_id, const T& val) {
    if (buf_el_ptrs_[buf_id]==nullptr) throw -1;
    *reinterpret_cast<T*>(buf_el_ptrs_[buf_id]) = val;
    buf_el_ptrs_[buf_id] = reinterpret_cast<char*>(buf_el_ptrs_[buf_id])
        + element_sizes[buf_id];
    // (had to use the pointer increment from element_sizes, because for
    // unused data fields, the ptr must remain on the datadummy_ element)
  }

  template <typename T>
  void fill_h5type(unsigned buf_id, const T& val);

  void fill_names();

  void _reset_buffer_ptrs();

private:
  std::mutex mutex_;
  HDF5EventBufConfig cfg_;
  unsigned long long buf_el_idx_;

  std::atomic_bool buf0full_;
  std::atomic_bool buf1full_;
  unsigned activebuf_;
  unsigned long long activebuf_len_; // number of written elements in active buf


  void* buffers_[NR_OF_BUFS*2];
  void* buf_el_ptrs_[NR_OF_BUFS];
  unsigned buf_incr_[NR_OF_BUFS]; // byte-address increment of each buffer

  EventDataFieldSelection::type mask_from_bufid[NR_OF_BUFS] =
  { EventDataFieldSelection::STARTCTR, EventDataFieldSelection::TIMETAG,
    EventDataFieldSelection::SUBDEV,   EventDataFieldSelection::CHANNEL,
    EventDataFieldSelection::SUM,      EventDataFieldSelection::DIF1,
    EventDataFieldSelection::DIF2,     EventDataFieldSelection::MRC,
    EventDataFieldSelection::ADC,      EventDataFieldSelection::SIGBIT
  };

  unsigned element_sizes[NR_OF_BUFS] = {
    sizeof(sc_DldEvent::start_counter), sizeof(sc_DldEvent::time_tag),
    sizeof(sc_DldEvent::subdevice),     sizeof(sc_DldEvent::channel),
    sizeof(sc_DldEvent::sum),           sizeof(sc_DldEvent::dif1),
    sizeof(sc_DldEvent::dif2),          sizeof(sc_DldEvent::master_rst_counter),
    sizeof(sc_DldEvent::adc),           sizeof(sc_DldEvent::signal1bit)
  };

  std::vector<std::string> names_;
  std::string emptyname_;

  hid_t element_h5types[NR_OF_BUFS];
  H5::DataType h5type_objs[NR_OF_BUFS];

  unsigned long long datadummy_;
};

#endif
