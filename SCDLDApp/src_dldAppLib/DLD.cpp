/* Copyright 2022 Surface Concept GmbH */

#include "DLD.hpp"
#include <string>
#include <scTDC.h>

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
    acquire(0),
    image_mode(0),
    num_images(1)
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

int DLD::write_Acquire(int v)
{
  if (data_.acquire == 0 && v == 1 && data_.initialized == 1) {
    auto time_ms = static_cast<int>(data_.exposure * 1000.0);
    for (auto& som_listener : som_listeners_) {
      som_listener->start_of_measurement(time_ms);
    }
    // start acquisition
    int ret = sc_tdc_start_measure2(dev_desc_, time_ms);
    if (ret == 0) {
      data_.acquire = 1;
    }
    return ret;
  }
  else if (data_.acquire == 1 && v == 0 && data_.initialized == 1) {
    // interrupt acquisition
    sc_tdc_interrupt2(dev_desc_); // asynchronous, do not update_Acquire yet
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

int DLD::write_DataType(int v)
{
  data_.data_type = v;
  return 0;
}

int DLD::read_DataType(int *dest)
{
  *dest = data_.data_type;
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
      data_.acquire = 0;
      update_Acquire(0);
    });
  }
}

void DLD::cb_static_measurement_complete(void *priv, int reason)
{
  reinterpret_cast<DLD*>(priv)->cb_measurement_complete(reason);
}

