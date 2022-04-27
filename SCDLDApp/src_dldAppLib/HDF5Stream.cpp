#include "HDF5Stream.hpp"

#include <scTDC_hdf5.h>               // add-on library, not the scTDC SDK
#include <scTDC_hdf5_error_codes.h>

namespace {
  enum DataSelEnum : unsigned {
    MASK_STARTCTR = 0x001,
    MASK_TIMETAG  = 0x002,
    MASK_SUBDEV   = 0x004,
    MASK_CHANNEL  = 0x008,
    MASK_TIME     = 0x010,
    MASK_X        = 0x020,
    MASK_Y        = 0x040,
    MASK_MRC      = 0x080,
    MASK_ADC      = 0x100,
    MASK_SIGNALBIT = 0x200
  };
}

HDF5Stream::HDF5Stream()
{
  hdf5obj_ = sc_tdc_hdf5_create();
  // this configuration can be changed at any time and comes into effect
  // once sc_tdc_hdf5_setactive(hdf5obj_, 1) is called
  sc_tdc_hdf5_cfg_datasel(hdf5obj_, MASK_X | MASK_Y | MASK_TIME );
}

HDF5Stream::~HDF5Stream()
{
  sc_tdc_hdf5_destroy(hdf5obj_);
}

int HDF5Stream::create(int dev_desc)
{
  sc_tdc_hdf5_connect(hdf5obj_, dev_desc);
  return 0;
}

void HDF5Stream::disconnect()
{
  sc_tdc_hdf5_disconnect(hdf5obj_);
}

void HDF5Stream::setFilePath(const std::string &v)
{
  filepath_ = v;
  sc_tdc_hdf5_cfg_outfile(hdf5obj_, filepath_.c_str());
}

std::string HDF5Stream::filePath() const
{
  return filepath_;
}

void HDF5Stream::setUserComment(const std::string &v)
{
  comment_ = v;
  sc_tdc_hdf5_cfg_comment(hdf5obj_, comment_.c_str());
}

std::string HDF5Stream::userComment() const
{
  return comment_;
}

int HDF5Stream::setActive(int v)
{
  auto retcode = sc_tdc_hdf5_setactive(hdf5obj_, v);
  file_error_ = (retcode == ERR_FILE) ? 1 : 0;
  return retcode;
}

int HDF5Stream::isActive() const
{
  return sc_tdc_hdf5_isactive(hdf5obj_);
}

int HDF5Stream::fileError() const
{
  return file_error_;
}
