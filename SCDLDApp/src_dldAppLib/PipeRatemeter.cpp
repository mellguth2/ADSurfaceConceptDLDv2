#include "PipeRatemeter.hpp"
#include <scTDC.h>
#include <scTDC_types.h>
#include <cstring>
#include <limits>
#include <algorithm>

PipeRatemeter::PipeRatemeter()
  : buf_(new statistics_t)
{
  memset(buf_.get(), 0, sizeof(statistics_t));
}

PipeRatemeter::~PipeRatemeter()
{
}

int PipeRatemeter::create(int dev_desc)
{
  sc_pipe_statistics_params_t p;
  p.allocator_owner = this;
  p.allocator_cb = static_allocator_cb;
  pipe_desc_ = sc_pipe_open2(dev_desc, STATISTICS, &p);
  dev_desc_ = dev_desc;
  return pipe_desc_;
}

void PipeRatemeter::start_of_measurement(int time_ms)
{
  last_time_ms_= time_ms;
}

void PipeRatemeter::end_of_measurement()
{
  void* dummy = 0;
  sc_pipe_read2(dev_desc_, pipe_desc_, &dummy, 100);
  if (!data_consumer_) {
    return;
  }
  if (last_time_ms_ <= 0) {
    last_time_ms_ = 1;
  }
  auto calc = [this](unsigned v) -> int {
    // convert from counts per measurement to counts per second
    // and from unsigned to signed saturating at max representable signed int
    v = static_cast<unsigned>(
      (static_cast<unsigned long long>(v) * 1000ull) / last_time_ms_ );
    static const unsigned maxint =
      static_cast<unsigned>(std::numeric_limits<int>::max());
    return v < maxint ? static_cast<int>(v) : maxint;
  };
  // this is specialized for modern 2D DLDs:
  // 4 channels for pulses on the anode terminals
  // + 1 channel for the reconstructed particle event
  outbuf_.resize(5);
  outbuf_[0] = calc(buf_->counts_read[0][0]);
  outbuf_[1] = calc(buf_->counts_read[0][1]);
  outbuf_[2] = calc(buf_->counts_read[0][2]);
  outbuf_[3] = calc(buf_->counts_read[0][3]);
  outbuf_[4] = calc(std::max(buf_->events_found[0], buf_->events_received[0]));
  data_consumer_(outbuf_.data(), outbuf_.size());
}

void PipeRatemeter::setDataConsumer(PipeRatemeter::data_consumer_t f)
{
  data_consumer_ = f;
}

int PipeRatemeter::static_allocator_cb(void* priv, void** buf)
{
  return static_cast<PipeRatemeter*>(priv)->allocator_cb(buf);
}

int PipeRatemeter::allocator_cb(void** buf)
{
  // scTDC library calls us at the start of a measurement to request a buffer
  // where to put the statistics data. Here, we choose to provide a single
  // static buffer that is used for every measurement
  memset(buf_.get(), 0, sizeof(statistics_t));
  *buf = buf_.get();
  return 0;
}
