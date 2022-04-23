#pragma once

#include "iCreatedAtInit.hpp"
#include "iEndOfMeasListener.hpp"
#include "iStartOfMeasListener.hpp"
#include <functional>
#include <memory>

struct statistics_t;

class PipeRatemeter
  : public iCreatedAtInit,
    public iStartOfMeasListener,
    public iEndOfMeasListener

{
public:
  typedef std::function<void(int*, std::size_t)> data_consumer_t;
  PipeRatemeter();
  virtual ~PipeRatemeter();
  int create(int dev_desc) override;
  void start_of_measurement(int time_ms) override;
  void end_of_measurement() override;
  void setDataConsumer(data_consumer_t);

private:
  static int static_allocator_cb(void* priv, void** buf);
  int allocator_cb(void** buf);

  int pipe_desc_ = -1;
  int dev_desc_ = -1;
  unsigned last_time_ms_ = 1;
  std::unique_ptr<statistics_t> buf_;
  std::vector<int> outbuf_;
  data_consumer_t data_consumer_;
};
