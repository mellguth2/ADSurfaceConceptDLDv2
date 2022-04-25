/* Copyright 2022 Surface Concept GmbH */

#include "DLD.hpp"
#include <string>
#include <scTDC.h>
#include <scTDC_error_codes.h>
#include <thread>
#include <future>
#include <iostream>


enum ImageModeEnum : int {
  IMAGEMODE_SINGLE = 0,
  IMAGEMODE_MULTIPLE = 1,
  IMAGEMODE_CONTINUOUS = 2
};
enum DetectorStateEnum : int {
  DETECTORSTATE_IDLE = 0,
  DETECTORSTATE_ACQUIRE = 1,
  DETECTORSTATE_READOUT = 2,
  DETECTORSTATE_CORRECT = 3,
  DETECTORSTATE_SAVING = 4,
  DETECTORSTATE_ABORTING = 5,
  DETECTORSTATE_ERROR = 6,
  DETECTORSTATE_WAITING = 7,
  DETECTORSTATE_INITIALIZING = 8,
  DETECTORSTATE_DISCONNECTED = 9,
  DETECTORSTATE_ABORTED = 10
};

namespace {
template <class F>
void call_async(F&& fun) {
    auto futptr = std::make_shared<std::future<void>>();
    *futptr = std::async(std::launch::async, [futptr, fun]() {
        fun();
    });
}
}

DLD::DLD()
  : Glue(this),
    dev_desc_(-1)
{
  configure_pipes();
}

DLD::Data::Data()
  : configfile("tdc_gpx3.ini"),
    initialized(0),
    exposure(1.0),
    acquire_period(0.1),
    acquire(0),
    image_mode(IMAGEMODE_SINGLE),
    num_images(1),
    image_counter(0),
    detector_state(DETECTORSTATE_DISCONNECTED),
    user_stop_request(false)
{ }

int DLD::write_Initialize(int v)
{
  if (data_.initialized == 0 && v == 1) {
    worker_.addTask( [this]() {
      update_StatusMessage("hardware initializing...");
      int ret = init_impl(); // this step can take seconds for some devices
      if (ret < 0) {
        char buf[ERRSTRLEN]; // ERRSTRLEN from scTDC.h, should be 256
        buf[0] = '\0';
        sc_get_err_msg(ret, buf);
        update_StatusMessage(buf);
      }
      else {
        data_.initialized = 1;
        update_Initialize(data_.initialized);
        update_StatusMessage("hardware ready");
        update_DetectorState(DETECTORSTATE_IDLE);
        data_.acquire = 0;
        update_Acquire(0);
        for (auto& createdAtInit : created_at_init_) {
          createdAtInit->create(dev_desc_);
        }
      }
    });
  }
  else if (data_.initialized == 1 && v == 0 && dev_desc_ >= 0) {
    worker_.addTask( [this]() {
      sc_tdc_deinit2(dev_desc_);
      data_.initialized = 0;
      update_Initialize(data_.initialized);
      update_StatusMessage("hardware closed");
      update_DetectorState(DETECTORSTATE_DISCONNECTED);
    });
  }
  return 0;
}

int DLD::read_Initialize(int *v)
{
  *v = data_.initialized;
  return 0;
}

int DLD::write_ConfigFile(const std::string& v)
{
  data_.configfile = v;
  return 0;
}

int DLD::read_ConfigFile(std::string& dest)
{
  dest = data_.configfile;
  return 0;
}

int DLD::read_StatusMessage(std::string &dest)
{
  dest = data_.statusmessage;
  return 0;
}

int DLD::read_DetectorState(int *dest)
{
  *dest = data_.detector_state;
  return 0;
}

int DLD::write_Exposure(double v)
{
  data_.exposure = v;
  return 0;
}

int DLD::read_Exposure(double *dest)
{
  *dest = data_.exposure;
  return 0;
}

int DLD::write_AcquirePeriod(double v)
{
  data_.acquire_period = v;
  return 0;
}

int DLD::read_AcquirePeriod(double *dest)
{
  *dest = data_.acquire_period;
  return 0;
}

int DLD::write_Acquire(int v)
{
  if (data_.acquire == 0 && v == 1 && data_.initialized == 1) {
    data_.image_counter = 0;
    data_.user_stop_request = false;
    return start_measurement();
  }
  else if (data_.acquire == 1 && v == 0 && data_.initialized == 1) {
    if (data_.image_mode == IMAGEMODE_SINGLE) {
      sc_tdc_interrupt2(dev_desc_); // asynchronous, do not update_Acquire yet
    }
    else {
      data_.user_stop_request = true;
    }
    return 0;
  }
  return 0;
}

int DLD::read_Acquire(int *dest)
{
  *dest = data_.acquire;
  return 0;
}

int DLD::write_ImageMode(int v)
{
  data_.image_mode = v;
  return 0;
}

int DLD::read_ImageMode(int *dest)
{
  *dest = data_.image_mode;
  return 0;
}

int DLD::write_NumImages(int v)
{
  data_.num_images = v;
  return 0;
}

int DLD::read_NumImages(int *dest)
{
  *dest = data_.num_images;
  return 0;
}

int DLD::read_NumImagesCounter(int *dest)
{
  *dest = data_.image_counter;
  return 0;
}

int DLD::write_BinX(int v)
{
  liveimagexy_.setBinX(v);
  return 0;
}

int DLD::read_BinX(int *dest)
{
  return 0;
}

int DLD::write_BinY(int v)
{
  liveimagexy_.setBinY(v);
  return 0;
}

int DLD::read_BinY(int *dest)
{
  *dest = liveimagexy_.binY();
  return 0;
}

int DLD::write_MinX(int v)
{
  liveimagexy_.setMinX(v);
  return 0;
}

int DLD::read_MinX(int *dest)
{
  *dest = liveimagexy_.minX();
  return 0;
}

int DLD::write_MinY(int v)
{
  liveimagexy_.setMinY(v);
  return 0;
}

int DLD::read_MinY(int *dest)
{
  *dest = liveimagexy_.minY();
  return 0;
}

int DLD::write_SizeX(int v)
{
  liveimagexy_.setSizeX(v);
  return 0;
}

int DLD::read_SizeX(int *dest)
{
  *dest = liveimagexy_.sizeX();
  return 0;
}

int DLD::write_SizeY(int v)
{
  liveimagexy_.setSizeY(v);
  return 0;
}

int DLD::read_SizeY(int *dest)
{
  *dest = liveimagexy_.sizeY();
  return 0;
}

int DLD::init_impl()
{
  int ret = sc_tdc_init_inifile(data_.configfile.c_str());
  if (ret == 0) {
    dev_desc_ = ret;
    sc_tdc_set_complete_callback2(
      dev_desc_, this, cb_static_measurement_complete);
  }
  return ret;
}

void DLD::configure_pipes()
{
  configure_pipes_ratemeter();
  configure_pipes_liveimagexy();
}

void DLD::configure_pipes_liveimagexy()
{
  liveimagexy_.setDataConsumer(
    [this](std::size_t length, std::size_t width, int* data) {
      update_LiveImageXY(length, width, data);
    });
  created_at_init_.push_back(&liveimagexy_);
  som_listeners_.push_back(&liveimagexy_);
  eom_listeners_.push_back(&liveimagexy_);
}

void DLD::configure_pipes_ratemeter()
{
  ratemeter_.setDataConsumer([this](int* data, std::size_t length){
    update_Ratemeter(length, data);
  });
  created_at_init_.push_back(&ratemeter_);
  som_listeners_.push_back(&ratemeter_);
  eom_listeners_.push_back(&ratemeter_);
}

void DLD::cb_measurement_complete(int reason)
{
#if 0
  static const int FINISHED = 1;
  static const int INTERRUPTED_BY_USER = 2;
  static const int BUFFER_FULL = 3;
#endif
  static const int EARLY_NOTIF = 4;
  if (reason != EARLY_NOTIF) {

    worker_.addTask([&]() {
      for (auto& eom_listener : eom_listeners_) {
        eom_listener->end_of_measurement();
      }
      data_.image_counter++;
      update_NumImagesCounter(data_.image_counter);
      if (data_.image_mode == IMAGEMODE_SINGLE) {
        data_.acquire = 0;
        update_Acquire(0);
        update_DetectorState(DETECTORSTATE_IDLE);
      }
      else if (data_.image_mode == IMAGEMODE_MULTIPLE ||
               data_.image_mode == IMAGEMODE_CONTINUOUS)
      {
        if ((data_.image_mode == IMAGEMODE_MULTIPLE &&
            data_.image_counter >= data_.num_images)
            || data_.user_stop_request)
        {
          data_.acquire = 0;
          update_Acquire(0);
          update_DetectorState(DETECTORSTATE_IDLE);
        }
        else {
          auto tp = last_acq_start_
            + std::chrono::milliseconds(
                static_cast<int>(data_.acquire_period * 1000.0));
          if (tp <= std::chrono::steady_clock::now()) {
            start_measurement();
          }
          else {
            update_DetectorState(DETECTORSTATE_WAITING);
            call_async([this, tp]() {
              std::this_thread::sleep_until(tp);
              start_measurement();
            });
          }
        }
      }
    });
  }
}

void DLD::cb_static_measurement_complete(void *priv, int reason)
{
  reinterpret_cast<DLD*>(priv)->cb_measurement_complete(reason);
}

int DLD::start_measurement()
{
  auto time_ms = static_cast<int>(data_.exposure * 1000.0);
  for (auto& som_listener : som_listeners_) {
    som_listener->start_of_measurement(time_ms);
  }
  last_acq_start_ = std::chrono::steady_clock::now();
  // start acquisition
  int ret = sc_tdc_start_measure2(dev_desc_, time_ms);
  int retries = 5;
  while (ret == SC_TDC_ERR_NOTRDY) {
    if (retries == 0) {
      break;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    ret = sc_tdc_start_measure2(dev_desc_, time_ms);
    retries--;
  }
  if (ret == 0) {
    data_.acquire = 1; // no update, client reads parameters after writing them
    update_DetectorState(DETECTORSTATE_ACQUIRE);
  }
  else {
    char buf[ERRSTRLEN]; // ERRSTRLEN from scTDC.h, should be 256
    buf[0] = '\0';
    sc_get_err_msg(ret, buf);
    update_StatusMessage(buf);
  }
  return ret;
}
