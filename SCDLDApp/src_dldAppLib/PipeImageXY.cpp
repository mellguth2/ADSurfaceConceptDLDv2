#include "PipeImageXY.hpp"

#include <cstring>
#include <scTDC.h>
#include <scTDC_types.h>


PipeImageXY::PipeImageXY()
{
  params_.reset(new sc_pipe_dld_image_xy_params_t);
  next_params_.reset(new sc_pipe_dld_image_xy_params_t);
  auto& p = *params_; // short alias
  p.depth = BS32;
  p.channel = -1;
  p.roi.offset.x = 0;
  p.roi.offset.y = 0;
  p.roi.offset.time = 0;
  p.roi.size.x = 1450;
  p.roi.size.y = 1450;
  p.roi.size.time = 1ull << 60;
  p.binning.x = 1;
  p.binning.y = 1;
  p.binning.time = 1;
  p.modulo = 0;
  p.accumulation_ms = 0xFFFFFFFFu;
  p.allocator_owner = this;
  p.allocator_cb = static_allocator_cb;
  *next_params_ = *params_;
}

PipeImageXY::~PipeImageXY()
{

}

int PipeImageXY::create(int dev_desc)
{
  dev_desc_ = dev_desc;
  resize_data();
  pipe_desc_ = sc_pipe_open2(dev_desc, DLD_IMAGE_XY, params_.get());
  return 0;
}

void PipeImageXY::start_of_measurement(int time_ms)
{
  // this function executes before the actual call to sc_tdc_start_measure2(),
  // so we have a chance here to replace the pipe for a new configuration
  if (change_request_) {
    change_request_ = false;
    sc_pipe_close2(dev_desc_, pipe_desc_);
    *params_ = *next_params_;
    resize_data();
    pipe_desc_ = sc_pipe_open2(dev_desc_, DLD_IMAGE_XY, params_.get());
  }
}

void PipeImageXY::end_of_measurement()
{
  void* dummy;
  sc_pipe_read2(dev_desc_, pipe_desc_, &dummy, 100);
  data_consumer_(
    data_.size(),
    params_->roi.size.x,
    reinterpret_cast<int*>(data_.data()));
  // slightly evil (unsigned misinterpreted as int. scTDC1 only has
  // unsigned buffer types for the histogram pipes, and ADDriver only has
  // signed buffer types and I don't want to convert. This is fine, as long
  // as the counts per pixel are not going above max int = 2^31 - 1, which
  // is about 2 billion)
}

void PipeImageXY::setDataConsumer(PipeImageXY::data_consumer_t v)
{
  data_consumer_ = v;
}

void PipeImageXY::setMinX(int v)
{
  if (v != params_->roi.offset.x) {
    change_request_ = true;
    next_params_->roi.offset.x = v;
  }
}

void PipeImageXY::setMinY(int v)
{
  if (v != params_->roi.offset.y) {
    change_request_ = true;
    next_params_->roi.offset.y = v;
  }
}

void PipeImageXY::setSizeX(int v_in)
{
  unsigned v = v_in > 1 ? static_cast<unsigned>(v_in) : 2u;
  if (v != params_->roi.size.x) {
    change_request_ = true;
    next_params_->roi.size.x = v;
  }
}

void PipeImageXY::setSizeY(int v_in)
{
  unsigned v = v_in > 1 ? static_cast<unsigned>(v_in) : 2u;
  if (v != params_->roi.size.y) {
    change_request_ = true;
    next_params_->roi.size.y = v;
  }
}

void PipeImageXY::setBinX(int v_in)
{
  if (v_in > 15) v_in = 15;
  if (v_in < 0) v_in = 0;
  unsigned v = 1u << static_cast<unsigned>(v_in);
  if (v != params_->binning.x) {
    change_request_ = true;
    next_params_->binning.x = v;
  }
}

void PipeImageXY::setBinY(int v_in)
{
  if (v_in > 15) v_in = 15;
  if (v_in < 0) v_in = 0;
  unsigned v = 1u << static_cast<unsigned>(v_in);
  if (v != params_->binning.y) {
    change_request_ = true;
    next_params_->binning.y = v;
  }
}

// the getter function shall represent the latest user-requested settings,
// not the currently active pipe, so we pass back values from next_params_
int PipeImageXY::minX() const
{
  return static_cast<int>(next_params_->roi.offset.x);
}

int PipeImageXY::minY() const
{
  return static_cast<int>(next_params_->roi.offset.y);
}

int PipeImageXY::sizeX() const
{
  return static_cast<int>(next_params_->roi.size.x);
}

int PipeImageXY::sizeY() const
{
  return static_cast<int>(next_params_->roi.size.y);
}

int PipeImageXY::binX() const
{
  return log2_(next_params_->binning.x);
}

int PipeImageXY::binY() const
{
  return log2_(next_params_->binning.y);
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

void PipeImageXY::resize_data()
{
  data_.resize(
    static_cast<std::size_t>(params_->roi.size.x) * params_->roi.size.y);
}

int PipeImageXY::log2_(unsigned v)
{
  int r = -1;
  while (v > 0) {
    v >>= 1u;
    r++;
  }
  return r;
}
