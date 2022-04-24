#include "PipeImageXY.hpp"

#include <cstring>
#include <scTDC.h>
#include <scTDC_types.h>


PipeImageXY::PipeImageXY()
{
  params_.reset(new sc_pipe_dld_image_xy_params_t);
}

PipeImageXY::~PipeImageXY()
{

}

int PipeImageXY::create(int dev_desc)
{
  dev_desc_ = dev_desc;
  auto& p = *params_; // short alias
  p.roi.offset.x = 0;
  p.roi.offset.y = 0;
  p.roi.offset.time = 0;
  p.roi.size.x = 1450;
  p.roi.size.y = 1450;
  p.roi.size.time = 1ull << 60;
  p.binning.x = 1;
  p.binning.y = 1;
  p.binning.time = 1;
  p.channel = -1;
  p.depth = BS32;
  p.modulo = 0;
  p.accumulation_ms = 0xFFFFFFFFu;
  p.allocator_owner = this;
  p.allocator_cb = static_allocator_cb;
  data_.resize(static_cast<std::size_t>(p.roi.size.x) * p.roi.size.y);
  pipe_desc_ = sc_pipe_open2(dev_desc, DLD_IMAGE_XY, params_.get());
  return 0;
}

void PipeImageXY::start_of_measurement(int time_ms)
{
}

void PipeImageXY::end_of_measurement()
{
  void* dummy;
  sc_pipe_read2(dev_desc_, pipe_desc_, &dummy, 100);
  data_consumer_(
    data_.size(),
    params_->roi.size.x,
    reinterpret_cast<int*>(data_.data())); // slightly evil ^^

}

void PipeImageXY::setDataConsumer(PipeImageXY::data_consumer_t v)
{
  data_consumer_ = v;
}

int PipeImageXY::static_allocator_cb(void* priv, void** buf)
{
  return static_cast<PipeImageXY*>(priv)->allocator_cb(buf);
}

int PipeImageXY::allocator_cb(void** buf)
{
  // called by scTDC at the beginning of the measurement
  std::fill(data_.begin(), data_.end(), 0u); // reset to all zeros
  *buf = data_.data();
  return 0;
}
