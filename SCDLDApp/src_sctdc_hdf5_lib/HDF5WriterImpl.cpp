/*
 * Copyright (C) 2020 Surface Concept GmbH
*/
#include "HDF5WriterImpl.hpp"
//#include <iostream>
#include "final_act.h"

HDF5WriterImpl::HDF5WriterImpl()
  : UcbAdapter<HDF5WriterImpl>(this),
    active_(false),
    dev_desc(-1),
    file_error_(false)
{

}

HDF5WriterImpl::~HDF5WriterImpl()
{

}

bool HDF5WriterImpl::active() const
{
  return active_.load();
}

bool HDF5WriterImpl::setActive(bool active_arg)
{
  bool is_active = active();
  if (active_arg == is_active)
    return is_active;
  if (active_arg) {
    hdf5_thread_.setConfig(cfg_);
    bool success1 = hdf5_thread_.start();
    if (!success1) {
      file_error_ = hdf5_thread_.fileError();
      return false;
    }
    file_error_ = false;
    if (dev_desc > -1)
      install(dev_desc); // install user callbacks pipe
  }
  else {
    hdf5_thread_.stop();
    if (dev_desc > -1) // if deinitialized in the meantime, pipe already closed
      deinstall();
  }
  active_.store(active_arg);
  return active_arg;
}

void HDF5WriterImpl::devConnected(int devdesc)
{
  dev_desc = devdesc;
}

void HDF5WriterImpl::devDisconnected()
{
  dev_desc = -1;
  setActive(false);
}

int HDF5WriterImpl::deviceDescriptor() const
{
  return dev_desc;
}

void HDF5WriterImpl::setConfig(const HDF5Config &c)
{
  cfg_ = c;
}

const HDF5Config& HDF5WriterImpl::config() const
{
  return cfg_;
}

bool HDF5WriterImpl::fileError() const
{
  return file_error_;
}

// -----------------------------------------------------------------------------
// ---                    events                                             ---
// -----------------------------------------------------------------------------

void HDF5WriterImpl::statistics(const statistics_t *stat)
{
  (void)stat;
}

void HDF5WriterImpl::tdc_event(const sc_TdcEvent * const e, size_t len)
{
  (void)e;
  (void)len;
}

// -----------------------------------------------------------------------------
// ---                    HDF5WriterImplThreadLocal                          ---
// -----------------------------------------------------------------------------
HDF5WriterImplThreadLocal::HDF5WriterImplThreadLocal()
{
  buf_ms.resize(BUFSIZE);
  buf_start.resize(BUFSIZE);
}

// -----------------------------------------------------------------------------
// ---                    HDF5WriterImplThread                               ---
// -----------------------------------------------------------------------------

bool HDF5WriterImplThread::start()
{
  if (thread_) {
    return false;
  }
  // initialize thread and wait until it notifies about being up and running
  abortRequest_.store(false);
  threadExited_.store(false);
  thread_.reset(new std::thread(&HDF5WriterImplThread::job_, this));
  semThreadStarted_.wait();
  if (fileError()) {
    stop();
    return false;
  }
  return true;
}

void HDF5WriterImplThread::stop()
{
  if (!threadExited_.load()) {
    abortRequest_.store(true);
    semWakeUp_.signal();
    thread_->join();
  }
  thread_.reset();
  //std::cout << "HDF5WriterImplThread::stop() : finished" << std::endl;
}

bool HDF5WriterImplThread::fileError()
{
  return file_error_.load();
}

void HDF5WriterImplThread::job_()
{
  //std::cout << "HDF5WriterImplThread::job_() : entered" << std::endl;
  auto clean_up1 = finally([this](){ threadExited_.store(true); });
  (void)clean_up1;
  loc_.file.open(cfg_.base_path);
  if (!loc_.file.isOpen()) {
    file_error_.store(true);
    semThreadStarted_.signal();
    while (!abortRequest_.load()) {
      std::this_thread::yield();
    }
    return;
  }
  file_error_.store(false);
  semThreadStarted_.signal();

  job_write_attributes_();
  job_add_datasets_();
  dld_event_counter_ = 0u;

  // ---------------------------------------------------------------------------
  // data streaming part:
  while(!abortRequest_.load()) {
    while (!dld_event_buf_->has_data_page() && !special_events_thresh_exc() &&
      !abortRequest_.load())
    {
      semWakeUp_.wait();
    }
    job_process_dld_events_();
    job_process_special_events_(false);
  }
  // ---------------------------------------------------------------------------
  // write last remaining data from ring buffers, close file, -> end of thread
  job_process_last_dld_events_();
  job_process_special_events_(true);
  loc_.file.closeDataSets();
  loc_.file.close();

  //std::cout << "HDF5WriterImplThread::job_() : exiting" << std::endl;
}

void HDF5WriterImplThread::job_write_attributes_()
{
  if (cfg_.user_comment.size()==0)
    cfg_.user_comment = "(empty)";
  loc_.file.addRootAttrib("UserComment", cfg_.user_comment);
  loc_.file.addRootAttrib("Description",
    std::string("DLD Detector data written from Surface Concept DldGui2"));
}

void HDF5WriterImplThread::job_add_datasets_()
{
  HDF5EventBuf& eb = *dld_event_buf_;
  for (std::size_t i = 0; i < eb.NR_OF_BUFS; i++) {
    if (cfg_.datasel.value & eb.maskFromBufId(i)) {
      DS_dld[i] = loc_.file.addDataSetRaw(eb.get_name(i), eb.get_h5_type(i));
    }
    else
      DS_dld[i] = 0xFFFFFFFFFFFFFFFFull;
  }
  DS_msMarkers = loc_.file.addDataSet<unsigned long long>("msMarkers");
  DS_startMarkers = loc_.file.addDataSet<unsigned long long>("startMarkers");
}



void HDF5WriterImplThread::job_process_dld_events_()
{
  // don't lock mutex here
  std::size_t s = dld_event_buf_->size();
  for (std::size_t buf_id = 0; buf_id < dld_event_buf_->NR_OF_BUFS; buf_id++)
  {
    void* b = dld_event_buf_->get_buf(buf_id);
    if (b)
      loc_.file.appendToDataSetRaw(
        DS_dld[buf_id], b, s, dld_event_buf_->get_h5_type(buf_id));
  }
  dld_event_buf_->release_page();
}

void HDF5WriterImplThread::job_process_last_dld_events_()
{
  std::unique_lock<std::mutex> lock(dld_event_buf_->mutex());
  void* b;
  std::size_t len;
  for (std::size_t buf_id = 0; buf_id < dld_event_buf_->NR_OF_BUFS; buf_id++)
  {
    dld_event_buf_->get_partial_buf(buf_id, &b, &len);
    if (b)
      loc_.file.appendToDataSetRaw(
        DS_dld[buf_id], b, len, dld_event_buf_->get_h5_type(buf_id));
  }
  dld_event_buf_->release_partial_page();
}


void HDF5WriterImplThread::job_process_special_events_(bool force)
{
  std::size_t thresh = (force) ? 0 : loc_.BUFSIZE;
  special_event_buf_.consume_at_least(
    thresh, [this](SpecialEvent *e, std::size_t len)
    {
      //
      std::size_t lenleft = len;
      std::size_t k = 0;
      while (lenleft > 0) {
        std::size_t jms = 0;
        std::size_t jsom = 0;
        std::size_t len1 = lenleft;
        if (len1 > loc_.BUFSIZE) len1 = loc_.BUFSIZE;
        for (std::size_t i = 0; i < len1; i++) {
          if (e[k].type == SpecialEvent::TYPE_DLD_MILLISEC) {
            loc_.buf_ms[jms++] = e[k].eventidx;
          }
          else if (e[k].type == SpecialEvent::TYPE_DLD_STARTMEAS) {
            loc_.buf_start[jsom++] = e[k].eventidx;
            //std::cout << "Start of meas encountered" << std::endl;
          }
          k++;
        }
        if (jms > 0) {
          loc_.file.appendToDataSet(DS_msMarkers, loc_.buf_ms.data(), jms);
          //std::cout << "appending " << jms << " ms markers" << std::endl;
        }
        if (jsom > 0)
          loc_.file.appendToDataSet(
                DS_startMarkers, loc_.buf_start.data(), jsom);
        lenleft = lenleft - len1;
      }
      return len;
    });
}
