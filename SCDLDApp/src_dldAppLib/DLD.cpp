#include "DLD.hpp"
#include <string>
#include <scTDC.h>

DLD::DLD()
  : Glue(this),
    configfile_("tdc_gpx3.ini"),
    initialized_(0),
    dev_desc_(-1)
{

}

int DLD::write_Initialize(int v)
{
  if (initialized_ == 0 && v == 1) {
    int ret = sc_tdc_init_inifile(configfile_.c_str());
    if (ret < 0) {
      return -1;
    }
    else {
      dev_desc_ = ret;
      initialized_ = 1;
    }
  }
  else if (initialized_ == 1 && v == 0) {
    if (dev_desc_ >= 0) {
      sc_tdc_deinit2(dev_desc_);
      initialized_ = 0;
    }
  }
}

int DLD::write_ConfigFile(const std::string& v)
{
  configfile_ = v;
}

int DLD::read_ConfigFile(std::string& dest)
{
  dest = configfile_;
  return 0;
}
