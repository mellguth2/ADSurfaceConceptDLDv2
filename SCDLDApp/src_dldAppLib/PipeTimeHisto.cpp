/* Copyright 2022 Surface Concept GmbH */
#include "PipeTimeHisto.hpp"

#include <cstring>
#include <scTDC.h>
#include <scTDC_types.h>
#include "TimeBin.hpp"

#include <iostream>

PipeTimeHisto::PipeTimeHisto(TimeBin& time_bin)
  : time_bin_(time_bin), accumulate_(false)
{
  user_tstart_ns_ = 0.0;
  user_tsize_ns_ = 1e6;
  params_.reset(new sc_pipe_dld_sum_histo_params_t);
  next_params_.reset(new sc_pipe_dld_sum_histo_params_t);
  auto& p = *params_; // short alias
  p.depth = BS32;
  p.channel = -1;
  p.modulo = 0;
  p.binning.x = 1;
  p.binning.y = 1;
  p.binning.time = 10;
  p.roi.offset.x = 0;
  p.roi.offset.y = 0;
  p.roi.offset.time = 0;
  p.roi.size.x = 16383;  // no effect on required memory, acceptance interval
  p.roi.size.y = 16383;  // no effect on required memory, acceptance interval
  p.roi.size.time = 100; // -> required memory
  p.accumulation_ms = 0xFFFFFFFFu;
  p.allocator_owner = this;
  p.allocator_cb = static_allocator_cb;
  *next_params_ = *params_;
}

PipeTimeHisto::~PipeTimeHisto()
{

}

int PipeTimeHisto::create(int dev_desc)
{
  dev_desc_ = dev_desc;
  pipe_desc_ = -1;
  change_request_ = true;
  return 0;
}

void PipeTimeHisto::start_of_measurement(int time_ms)
{
  // this function executes before the actual call to sc_tdc_start_measure2(),
  // so we have a chance here to replace the pipe for a new configuration
  check_update_pipe();
}

void PipeTimeHisto::end_of_measurement()
{
  void* dummy;
  sc_pipe_read2(dev_desc_, pipe_desc_, &dummy, 100);
  // compute x axis
  auto nrsteps = data_.size() > 1u ? data_.size() - 1u : 1u;
  auto tstep = actual_tsize_ns_ / nrsteps;
  for (std::size_t i = 0; i < xaxis_.size(); i++) {
    xaxis_[i] = actual_tstart_ns_ + i * tstep;
  }
  // convert histogram values to double
  for (std::size_t i = 0; i < data_.size(); i++) {
    yaxis_[i] = static_cast<double>(data_[i]);
  }
  // send out x and y values
  data_consumer_(data_.size(), xaxis_.data(), yaxis_.data());
}

void PipeTimeHisto::setDataConsumer(PipeTimeHisto::data_consumer_t v)
{
  data_consumer_ = v;
}

void PipeTimeHisto::setSizeT(int v_in)
{
  unsigned long long v =
    v_in < 0 ? 2ull : static_cast<unsigned long long>(v_in);
  if (v != params_->roi.size.time) {
    change_request_ = true;
    next_params_->roi.size.time = v;
  }
}

void PipeTimeHisto::setMinTSI(double v)
{
  if (v != user_tstart_ns_) {
    change_request_ = true;
    user_tstart_ns_ = v;
  }
}

void PipeTimeHisto::setSizeTSI(double v)
{
  if (v != user_tsize_ns_) {
    change_request_ = true;
    user_tsize_ns_ = v;
  }
}

int PipeTimeHisto::sizeT() const
{
  return next_params_->roi.size.time;
}

double PipeTimeHisto::minTSI() const
{
  return user_tstart_ns_;
}

double PipeTimeHisto::sizeTSI() const
{
  return user_tsize_ns_;
}

void PipeTimeHisto::setAccumulate(int v)
{
  accumulate_ = v > 0;
}

int PipeTimeHisto::accumulate() const
{
  return accumulate_ ? 1 : 0;
}


int PipeTimeHisto::static_allocator_cb(void *priv, void **buf)
{
  return static_cast<PipeTimeHisto*>(priv)->allocator_cb(buf);
}

int PipeTimeHisto::allocator_cb(void **buf)
{
  if (!accumulate_) {
    std::fill(data_.begin(), data_.end(), 0u); // reset to all zeros
  }
  *buf = data_.data();
  return 0;
}

void PipeTimeHisto::resize_data()
{
  auto s = static_cast<std::size_t>(params_->roi.size.time);
  data_.resize(s);
  std::fill(data_.begin(), data_.end(), 0u);
  xaxis_.resize(s);
  yaxis_.resize(s);
}

void PipeTimeHisto::check_update_pipe()
{
  if (change_request_ || pipe_desc_ < 0) {
    change_request_ = false;
    if (pipe_desc_ >= 0) {
      sc_pipe_close2(dev_desc_, pipe_desc_);
    }
    auto& p = *next_params_;
    TimeBin::Result r = time_bin_.autobins(
      user_tstart_ns_, user_tsize_ns_, p.roi.size.time);
    p.binning.time = 1ull << r.binpow;
    p.roi.offset.time = r.offset;
    actual_tstart_ns_ = r.tstart_ns;
    actual_tsize_ns_ = r.tsize_ns;

    *params_ = *next_params_;
    resize_data();
    pipe_desc_ = sc_pipe_open2(dev_desc_, DLD_SUM_HISTO, params_.get());
  }
}
