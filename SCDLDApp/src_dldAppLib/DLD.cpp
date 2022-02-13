/* Copyright 2022 Surface Concept GmbH */

#include "DLD.hpp"
#include <string>
#include <scTDC.h>

DLD::DLD()
  : Glue(this),
    configfile_("tdc_gpx3.ini"),
    initialized_(0),
    dev_desc_(-1),
    exposure_(1.0),
    acquire_(0),
    image_mode_(0),
    num_images_(1)
{

}

int DLD::write_Initialize(int v)
{
  if (initialized_ == 0 && v == 1) {
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
        initialized_ = 1;
        update_Initialize(initialized_);
        update_StatusMessage("hardware ready");
      }
    });
  }
  else if (initialized_ == 1 && v == 0 && dev_desc_ >= 0) {
    worker_.addTask( [this]() {
      sc_tdc_deinit2(dev_desc_);
      initialized_ = 0;
      update_Initialize(initialized_);
      update_StatusMessage("hardware closed");
    });
  }
  return 0;
}

int DLD::read_Initialize(int *v)
{
  *v = initialized_;
  return 0;
}

int DLD::write_ConfigFile(const std::string& v)
{
  configfile_ = v;
  return 0;
}

int DLD::read_ConfigFile(std::string& dest)
{
  dest = configfile_;
  return 0;
}

int DLD::read_StatusMessage(std::string &dest)
{
  dest = statusmessage_;
  return 0;
}

int DLD::write_Exposure(double v)
{
  exposure_ = v;
  return 0;
}

int DLD::read_Exposure(double *dest)
{
  *dest = exposure_;
  return 0;
}

int DLD::write_Acquire(int v)
{
  if (acquire_ == 0 && v == 1 && initialized_ == 1) {
    // start acquisition
    int ret =
      sc_tdc_start_measure2(dev_desc_, static_cast<int>(exposure_ * 1000.0));
    if (ret == 0) {
      acquire_ = 1;
    }
    return ret;
  }
  else if (acquire_ == 1 && v == 0 && initialized_ == 1) {
    // interrupt acquisition
    sc_tdc_interrupt2(dev_desc_); // asynchronous, do not update_Acquire yet
    return 0;
  }
  return 0;
}

int DLD::read_Acquire(int *dest)
{
  *dest = acquire_;
  return 0;
}

int DLD::write_ImageMode(int v)
{
  image_mode_ = v;
  return 0;
}

int DLD::read_ImageMode(int *dest)
{
  *dest = image_mode_;
  return 0;
}

int DLD::write_NumImages(int v)
{
  num_images_ = v;
  return 0;
}

int DLD::read_NumImages(int *dest)
{
  *dest = num_images_;
  return 0;
}

int DLD::write_DataType(int v)
{
  data_type_ = v;
  return 0;
}

int DLD::read_DataType(int *dest)
{
  *dest = data_type_;
  return 0;
}

int DLD::init_impl()
{
  int ret = sc_tdc_init_inifile(configfile_.c_str());
  if (ret == 0) {
    dev_desc_ = ret;
    sc_tdc_set_complete_callback2(
      dev_desc_, this, cb_static_measurement_complete);
  }
  return ret;
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
      acquire_ = 0;
      update_Acquire(0);
    });
  }
}

void DLD::cb_static_measurement_complete(void *priv, int reason)
{
  reinterpret_cast<DLD*>(priv)->cb_measurement_complete(reason);
}

