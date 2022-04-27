#ifndef SCTDC_HDF5_HDF5WRITERIMPL_HPP
#define SCTDC_HDF5_HDF5WRITERIMPL_HPP

/*
 * Copyright (C) 2020 Surface Concept GmbH
*/

#include <atomic>
#include <string>
#include <vector>
#include <memory>
#include <thread>
#include "HDF5DataFile.hpp"
#include "HDF5Config.hpp"
#include "HDF5EventBuf.hpp"
#include "UcbAdapter.hpp"
#include "CircularBuf.hpp"
#include "ThirdParty/sema.h"
//#include <iostream>

//------------------------------------------------------------------------------

struct SpecialEvent {
  static const unsigned TYPE_NONE = 0;
  static const unsigned TYPE_DLD_MILLISEC = 0x10;
  static const unsigned TYPE_DLD_STARTMEAS = 0x11;
  static const unsigned TYPE_DLD_ENDMEAS = 0x12;
  unsigned long long eventidx;
  unsigned type;
};

struct HDF5WriterImplThreadLocal {
  static const std::size_t BUFSIZE = 50000;
  std::vector<unsigned short> bufx;
  std::vector<unsigned short> bufy;
  std::vector<unsigned long long> buftime;
  std::vector<unsigned long long> buf_ms;
  std::vector<unsigned long long> buf_start;
  HDF5DataFile file;
  HDF5WriterImplThreadLocal();
};


class HDF5WriterImplThread
{
  static const size_t DLD_EVENT_BUF_SIZE = 1<<20; // in number of elements
  static const size_t SPECIAL_EVENT_BUF_SIZE = 1<<16; // in number of elements
  //std::string basepath_;
  std::unique_ptr<std::thread> thread_;
  Semaphore semThreadStarted_;
  Semaphore semWakeUp_;
  std::atomic_bool abortRequest_;
  std::atomic_bool threadExited_;
  std::atomic_bool fileError_;
  std::unique_ptr<HDF5EventBuf> dld_event_buf_;
  CircularBuf<SpecialEvent> special_event_buf_{SPECIAL_EVENT_BUF_SIZE};
  HDF5WriterImplThreadLocal loc_;
  HDF5Config cfg_;
  std::atomic_bool file_error_;

  //std::size_t dld_thresh_counter_ = 0;
  std::size_t dld_event_counter_ = 0;
  std::size_t special_thresh_counter_ = 0;
  // ----------
  std::size_t DS_msMarkers;
  std::size_t DS_startMarkers;
  std::size_t DS_dld[HDF5EventBuf::NR_OF_BUFS];

public:
  //static const size_t DLD_NOTIF_THR = 50000; //DLD_EVENT_BUF_SIZE/128;
  static const size_t SPECIAL_NOTIF_THR = 50000; //SPECIAL_EVENT_BUF_SIZE/128;

  HDF5WriterImplThread() {}
  bool start();
  void stop();
  bool fileError();
  void push(const sc_DldEvent * const e, size_t len) {
    if (dld_event_buf_->push(e, len)) {
      semWakeUp_.signal();
    }
    dld_event_counter_ += len;
  }

  void push_millisecond() {
    push_special_event(SpecialEvent::TYPE_DLD_MILLISEC, dld_event_counter_);
  }

  void push_start_of_meas() {
    push_special_event(SpecialEvent::TYPE_DLD_STARTMEAS, dld_event_counter_);
  }

  void setConfig(const HDF5Config& c) {
    cfg_ = c;
    HDF5EventBufConfig ebc(c.datasel, 50000);
    dld_event_buf_.reset(new HDF5EventBuf(ebc));
  }

  const HDF5Config& config() const {
    return cfg_;
  }

private:
  void job_();
  void job_write_attributes_();
  void job_add_datasets_();
  void job_process_dld_events_();
  void job_process_last_dld_events_();
  void job_process_special_events_(bool force);
  bool special_events_thresh_exc() {
    return (special_event_buf_.avail_read() > loc_.BUFSIZE);
  }

  void push_special_event(unsigned type, unsigned long long eventidx) {
    SpecialEvent e;
    e.type = type;
    e.eventidx = eventidx;
    special_event_buf_.put(&e, 1);
    special_thresh_counter_ += 1;
    if (special_thresh_counter_ > SPECIAL_NOTIF_THR) {
      special_thresh_counter_ = 0;
      semWakeUp_.signal();
    }
  }
};

//------------------------------------------------------------------------------

class HDF5WriterImpl : private UcbAdapter<HDF5WriterImpl>
{
  friend class UcbAdapter<HDF5WriterImpl>;
  std::atomic_bool active_; // represents user request
  HDF5WriterImplThread hdf5_thread_;
  int dev_desc;
  bool file_error_;
  HDF5Config cfg_;

public:
  HDF5WriterImpl();
  ~HDF5WriterImpl();
  bool setActive(bool active);
  bool active() const;
  void devConnected(int devdesc);
  void devDisconnected();
  int deviceDescriptor() const;
  /**
   * @brief setConfig buffers the config, transfer to worker thread before start
   * @param c structure holding the config parameters
   */
  void setConfig(const HDF5Config& c);
  const HDF5Config& config() const;
  bool fileError() const;

private:
  void millisecond() {
    hdf5_thread_.push_millisecond();
  }
  void start_of_meas() {
    //std::cout << "start of meas" << std::endl;
    hdf5_thread_.push_start_of_meas();
  }
  void end_of_meas() {
    //hdf5_thread_.push_end_of_meas();
  }
  void statistics(const struct statistics_t *stat);
  void tdc_event(const struct sc_TdcEvent *const event_array,
    size_t event_array_len);
  void dld_event(const struct sc_DldEvent *const event_array,
    size_t event_array_len)
  {
    hdf5_thread_.push(event_array, event_array_len);
  }

};

//------------------------------------------------------------------------------

#endif // SCTDC_HDF5_HDF5WRITERIMPL_HPP
